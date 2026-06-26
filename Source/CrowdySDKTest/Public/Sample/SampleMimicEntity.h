#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SampleMimicEntity.generated.h"

class UCrowdyEntityComponent;
class USkeletalMeshComponent;

/**
 * A live "front mirror" of the player, spawned as a Dynamic Crowdy entity. It wears the
 * player's mannequin (set as a class default so every copy — the owner and the pooled
 * remote proxies alike — shows it without a runtime payload, which the actor pool would
 * otherwise drop). The owner moves it to the mirrored transform of its target pawn every
 * frame; the Dynamic executor snapshots that transform and the SDK replicates it, so the
 * reflection follows the player on every client. Mirroring is done with
 * UMathOperations::GetMimicTransform across a fixed plane captured at spawn.
 */
UCLASS()
class CROWDYSDKTEST_API ASampleMimicEntity : public AActor
{
	GENERATED_BODY()

public:

	ASampleMimicEntity();

	// The switch calls this right after spawning: whom to mirror, and the mirror plane
	// (a point and facing in the world) to reflect that pawn across.
	UFUNCTION(BlueprintCallable, Category="Sample")
	void SetMirror(APawn* InTarget, const FTransform& InMirrorPlane)
	{
		MimicTarget = InTarget;
		MirrorPlane = InMirrorPlane;
	}

protected:

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Sample")
	TObjectPtr<UCrowdyEntityComponent> CrowdyEntity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Sample")
	TObjectPtr<USkeletalMeshComponent> Mesh;

	// Pushes the reflection this far along the mirror normal. 0 is a true mirror; a small
	// positive value pulls the reflection off the plane so it never clips the player.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sample")
	float ForwardOffset = 0.f;

private:

	UPROPERTY()
	TWeakObjectPtr<APawn> MimicTarget;

	FTransform MirrorPlane = FTransform::Identity;
};
