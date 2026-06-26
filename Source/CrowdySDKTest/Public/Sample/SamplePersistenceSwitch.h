#pragma once

#include "CoreMinimal.h"
#include "Sample/SampleSwitchBase.h"
#include "SamplePersistenceSwitch.generated.h"

/**
 * Saves and loads the interacting player's progress through the persistence
 * subsystem. The first interaction saves (push over UDP, fast); the next loads (pull
 * over GraphQL, async) and teleports the player back to the saved spot. The Subject
 * passed to push and pull is the player pawn, which the SDK uses to derive a
 * per-player instance key.
 */
UCLASS()
class CROWDYSDKTEST_API ASamplePersistenceSwitch : public ASampleSwitchBase
{
	GENERATED_BODY()

public:

	ASamplePersistenceSwitch();

protected:

	virtual void Interact_Implementation(APawn* Interactor) override;

private:

	int32 SavedLevel = 0;
	int32 Step = 0;
};
