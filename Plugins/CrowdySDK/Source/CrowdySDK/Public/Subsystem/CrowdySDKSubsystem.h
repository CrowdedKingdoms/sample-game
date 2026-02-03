// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/GraphQL/Interfaces/ICrowdyQueryReceptionLayer.h"
#include "Shared/Types/Structures/Versioning/FGameVersion.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CrowdySDKSubsystem.generated.h"



// Structs
struct FUDPAddressNotify;
struct FRegisterResponse;
struct FLoginResponse;
struct FVersionInfoResponse;
struct FGameSessionInfo;
struct FTeleportResponse;

// Interfaces
class ICrowdyQueryRequest;
class ICrowdyQueryTransmissionLayer;
class ICrowdyQueryReceptionLayer;
class ICrowdyTransmissionLayer;
class ICrowdyService;
class ICrowdyReceptionLayer;
class ICrowdyMessage;
class ICrowdyQueryResponse;

// Core Classes
class FCrowdyServiceRegistry;
class FCrowdyMessageParser;
class FMessageBufferPool;
class FCrowdyDataRegistry;
class FCrowdyQueryParser;
class FVoiceChatService;

// Unreal Subsystems
class UCrowdyWorkerThreadsSubsystem;
class UCrowdyQuerySubsystem;
class UCrowdyGameSession;
class UCrowdyUDPSubsystem;
class UVoiceChatSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLogin, bool, bSuccess, FString, GameToken);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRegister, bool, bSuccess, FString, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLogout, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUDPAddressNotify, bool, bSuccess, bool, GateKeep);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVersionInfo, FGameVersion, ServerVersion, FGameVersion, ClientVersion);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUDPConnectionSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUDPTimedOut);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeleportPermission, bool, bAllowed);

/**
 *
 */
UCLASS(BlueprintType)
class CROWDYSDK_API UCrowdySDKSubsystem : public UGameInstanceSubsystem, public ICrowdyQueryReceptionLayer
{
	GENERATED_BODY()
	
public:
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	UPROPERTY(BlueprintAssignable, Category = "CrowdySDK|Authentication")
	FOnLogin OnLogin;
	
	UPROPERTY(BlueprintAssignable, Category = "CrowdySDK|Authentication")
	FOnRegister OnRegister;
	
	UPROPERTY(BlueprintAssignable, Category = "CrowdySDK|Authentication")
	FOnLogout OnLogout;
	
	UPROPERTY(BlueprintAssignable, Category = "CrowdySDK|Authentication")
	FOnUDPAddressNotify OnUDPAddressNotify;
	
	UPROPERTY(BlueprintAssignable, Category = "CrowdySDK|Authentication")
	FOnVersionInfo OnVersionInfo;
	
	UPROPERTY(BlueprintAssignable, Category = "CrowdySDK|Connection")
	FOnUDPConnectionSuccess OnUDPConnectionSuccess;
	
	UPROPERTY(BlueprintAssignable, Category = "CrowdySDK|Connection")
	FOnUDPTimedOut OnUDPTimedOut;
	
	UPROPERTY(BlueprintAssignable, Category= "CrowdySDK|Permissions")
	FOnTeleportPermission OnTeleportPermission;
	
	UFUNCTION(BlueprintCallable, Category = "CrowdySDK|Authentication")
	void Login(const FString Email, const FString Password) const;
	
	UFUNCTION(BlueprintCallable, Category = "CrowdySDK|Authentication")
	void Register(const FString Email, const FString Password) const;
	
	UFUNCTION(BlueprintCallable, Category = "CrowdySDK|Authentication")
	void Logout() const;
	
	UFUNCTION(BlueprintCallable, Category = "CrowdySDK|Authentication")
	void SetGameSessionInfo(const FGameSessionInfo GameSessionInfo);
	
	UFUNCTION(BlueprintCallable, Category = "CrowdySDK|Authentication")
	void RequestUDPAccess() const;
	
	UFUNCTION(BlueprintCallable, Category = "CrowdySDK|Authentication")
	void RequestVersionInfo() const;
	
	UFUNCTION(BlueprintCallable, Category = "CrowdySDK|Authentication")
	void SetQueryEndpoint(const FString InEndpoint) const;
	
