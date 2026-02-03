// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/UDP/Interfaces/ICrowdyReceptionLayer.h"
#include "GameFramework/Actor.h"
#include "NPCManager.generated.h"

class UCrowdySDKSubsystem;
struct FActorData;
struct FActorUpdateNotificationMessage;

UCLASS(BlueprintType)
class CROWDYSDKTEST_API ANPCManager : public AActor, public ICrowdyReceptionLayer
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ANPCManager();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	virtual void OnMessageReceived(TSharedRef<ICrowdyMessage> Message) override;
	virtual TArray<ECrowdyMessageType> GetSupportedResponseTypes() const override;
	
	UPROPERTY(BlueprintReadWrite, Category = "NPC Manager")
	FGuid OwnerUUID;
	
	UPROPERTY(BlueprintReadWrite, Category = "NPC Manager")
	bool bProcessOwnerUpdates;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(ExposeOnSpawn))
	UStaticMesh* MeshToSpawn;
	
	UFUNCTION(BlueprintCallable, Category = "NPC Manager")
	void SendActorUpdate(const int64 ChunkX, const int64 ChunkY, const int64 ChunkZ, const FString UUID, const FActorData& ActorData) const;


protected:
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Manager|Timeout")
	float ActorTimeoutThreshold = 2.0f;

private:
	
	UPROPERTY()
	UCrowdySDKSubsystem* SDK;
	
	UPROPERTY(VisibleAnywhere)
	UInstancedStaticMeshComponent* ISM;
	
	UPROPERTY()
	TMap<FGuid, int32> SpawnedActors;
	
	UPROPERTY()
	TMap<FGuid, float>  LastUpdateTimeById;
	
	UPROPERTY()
	TMap<int32, FGuid> IdByInstanceIndex;
	
	UPROPERTY()
	TMap<FGuid, FTransform> TargetById;

	UPROPERTY()
	TMap<FGuid, FTransform> CurrentById;

	UPROPERTY()
	TMap<FGuid, FVector> VelocityById;
	
	UPROPERTY()
	TMap<FGuid, FVector> CurrentVelById;
	
	UPROPERTY()
	TMap<FGuid, FRotator> TargetRotById;
	
	UPROPERTY()
	TMap<FGuid, FVector> LastTargetPosById;
	
	UPROPERTY()
	TMap<FGuid, float>   LastTargetTimeById;
	
	TQueue<FActorData, EQueueMode::Spsc> ActorUpdateQueue;
	
	void EnqueueActorUpdate(FActorUpdateNotificationMessage& ActorUpdateNotify);
	
	static FVector SmoothDampVector(const FVector& Current, const FVector& Target, FVector& CurrentVelocity, float SmoothTime, float DeltaTime, float MaxSpeed);
	
	void RemoveActorInstance(const FGuid& ActorId);
	
};
	
