#pragma once

#include "CoreMinimal.h"
#include "Sample/SampleSwitchBase.h"
#include "SampleGhostSwitch.generated.h"

/**
 * Toggles a "ghost" of yourself on and off. The ghost is not a local copy: it is the
 * round trip of your own replicated state. With owner tracking on, the server echoes
 * your entity's state back to you and the rendering backend draws it as a proxy, so
 * you see exactly what the other clients see after a full client to server to client
 * trip. For there to be anything to echo, your player pawn must be a Dynamic Crowdy
 * entity (a UCrowdyEntityComponent in Dynamic mode).
 */
UCLASS()
class CROWDYSDKTEST_API ASampleGhostSwitch : public ASampleSwitchBase
{
	GENERATED_BODY()

public:

	ASampleGhostSwitch();

protected:

	virtual void Interact_Implementation(APawn* Interactor) override;

private:

	bool bGhostOn = false;
};