	UFUNCTION(BlueprintCallable, Category = "CrowdySDK|Connection")
	void StartUDPTimeoutMonitoring(const float ThresholdSeconds = 30.0f) const;
	
	UFUNCTION(BlueprintCallable, Category = "CrowdySDK|Connection")
	void StopUDPTimeoutMonitoring() const;
	
	UFUNCTION(BlueprintCallable, Category = "CrowdySDK|Connection")
	void StopNetworkOperations() const;
	
	UFUNCTION(BlueprintCallable, Category = "CrowdySDK|Connection")
	void ToggleNetworkMessageProcessing() const;
	
	UFUNCTION(BlueprintCallable, Category = "CrowdySDK|Connection")
	void TriggerUdpHeartbeat() const;
	
	UFUNCTION(BlueprintCallable, Category = "CrowdySDK|Communication")
	void StartVoiceChat();
	
	UFUNCTION(BlueprintCallable, Category = "CrowdySDK|Communication")
	void StopVoiceChat();
	
	UFUNCTION(BlueprintCallable, Category = "CrowdySDK|Communication")
	void PlayVoiceChat();
	
	UFUNCTION(BlueprintCallable, Category = "CrowdySDK|Communication")
	void MuteVoiceChat();
	
	UFUNCTION(BlueprintCallable, Category = "CrowdySDK|Communication")
	void SetVoiceChatStreamTimeoutThreshold(const float InSeconds);
	
	UFUNCTION(BlueprintCallable, Category="CrowdySDK|Communication")
	void ToggleOwnerEcho() const;
	
	UFUNCTION(BlueprintCallable, Category="CrowdySDK|Permissions")
	void RequestTeleportPermission(const int64 ChunkX, const int64 ChunkY, const int64 ChunkZ, const int32 VoxelX,
	                               const int32 VoxelY, const int32 VoxelZ) const;
	
	void RegisterReceptionLayer(ICrowdyReceptionLayer* Layer) const;
	void RegisterQueryReceptionLayer(ICrowdyQueryReceptionLayer* LayerToRegister) const;
	
	void SendMessage(const ICrowdyMessage& Message) const;
	
	void ExecuteQuery(ICrowdyQueryRequest& Query) const;
	
	inline void SetTransmissionLayer(ICrowdyTransmissionLayer* Layer);
	inline void SetQueryTransmissionLayer(ICrowdyQueryTransmissionLayer* Layer);

	virtual void OnResponseReceived(TSharedPtr<ICrowdyQueryResponse> Response) override;
	virtual TArray<EQueryResponseType> GetSupportedResponseType() const override; 

	
private:
	
	UPROPERTY()
	UCrowdyWorkerThreadsSubsystem* WorkerThreadsSubsystem;
	
	UPROPERTY()
	UCrowdyQuerySubsystem* QuerySubsystem;
	
	UPROPERTY()
	UCrowdyGameSession* GameSession;
	
	UPROPERTY()
	UCrowdyUDPSubsystem* UdpSubsystem;
	
	UPROPERTY()
	UVoiceChatSubsystem* VoiceChatSubsystem;
	
	// Internal Service References
	FVoiceChatService* VoiceChatService;
	
	// UDP Messaging
	FCrowdyServiceRegistry* ServiceRegistry;
	FCrowdyMessageParser* Parser;
	FMessageBufferPool* BufferPool;
	ICrowdyTransmissionLayer* TransmissionLayer;

	// GraphQL Query Execution
	FCrowdyDataRegistry* DataRegistry;
	FCrowdyQueryParser* QueryParser;
	ICrowdyQueryTransmissionLayer* QueryTransmissionLayer;
	
	void HandleLogin(const FLoginResponse& LoginResponse) const;
	void HandleRegister(const FRegisterResponse& RegisterResponse) const;
	void HandleUDPAddressNotify(const FUDPAddressNotify& UDPAddressNotify) const;
	void HandleVersionInfoResponse(const FVersionInfoResponse& VersionInfoResponse) const;
	void HandleTeleportPermissionResponse(const FTeleportResponse& TeleportResponse) const;
	bool ValidateVoiceChatSubsystem();
	
	UFUNCTION()
	void OnUDPTimeout();
	
	UFUNCTION()
	void OnUDPConnectionSuccessful();
};
