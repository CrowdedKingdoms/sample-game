#pragma once

#include "CoreMinimal.h"
#include "Data/CrowdyRepApplicationPolicy.h"
#include "Data/TInterpolatedField.h"
#include "SampleTransformReplicationPolicy.generated.h"

/**
 * The remote-proxy side of Dynamic-entity replication. The Actor Pool backend builds
 * one instance of this from UCrowdyActorPoolBackendConfig::ReplicationPolicyClass and
 * feeds it every FSampleEntityState snapshot the AutoReplicator delivers. ExtractFields
 * buffers position/rotation per slot; ApplyToActor samples the buffer at render time so
 * remote copies move smoothly between snapshots.
 *
 * The SDK base class is Abstract, so a concrete subclass like this one is what shows up
 * in the ReplicationPolicyClass dropdown. Without it the backend init bails and
 * replication silently no-ops.
 */
UCLASS(BlueprintType, meta=(DisplayName="Sample Transform Replication Policy"))
class CROWDYSDKTEST_API USampleTransformReplicationPolicy : public UCrowdyRepApplicationPolicy
{
	GENERATED_BODY()

public:

	virtual bool ExtractFields(const FInstancedStruct& State, int64 ServerTimestampMs, int32 SlotId) override;
	virtual void ApplyToActor(AActor* Actor, int32 SlotId, int64 RenderTimeMs) override;
	virtual void OnInstanceDeactivated(int32 SlotId) override;

private:

	TArray<TInterpolatedField<FVector>> Positions;
	TArray<TInterpolatedField<FRotator>> Rotations;

	void EnsureSlot(int32 SlotId);
};
