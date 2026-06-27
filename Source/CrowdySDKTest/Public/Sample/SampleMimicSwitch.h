#pragma once

#include "CoreMinimal.h"
#include "Sample/SampleSwitchBase.h"
#include "SampleMimicSwitch.generated.h"

class ASampleMimicEntity;

/** Interacting spawns a mirror of the interacting player; interacting again removes it. */
UCLASS()
class CROWDYSDKTEST_API ASampleMimicSwitch : public ASampleSwitchBase
{
	GENERATED_BODY()

public:

	ASampleMimicSwitch();

protected:

	virtual void Interact_Implementation(APawn* Interactor) override;

	// The invisible "puppeteer" spawned locally to drive the reflection. It only sources
	// the mirrored state stream, it is never what other clients see.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sample")
	TSubclassOf<ASampleMimicEntity> MimicClass;

	// The actor the pool should spawn on every client for the reflection your player
	// character (e.g., BP_ThirdPersonCharacter). The mimic streams FSampleEntityState, so
	// this must be the same class your player pawn maps that struct to. Leave it unset to
	// fall back to MimicClass.
	// This is only needed to set for this complex example, 
	// in the normal development you'd never have to set proxy classes when using Actor Pool System within Crowdy SDK
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sample")
	TSubclassOf<AActor> ProxyClass;

private:

	UPROPERTY()
	TWeakObjectPtr<AActor> SpawnedMimic;
};
