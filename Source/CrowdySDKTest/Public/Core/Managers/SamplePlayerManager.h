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
	
	virtual TArray<FName> GetSupportedActorUpdateTypes() const override;
	virtual TArray<FName> GetSupportedEventTypes() const override;
	
	// Called from within different blueprints to enable or disable owner reflection 
	UFUNCTION(BlueprintCallable, Category="Sample Player Manager|Config")
	void SetOwnerGhostEnabled(const bool bEnable);
	
protected:
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	
	// Main SDK
	UPROPERTY()
	UCrowdySDKSubsystem* CrowdySDK;
	
	// Holds information such as AppID, your own UUID, Current Chunk Coordinate etc.
	UPROPERTY()
	UCrowdyGameSession* CrowdyGameSession;
	
	UPROPERTY()
	UCrowdyWorkerThreadsSubsystem* WorkerThreadsSubsystem;
	
	UPROPERTY(EditAnywhere, Category="Sample Player Manager|Config")
	ASamplePawnManager* PawnManager;
	
	// Whether to process owner updates or not
	UPROPERTY(EditAnywhere, Category="Sample Player Manager|Config")
	bool bEnableOwnerGhost;
	
	// Threshold for when to check for if a replicated actor has timed out i.e. no longer receiving updates for a specific UUID
	UPROPERTY(EditAnywhere, Category="Sample Player Manager|Config", meta=(ClampMin=0.0f, ClampMax=10.0f))
	float ActorTimeoutThreshold = 3.0f;
	
	// We use multi producer single consumer queues for this, since our message processing is async 
	// and uses multiple threads so this is safe
	// Each worker gets its own queue, this maximizes parallelism and efficiency.
	// This scales well with more core count
	// Although it is not necessary to follow this pattern, as each implementation can vary
	TArray<TUniquePtr<TQueue<FSampleActorUpdate, EQueueMode::Mpsc>>> UpdateQueues;
	
	// These are used to indicate to workers that work is available
	// This is done so that the workers don't sleep unnecessary and this makes the system event driven 
	// rather than constant polling
	TArray<std::atomic<bool>> WorkerTaskScheduledFlags;
	
	// Actor Tracking
	TSet<FGuid> ReplicatedPlayers; 
	TSet<FGuid> PendingSpawns;
	TMap<FGuid, float> LastUpdateTimes; 
	FTimerHandle TimeoutCheckTimerHandle;
	
	// Threading 
	int32 NumberOfWorkerThreads = 2; // dynamically determined 
	int32 MaxUpdatesPerBatch = 500; // Tweak as needed
	double MaxBatchWaitTime = 0.002;
	FRWLock UUIDLock;
	FRWLock LastUpdateLock;
	FRWLock TimeoutLock;
	
private:
	
	void SetupUpdateQueues();
	void HandleActorUpdateMessage(const FSampleActorUpdate& Update);
	void HandleGameEvent(const FGameEventNotification& GameEventNotification) const;
	void ProcessUpdateQueue(int32 WorkerIndex);
	void ProcessTimedOutActors(const TArray<FGuid>& TimedOutActors);
	
	UFUNCTION()
	void CheckForActorTimeouts();
	
};
