#include "Sample/SampleMimicExecutor.h"

#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Sample/SampleMirrorSource.h"
#include "Sample/SampleTypes.h"

FInstancedStruct USampleMimicExecutor::GetActorState_Implementation(const UActorComponent* UpdateComponent) const
{
	FSampleEntityState State;

	if (!UpdateComponent)
	{
		return FInstancedStruct::Make(State);
	}

	AActor* Owner = UpdateComponent->GetOwner();
	if (!Owner || !Owner->Implements<USampleMirrorSource>())
	{
		return FInstancedStruct::Make(State);
	}

	// The mimic actor's Tick has already moved it to the mirrored pose, so its transform
	// is the reflection we want to send.
	State.Location = Owner->GetActorLocation();
	State.Rotation = Owner->GetActorRotation();

	// Velocity and falling come from the real character we are mirroring (the mimic
	// itself is teleported and has no movement of its own). Reflect the velocity across
	// the mirror plane: into plane-local space, negate the forward component, back to
	// world — the same negate-local-X that GetMimicTransform applies to position.
	if (const APawn* Target = ISampleMirrorSource::Execute_GetMirrorTarget(Owner))
	{
		FVector WorldVelocity = Target->GetVelocity();
		bool bFalling = false;

		if (const ACharacter* Char = Cast<ACharacter>(Target))
		{
			if (const UCharacterMovementComponent* Move = Char->GetCharacterMovement())
			{
				WorldVelocity = Move->Velocity;
				bFalling = Move->IsFalling();
			}
		}

		const FTransform PlaneTM = ISampleMirrorSource::Execute_GetMirrorPlaneTransform(Owner);
		FVector LocalVelocity = PlaneTM.InverseTransformVector(WorldVelocity);
		LocalVelocity.X = -LocalVelocity.X;
		State.Velocity = PlaneTM.TransformVector(LocalVelocity);
		State.bIsFalling = bFalling;
	}

	return FInstancedStruct::Make(State);
}

UScriptStruct* USampleMimicExecutor::GetStateStruct_Implementation() const
{
	return FSampleEntityState::StaticStruct();
}
