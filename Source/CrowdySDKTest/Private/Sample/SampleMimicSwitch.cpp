#include "Sample/SampleMimicSwitch.h"

#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Replication/Subsystems/CrowdyActorManager.h"
#include "Sample/SampleMimicEntity.h"
#include "Sample/SampleTypes.h"

ASampleMimicSwitch::ASampleMimicSwitch()
{
	MimicClass = ASampleMimicEntity::StaticClass();
	SwitchLabel = FText::FromString(TEXT("MIMIC\nFront-mirror of your pawn"));
	SwitchColor = FColor(0, 200, 255);
}

void ASampleMimicSwitch::Interact_Implementation(APawn* Interactor)
{
	// Already showing a mirror: tear it down. The mimic is a purely local source, so a
	// local Destroy is enough — its EndPlay unregisters it from the AutoReplicator and
	// the remote proxies fall out of the pool once their stream stops (ActorTimeout).
	if (SpawnedMimic.IsValid())
	{
		SpawnedMimic->Destroy();
		SpawnedMimic = nullptr;
		return;
	}

	if (!MimicClass || !Interactor)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
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

	// Spawn the mimic LOCALLY — no spawn event. Deferred so its mirror target is set
	// before BeginPlay registers the entity and starts streaming. The mimic's Dynamic
	// component resolves Role=Owner and joins the AutoReplicator on its own; remote
	// clients materialise the reflection from the state struct alone.
	const FTransform SpawnTM = Interactor->GetActorTransform();
	ASampleMimicEntity* Mimic = World->SpawnActorDeferred<ASampleMimicEntity>(MimicClass, SpawnTM);
	if (!Mimic)
	{
		return;
	}

	Mimic->SetMirror(Interactor, MirrorPlane);
	Mimic->FinishSpawning(SpawnTM);

	// The mimic's component just registered FSampleEntityState -> ASampleMimicEntity on
	// this client. Re-point that struct at the player proxy class so the reflection (and
	// any other FSampleEntityState entity we render) spawns the character, not the
	// invisible puppeteer. Remote clients already map this struct to their own pawn class.
	if (UCrowdyActorManager* Manager = World->GetSubsystem<UCrowdyActorManager>())
	{
		const TSubclassOf<AActor> ResolvedProxy = ProxyClass ? ProxyClass : TSubclassOf<AActor>(MimicClass);
		Manager->RegisterStateClass(FSampleEntityState::StaticStruct(), ResolvedProxy);
	}

	SpawnedMimic = Mimic;
}
