#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sample/SampleMirrorSource.h"
#include "SampleMimicEntity.generated.h"

class UCrowdyEntityComponent;

/**
 * An invisible "front mirror" source. Spawned locally as a Dynamic Crowdy entity, it has
 * no mesh of its own: each frame the owner moves it to the mirrored transform of its
 * target pawn, and USampleMimicExecutor streams that as the same FSampleEntityState the
 * player streams. Remote clients (and the owner's own round-trip) spawn the character
 * proxy from that struct, so the reflection looks and animates like the player. Mirroring
 * is done with UMathOperations::GetMimicTransform across a fixed plane captured at spawn.
 */
UCLASS()
class CROWDYSDKTEST_API ASampleMimicEntity : public AActor, public ISampleMirrorSource
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

	// ISampleMirrorSource which is read by USampleMimicExecutor to pull the real character's
	// movement and the plane to reflect it across.
	virtual APawn* GetMirrorTarget_Implementation() const override { return MimicTarget.Get(); }
	virtual FTransform GetMirrorPlaneTransform_Implementation() const override { return MirrorPlane; }

protected:

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Sample")
	TObjectPtr<UCrowdyEntityComponent> CrowdyEntity;

	// Pushes the reflection this far along the mirror normal. 0 is a true mirror; a small
	// positive value pulls the reflection off the plane so it never clips the player.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sample")
	float ForwardOffset = 0.f;

private:

	UPROPERTY()
	TWeakObjectPtr<APawn> MimicTarget;

	FTransform MirrorPlane = FTransform::Identity;
};
