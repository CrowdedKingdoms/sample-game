#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Replication/RPC/CrowdyEvent.h"
#include "SampleObjectEntity.generated.h"

class UCrowdyEntityComponent;
class UStaticMeshComponent;

/**
 * A simple object you can spawn, move, rotate, and destroy across the network. It is
 * a Static entity (no continuous state channel); it changes only when an RPC arrives.
 *
 * The two RPCs below take an FVector and an FRotator directly. In SDK v2 a CrowdyEvent
 * can carry any supported type as a plain parameter, so there is no payload struct to
 * declare. Each receiver is a normal UFUNCTION tagged meta=(CrowdyEvent) with its
 * routing; the matching CROWDY_EVENT line generates the call-site that replicates it.
 */
UCLASS()
class CROWDYSDKTEST_API ASampleObjectEntity : public AActor
{
	GENERATED_BODY()

public:

	ASampleObjectEntity();

	// Runs on every client in range over the spatial path. Calling SetObjectLocation
	// on the owner moves the object here and announces the move to the others.
	UFUNCTION(meta=(CrowdyEvent, CrowdyRecipient="SpatialMulticast"))
	void SetObjectLocation_Implementation(FVector NewLocation);
	CROWDY_EVENT(SetObjectLocation)

	UFUNCTION(meta=(CrowdyEvent, CrowdyRecipient="SpatialMulticast"))
	void SetObjectRotation_Implementation(FRotator NewRotation);
	CROWDY_EVENT(SetObjectRotation)

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Sample")
	TObjectPtr<UCrowdyEntityComponent> CrowdyEntity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Sample")
	TObjectPtr<UStaticMeshComponent> Mesh;
};
