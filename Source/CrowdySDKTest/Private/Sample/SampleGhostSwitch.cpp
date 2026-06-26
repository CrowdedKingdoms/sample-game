#include "Sample/SampleGhostSwitch.h"

#include "Replication/Subsystems/CrowdyActorTracker.h"

ASampleGhostSwitch::ASampleGhostSwitch()
{
	SwitchLabel = FText::FromString(TEXT("GHOST\nState round-trip of you"));
	SwitchColor = FColor(170, 110, 255);
}

void ASampleGhostSwitch::Interact_Implementation(APawn* Interactor)
{
	UWorld* World = GetWorld();
	UCrowdyActorTracker* Tracker = World ? World->GetSubsystem<UCrowdyActorTracker>() : nullptr;
	if (!Tracker)
	{
		return;
	}

	bGhostOn = !bGhostOn;

	// Owner tracking stops the SDK from dropping your own echoed state. With it on, the
	// backend renders that echoed state as a proxy: the ghost is your own replication
	// coming back over the network, not a local duplicate.
	Tracker->ToggleOwnerTracking(bGhostOn);
}
