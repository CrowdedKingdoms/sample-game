#pragma once

#include "CoreMinimal.h"
#include "Sample/SampleSwitchBase.h"
#include "SampleVoiceSwitch.generated.h"

/**
 * Toggles voice chat for the local player. The voice functions are already
 * BlueprintCallable on UCrowdySDKSubsystem, so there is nothing to wrap: this switch
 * just calls them. Start/Stop control your microphone capture; Play/Mute control
 * whether you hear others.
 */
UCLASS()
class CROWDYSDKTEST_API ASampleVoiceSwitch : public ASampleSwitchBase
{
	GENERATED_BODY()

public:

	ASampleVoiceSwitch();

protected:

	virtual void Interact_Implementation(APawn* Interactor) override;

private:

	bool bVoiceOn = false;
};
