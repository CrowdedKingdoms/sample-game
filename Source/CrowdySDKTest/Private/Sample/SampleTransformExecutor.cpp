#include "Sample/SampleTransformExecutor.h"

#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Sample/SampleTypes.h"

FInstancedStruct USampleTransformExecutor::GetActorState_Implementation(const UActorComponent* UpdateComponent) const
{
	FSampleEntityState State;

	// The parameter is the UCrowdyEntityComponent driving replication; its owner is
	// the actor whose state we are sending. This runs only on the owner, where the
	// movement component is still authoritative.
	if (UpdateComponent)
	{
		if (const AActor* Owner = UpdateComponent->GetOwner())
		{
			State.Location = Owner->GetActorLocation();
			State.Rotation = Owner->GetActorRotation();

			if (const ACharacter* Char = Cast<ACharacter>(Owner))
			{
				if (const UCharacterMovementComponent* Move = Char->GetCharacterMovement())
				{
					State.Velocity = Move->Velocity;
					State.bIsFalling = Move->IsFalling();
				}
			}
		}
	}

	return FInstancedStruct::Make(State);
}

UScriptStruct* USampleTransformExecutor::GetStateStruct_Implementation() const
{
	return FSampleEntityState::StaticStruct();
}
