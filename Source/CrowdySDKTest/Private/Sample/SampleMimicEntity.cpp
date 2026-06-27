#include "Sample/SampleMimicEntity.h"

#include "Components/SceneComponent.h"
#include "FuntionLibraries/MathOperations.h"
#include "GameFramework/Pawn.h"
#include "Replication/Components/CrowdyEntityComponent.h"
#include "Sample/SampleMimicExecutor.h"

ASampleMimicEntity::ASampleMimicEntity()
{
	PrimaryActorTick.bCanEverTick = true;

	// No mesh: this actor is only a state source. The root keeps the actor origin at the
	// player's reference point so the streamed transform lines up with the character proxy
	// the pool spawns on the other side.
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	CrowdyEntity = CreateDefaultSubobject<UCrowdyEntityComponent>(TEXT("CrowdyEntity"));

	// Dynamic mode streams the executor's snapshot every interval, which is what makes
	// the mirror move smoothly on the other clients.
	CrowdyEntity->Mode = ECrowdyEntityMode::Dynamic;

	// Spawned locally (not via a spawn event), so let the SDK mint a fresh NetID. With
	// Role resolved to Owner, the component auto-registers into the AutoReplicator and
	// starts streaming; remote clients spawn the proxy purely from the state struct.
	CrowdyEntity->IdentityPolicy = ECrowdyIdentityPolicy::Random;

	// The mimic executor reads the mirrored target and emits an FSampleEntityState — the
	// same struct the player pawn streams — so the pool spawns the character proxy for it.
	CrowdyEntity->StateExecutor = CreateDefaultSubobject<USampleMimicExecutor>(TEXT("MirrorExecutor"));
}

void ASampleMimicEntity::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Only the client that spawned the mirror simulates it. Remote clients never run this
	// actor; they only render the character proxy the pool spawns from its stream.
	if (!CrowdyEntity->IsLocallyOwned() || !MimicTarget.IsValid())
	{
		return;
	}

	const APawn* Target = MimicTarget.Get();

	// Reflect the player across the fixed mirror plane: stand on the far side, face the
	// player, swap left and right. Because the plane is fixed in the world, walking away
	// from it pushes the reflection away from you, exactly like a real mirror.
	FVector OutLocation;
	FRotator OutRotation;
	UMathOperations::GetMimicTransform(
		Target->GetActorLocation(), Target->GetActorRotation(),
		MirrorPlane.GetLocation(), MirrorPlane.Rotator(),
		ForwardOffset, OutLocation, OutRotation);

	SetActorLocationAndRotation(OutLocation, OutRotation);
}
