#include "Sample/SampleObjectSwitch.h"

#include "Sample/SampleObjectEntity.h"
#include "StructUtils/InstancedStruct.h"
#include "Utils/CrowdyUtilities.h"

ASampleObjectSwitch::ASampleObjectSwitch()
{
	SwitchLabel = FText::FromString(TEXT("OBJECT\nSpawn / Move / Rotate / Destroy"));
	SwitchColor = FColor(255, 140, 0);
}

void ASampleObjectSwitch::Interact_Implementation(APawn* Interactor)
{
	switch (Step % 4)
	{
	case 0:
		// Spawn. SpawnCrowdyEntity replicates the spawn to every client.
		if (ObjectClass)
		{
			const FTransform SpawnTM(GetActorRotation(), GetActorLocation() + SpawnOffset);
			SpawnedObject = Cast<ASampleObjectEntity>(
				UCrowdyUtilities::SpawnCrowdyEntity(this, ObjectClass, SpawnTM, FInstancedStruct()));
		}
		break;

	case 1:
		// Move. The RPC takes an FVector directly and runs on every client in range.
		if (SpawnedObject.IsValid())
		{
			SpawnedObject->SetObjectLocation(SpawnedObject->GetActorLocation() + FVector(0.f, 0.f, 150.f));
		}
		break;

	case 2:
		// Rotate. Same idea with an FRotator.
		if (SpawnedObject.IsValid())
		{
			SpawnedObject->SetObjectRotation(SpawnedObject->GetActorRotation() + FRotator(0.f, 45.f, 0.f));
		}
		break;

	case 3:
		// Destroy. The destroy event removes the object on every client.
		if (SpawnedObject.IsValid())
		{
			UCrowdyUtilities::DestroyCrowdyEntity(this, SpawnedObject.Get());
			SpawnedObject = nullptr;
		}
		break;

	default:
		break;
	}

	++Step;
}
