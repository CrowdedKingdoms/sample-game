#pragma once

#include "CoreMinimal.h"
#include "Replication/Executor/ActorUpdateExecutor.h"
#include "SampleTransformExecutor.generated.h"

/**
 * The per-entity state hook for a Dynamic entity. The AutoReplicator calls
 * GetActorState every replication interval (only on the owner) and sends whatever it
 * returns; remote clients apply it through the rendering backend. This one snapshots
 * the owning actor's transform into an FSampleEntityState. GetStateStruct returns the
 * struct type, which is also what registers it for wire serialization.
 */
UCLASS(EditInlineNew)
class CROWDYSDKTEST_API USampleTransformExecutor : public UActorUpdateExecutor
{
	GENERATED_BODY()

public:

	virtual FInstancedStruct GetActorState_Implementation(const UActorComponent* UpdateComponent) const override;
	virtual UScriptStruct* GetStateStruct_Implementation() const override;
};
