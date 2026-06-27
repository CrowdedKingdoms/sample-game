#pragma once

#include "CoreMinimal.h"
#include "Replication/Executor/ActorUpdateExecutor.h"
#include "SampleMimicExecutor.generated.h"

/**
 * State hook for the Mimic. Unlike USampleTransformExecutor (which snapshots its own
 * owner), this reads the mirrored player: it ships the mimic actor's already-mirrored
 * transform plus the TARGET character's velocity and falling flag, with the velocity
 * reflected across the mirror plane the same way GetMimicTransform reflects position.
 *
 * It produces the same FSampleEntityState the player pawn streams, so the pool spawns
 * the same proxy class (the character) for the reflection, and that proxy's AnimBP
 * animates from the replicated velocity instead of sitting in idle.
 */
UCLASS(EditInlineNew)
class CROWDYSDKTEST_API USampleMimicExecutor : public UActorUpdateExecutor
{
	GENERATED_BODY()

public:

	virtual FInstancedStruct GetActorState_Implementation(const UActorComponent* UpdateComponent) const override;
	virtual UScriptStruct* GetStateStruct_Implementation() const override;
};
