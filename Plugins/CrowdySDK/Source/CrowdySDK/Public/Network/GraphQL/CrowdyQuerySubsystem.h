// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Shared/Types/Structures/Versioning/FGameVersion.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Dom/JsonObject.h"
#include "TimerManager.h"
#include "CrowdyQuerySubsystem.generated.h"

class FCrowdyDataRegistry;
class FCrowdyQueryParser;
enum class EGraphQLQuery : uint8;
class UGraphQLQueryDatabase;


DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnGraphQLResponse, bool, bSuccess, const FString&, ResponseJson);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeleportResponse, bool, bTeleportAllowed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGetVersionInfo, FGameVersion, ServerVersion, FGameVersion, ClientVersion);



USTRUCT(BlueprintType)
struct FQueryStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Crowdy Query Subsystem")
	int32 QueriesSentPerSecond;

	UPROPERTY(BlueprintReadOnly, Category = "Crowdy Query Subsystem")
	int32 QueryBytesSentPerSecond;

	UPROPERTY(BlueprintReadOnly, Category = "Crowdy Query Subsystem")
	int32 ResponseBytesReceivedPerSecond;

	UPROPERTY(BlueprintReadOnly, Category = "Crowdy Query Subsystem")
	int32 ResponseReceivedPerSecond;

	UPROPERTY(BlueprintReadOnly, Category = "Crowdy Query Subsystem")
	bool bIsReceivingData;
};


/**
 * 
 */
UCLASS()
class CROWDYSDK_API UCrowdyQuerySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	UPROPERTY(BlueprintReadWrite, Category = "Crowdy Query Subsystem")
	UGraphQLQueryDatabase* QueryDatabase;
	
	UPROPERTY(BlueprintAssignable, Category = "Crowdy Query Subsystem")
	FOnGetVersionInfo OnGetVersionInfo;
	
	UPROPERTY()
	FOnGraphQLResponse OnComplete;
	
	UPROPERTY(BlueprintAssignable, Category = "GraphQL Service")
	FOnTeleportResponse OnTeleport;
	
	void InitializeQuerySubsystem(FCrowdyDataRegistry* InDataRegistry);
	
	void SetAuthToken(const FString& InAuthToken);
	
	void ClearAuthToken();
	
	[[nodiscard]] bool HasAuthToken() const;
	
	void ExecuteQueryByID(const EGraphQLQuery QueryID, const TMap<FString, FString>& RuntimeVariables, const bool bIncludeAuthToken = true, const bool bUseNestedJson = false);
	
	void SetEndpoint(const FString& InEndpoint);
	
	
	UFUNCTION(BlueprintCallable, Category = "Crowdy Query Subsystem")
	FQueryStats GetQueryStats() const;

private:
	
	FCrowdyQueryParser* QueryParser;
	FCrowdyDataRegistry* DataRegistry;
	
	UPROPERTY()
	FString AuthToken;
	
	UPROPERTY()
	bool bIsDevelopment;
	
	UPROPERTY()
	FString GraphQLEndpoint;
	
	UPROPERTY()
	FTimerHandle StatsTimerHandle;
	
	std::atomic<int32> QueriesSentPerSecond{0};
	std::atomic<int32> QueryBytesSentPerSecond{0};
	std::atomic<int32> ResponseBytesReceivedPerSecond{0};
	std::atomic<int32> ResponseReceivedPerSecond{0};
	std::atomic<bool> bIsReceivingData{false};
	
	UFUNCTION()
	void UpdateStats();
	
	UFUNCTION()
	FString GetCurrentEndpoint() const;
	
	void ExecuteQuery(const FString& Query, const bool bIncludeAuthToken, const TSharedPtr<FJsonObject>& Variables);
	void OnHttpsRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void ParseAndDispatchToServices(const FString& ResponseContent) const;
};
