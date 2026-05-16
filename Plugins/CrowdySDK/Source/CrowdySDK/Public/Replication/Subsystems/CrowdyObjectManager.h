// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/UDP/Interfaces/ICrowdyReceptionLayer.h"
#include "StructUtils/InstancedStruct.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrowdyObjectManager.generated.h"

class UCrowdyGameSession;
class UCrowdyObjectEventHandler;


/**
 * 
 */
UCLASS(BlueprintType, meta=(DisplayName="Crowdy Object Manager"))
class CROWDYSDK_API UCrowdyObjectManager : public UWorldSubsystem, public ICrowdyReceptionLayer
{
	GENERATED_BODY()
	
public:
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	
	virtual void OnMessageReceived(TSharedRef<ICrowdyMessage> Message) override;
	virtual TArray<ECrowdyMessageType> GetSupportedResponseTypes() const override;
	virtual TArray<FName> GetSupportedEventTypes() const override;
	
	UFUNCTION(BlueprintCallable, Category="Crowdy SDK|Crowdy Object Manager")
	void RegisterObject(const FGuid ObjectID, const FGuid& OwnerID, FInstancedStruct& RegistrationPayload, AActor* Actor) const;
	
	UFUNCTION(BlueprintCallable, Category="Crowdy SDK|Crowdy Object Manager")
	void UnregisterObject(const FGuid& ObjectID);
	
	UFUNCTION(BlueprintCallable, Category="Crowdy Object")
	void RouteObjectEvent(const FGuid& UUID, const FInstancedStruct& EventPayload);
	
	UFUNCTION(BlueprintCallable, Category="Crowdy Object")
	void RegisterEventHandler(UCrowdyObjectEventHandler* Handler);
	
	AActor* FindActor(const FGuid& ObjectID) const;
	FGuid FindID(const AActor* Actor) const;
	bool IsLocallyOwned(const FGuid& ObjectID) const;
	bool IsLocallyOwned(const AActor* Actor) const;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crowdy SDK|Crowdy Object Manager")
	TArray<FName> SupportedGameEvents; 

private:
	
	struct FObjectEntry
	{
		FGuid ObjectID;
		FGuid OwnerID;
		FInstancedStruct Payload;
		TWeakObjectPtr<AActor> Actor;
	};
	
	TMap<FGuid, FObjectEntry> IDToObject;
	
	UPROPERTY()
	TMap<AActor*, FGuid> ActorToID;
	
	UPROPERTY()
	TObjectPtr<UCrowdyGameSession> CrowdyGameSession;
	
	UPROPERTY()
	FGuid CachedLocalPlayerID;
	
	UPROPERTY()
	TMap<UScriptStruct*, TObjectPtr<UCrowdyObjectEventHandler>> EventHandlers;
	
	UPROPERTY()
	TSet<TObjectPtr<UScriptStruct>> SupportedStructTypes;
	
private:
	
	UCrowdyObjectEventHandler* FindHandler(const FInstancedStruct& Payload) const;
	void LoadSupportedStructTypes();
	void LoadConfig();
};
