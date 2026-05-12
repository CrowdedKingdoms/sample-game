// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Structs/Game/FSampleActorUpdate.h"
#include "GameFramework/Actor.h"
#include "Replication/Subsystems/CrowdyActorTracker.h"
#include "StructUtils/InstancedStruct.h"
#include "SamplePawnManager.generated.h"

enum class ESampleAnimState : uint8;

UCLASS(BlueprintType)
class CROWDYSDKTEST_API ASamplePawnManager : public AActor
{
	GENERATED_BODY()

public:
	
	// Sets default values for this actor's properties
	ASamplePawnManager();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void AddInstance(const FGuid& UUID, const FSampleActorUpdate& Update);
	void DestroyInstance(const FGuid& UUID);
	void AppendInstanceUpdate(const FSampleActorUpdate& Update);
	void ChangeAnimation(const FGuid& UUID, const ESampleAnimState NewAnimationState);
	
protected:
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	
	UPROPERTY(EditAnywhere, Category="Sample Pawn Manager|Config")
	TSubclassOf<AActor> ReplicatedActorClass;
	
	UPROPERTY(EditAnywhere, Category="Sample Pawn Manager|Config")
	int32 PoolSize = 10;
	
	UPROPERTY(EditAnywhere, Category="Sample Pawn Manager|Config")
	bool bAllowPoolGrowth = true;
	
	UPROPERTY(EditAnywhere, Category="Sample Pawn Manager|Config", meta=(ClampMin=0, DisplayName="Interpolation Delay (Milliseconds)"))
	int64 InterpolationDelayMs = 50;
	
	UPROPERTY()
	TArray<AActor*> ActiveActors;
	
	UPROPERTY()
	TArray<AActor*> InactiveActors;
	
	
	TMap<FGuid, int32> UUIDToSlot;
	
	TQueue<FSampleActorUpdate, EQueueMode::Mpsc> UpdateQueue;
	
	TArray<FActorSlot> Slots;
	TArray<FTransformSnapshot> TransformSnapshots;
	TArray<FTransform> TransformBuffer;
	TArray<int32> PendingRemovals;
	TArray<FVector> LastAppliedPositions;
	TArray<FRotator> LastAppliedRotations;
	
	int64 ServerTimeOffsetMs = 0;
	int64 BestOffsetMs = MIN_int64;
	bool bHasInitialOffset = false;


private:
	
	UFUNCTION()
	void OnActorSpawned(FGuid UUID, FInstancedStruct InitialState, int32 ActorCount);
	
	UFUNCTION()
	void OnActorDestroyed(FGuid UUID, int32 ActorCount);
	
	void OnUpdateBatch(const TArray<FCrowdyActorUpdate>& Updates);
	
	void InitializePool();
	static void ActivateActor(AActor* Actor, const FVector& Location, const FRotator& Rotation);
	static void DeactivateActor(AActor* Actor);
	
	AActor* GetPawnFromPool();
	void ReturnPawnToPool(AActor* Actor);
	
	void ApplyPendingUpdates();
	void ProcessRemovals();
	void UpdateMovementData();
	void ApplyTransformData();
	void UpdateServerTimeOffset(const int64 ServerTimestampMs);
	int64 GetEstimatedServerTimeMs() const;
	static FTransform InterpolateTransform(const FTransformSnapshot& Buffer, const int64 RenderTime);
};
