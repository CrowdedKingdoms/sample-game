// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/UDP/Interfaces/ICrowdyReceptionLayer.h"
#include "GameFramework/Actor.h"
#include "Core/Structs/Game/FSampleActorUpdate.h"
#include "SamplePlayerManager.generated.h"

struct FGameEventNotification;
class ASamplePawnManager;
class UCrowdyWorkerThreadsSubsystem;
class UCrowdyGameSession;
class UCrowdySDKSubsystem;
struct FSampleActorState;

UCLASS(BlueprintType)
class CROWDYSDKTEST_API ASamplePlayerManager : public AActor, public ICrowdyReceptionLayer
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASamplePlayerManager();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// Implement these to receive messages
	virtual void OnMessageReceived(TSharedRef<ICrowdyMessage> Message) override;
	virtual TArray<ECrowdyMessageType> GetSupportedResponseTypes() const override;
	virtual TArray<FName> GetSupportedEventTypes() const override;

protected:
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	
	// Main SDK
	UPROPERTY()
	UCrowdySDKSubsystem* CrowdySDK;
	
	UPROPERTY(EditAnywhere, Category="Sample Player Manager|Config")
	ASamplePawnManager* PawnManager;


private:
	void HandleGameEvent(const FGameEventNotification& GameEventNotification) const;
};
