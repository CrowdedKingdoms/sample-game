#include "Sample/SampleTransformExecutor.h"

#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "Sample/SampleTypes.h"

FInstancedStruct USampleTransformExecutor::GetActorState_Implementation(const UActorComponent* UpdateComponent) const
{
	FSampleEntityState State;

	// The parameter is the UCrowdyEntityComponent driving replication; its owner is
	// the actor whose state we are sending.
	if (UpdateComponent)
	{
		if (const AActor* Owner = UpdateComponent->GetOwner())
		{
			State.Location = Owner->GetActorLocation();
			State.Rotation = Owner->GetActorRotation();
		}
	}

	return FInstancedStruct::Make(State);
}

UScriptStruct* USampleTransformExecutor::GetStateStruct_Implementation() const
{
	return FSampleEntityState::StaticStruct();
}
