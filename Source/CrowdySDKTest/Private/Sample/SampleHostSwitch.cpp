#include "Sample/SampleHostSwitch.h"

#include "Sample/SampleObjectEntity.h"
#include "StructUtils/InstancedStruct.h"
#include "Utils/CrowdyUtilities.h"

DEFINE_LOG_CATEGORY_STATIC(LogCrowdySample, Log, All);

ASampleHostSwitch::ASampleHostSwitch()
{
	SwitchLabel = FText::FromString(TEXT("HOST SPAWN\nAuthority-gated"));
	SwitchColor = FColor(230, 50, 50);
}

void ASampleHostSwitch::Interact_Implementation(APawn* Interactor)
{
	// The host is a convention, not an enforced server role; this check just asks the
	// SDK whether the local client is currently the elected host.
	if (!UCrowdyUtilities::CrowdyHasAuthority(this))
	{
		UE_LOG(LogCrowdySample, Log, TEXT("Not the host; ignoring the world-spawn request."));
		return;
	}
	
	if (IsValid(SpawnedEntity.Get()))
	{
		UCrowdyUtilities::DestroyCrowdyEntity(this, SpawnedEntity.Get());
		SpawnedEntity = nullptr;
		return;
	}

	if (WorldObjectClass)
	{
		const FTransform SpawnTM(GetActorRotation(), GetActorLocation() + FVector(0.f, 200.f, 100.f));
		SpawnedEntity = UCrowdyUtilities::SpawnCrowdyEntity(this, WorldObjectClass, SpawnTM, FInstancedStruct());
	}
}
