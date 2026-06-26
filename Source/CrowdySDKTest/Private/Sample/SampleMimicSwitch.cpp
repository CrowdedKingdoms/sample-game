#include "Sample/SampleMimicSwitch.h"

#include "GameFramework/Pawn.h"
#include "Sample/SampleMimicEntity.h"
#include "StructUtils/InstancedStruct.h"
#include "Utils/CrowdyUtilities.h"

ASampleMimicSwitch::ASampleMimicSwitch()
{
	MimicClass = ASampleMimicEntity::StaticClass();
	SwitchLabel = FText::FromString(TEXT("MIMIC\nFront-mirror of your pawn"));
	SwitchColor = FColor(0, 200, 255);
}

void ASampleMimicSwitch::Interact_Implementation(APawn* Interactor)
{
	// Already showing a mirror: tear it down. DestroyCrowdyEntity broadcasts the
	// destroy event so every client removes its copy.
	if (SpawnedMimic.IsValid())
	{
		UCrowdyUtilities::DestroyCrowdyEntity(this, SpawnedMimic.Get());
		SpawnedMimic = nullptr;
		return;
	}

	if (!MimicClass || !Interactor)
	{
		return;
	}

	// The mirror plane sits at the switch, facing the player who triggered it (yaw only,
	// so the reflection stays upright). Reflecting the player across this fixed plane is
	// what makes it behave like a real front mirror.
	const FVector PlaneLocation = GetActorLocation();
	FRotator PlaneRotation = (Interactor->GetActorLocation() - PlaneLocation).Rotation();
	PlaneRotation.Pitch = 0.f;
	PlaneRotation.Roll = 0.f;
	const FTransform MirrorPlane(PlaneRotation, PlaneLocation);

	// SpawnCrowdyEntity defers the spawn so the entity has its identity before
	// BeginPlay, then broadcasts the spawn event so every client spawns the same class.
	// It returns once the local actor is ready, so we can finish wiring it up.
	AActor* Spawned = UCrowdyUtilities::SpawnCrowdyEntity(
		this, MimicClass, Interactor->GetActorTransform(), FInstancedStruct());

	if (ASampleMimicEntity* Mimic = Cast<ASampleMimicEntity>(Spawned))
	{
		Mimic->SetMirror(Interactor, MirrorPlane);
	}

	SpawnedMimic = Spawned;
}
