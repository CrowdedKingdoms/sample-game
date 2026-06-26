#include "Sample/SampleMimicEntity.h"

#include "Animation/AnimInstance.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "FuntionLibraries/MathOperations.h"
#include "GameFramework/Pawn.h"
#include "Replication/Components/CrowdyEntityComponent.h"
#include "Sample/SampleTransformExecutor.h"
#include "UObject/ConstructorHelpers.h"

ASampleMimicEntity::ASampleMimicEntity()
{
	PrimaryActorTick.bCanEverTick = true;

	// A plain scene root keeps the actor origin at the player's reference point (capsule
	// center); the mesh hangs below it by the standard mannequin offset so the feet sit
	// on the ground and the snapshot transform stays at the same height as the player.
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -90.f), FRotator(0.f, -90.f, 0.f));
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// The player's mannequin, baked in as a class default so it is present on every copy
	// including pre-warmed pool proxies (the actor pool swaps the spawned actor for a
	// pooled one, so a runtime appearance payload would never reach the remote view).
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> PlayerMesh(
		TEXT("/Game/Gameplay/Characters/Mannequins/Meshes/SKM_Quinn_Simple.SKM_Quinn_Simple"));
	if (PlayerMesh.Succeeded())
	{
		Mesh->SetSkeletalMeshAsset(PlayerMesh.Object);
	}

	static ConstructorHelpers::FClassFinder<UAnimInstance> PlayerAnim(
		TEXT("/Game/Gameplay/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed"));
	if (PlayerAnim.Succeeded())
	{
		Mesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		Mesh->SetAnimInstanceClass(PlayerAnim.Class);
	}

	CrowdyEntity = CreateDefaultSubobject<UCrowdyEntityComponent>(TEXT("CrowdyEntity"));

	// Dynamic mode streams the executor's snapshot every interval, which is what makes
	// the mirror move smoothly on the other clients.
	CrowdyEntity->Mode = ECrowdyEntityMode::Dynamic;

	// Runtime spawns have no stable level path to hash, so let the SDK mint a fresh
	// NetID. The spawn event carries that NetID to every client.
	CrowdyEntity->IdentityPolicy = ECrowdyIdentityPolicy::Random;

	// The executor produces the snapshot the AutoReplicator sends.
	CrowdyEntity->StateExecutor = CreateDefaultSubobject<USampleTransformExecutor>(TEXT("MirrorExecutor"));
}

void ASampleMimicEntity::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Only the client that spawned the mirror simulates it. On every other client the
	// mirror is a RemoteProxy driven by the replicated snapshots, so it must not move
	// itself.
	if (!CrowdyEntity->IsLocallyOwned() || !MimicTarget.IsValid())
	{
		return;
	}

	// This locally-owned actor is the invisible source: it computes the mirror transform
	// and replicates it, but the owner views the reflection through the same round-trip as
	// everyone else (the pooled proxy the server echoes back), not this immediate copy.
	// Hiding it leaves the round-tripped proxy as the only mirror the owner sees.
	if (!IsHidden())
	{
		SetActorHiddenInGame(true);
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
