#include "Sample/SampleVoiceSwitch.h"

#include "Engine/GameInstance.h"
#include "Subsystem/CrowdySDKSubsystem.h"

ASampleVoiceSwitch::ASampleVoiceSwitch()
{
	SwitchLabel = FText::FromString(TEXT("VOICE\nToggle mic + playback"));
	SwitchColor = FColor(40, 200, 80);
}

void ASampleVoiceSwitch::Interact_Implementation(APawn* Interactor)
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	UCrowdySDKSubsystem* SDK = GameInstance->GetSubsystem<UCrowdySDKSubsystem>();
	if (!SDK)
	{
		return;
	}

	bVoiceOn = !bVoiceOn;

	if (bVoiceOn)
	{
		// Capture the microphone and play incoming voice.
		SDK->StartVoiceChat();
		SDK->PlayVoiceChat();
	}
	else
	{
		SDK->StopVoiceChat();
		SDK->MuteVoiceChat();
	}
}
