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

	// The entity spawned as the reflection. Defaults to ASampleMimicEntity, which wears
	// the interacting player's appearance, so the raw C++ class already looks like you.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sample")
	TSubclassOf<ASampleMimicEntity> MimicClass;

private:

	UPROPERTY()
	TWeakObjectPtr<AActor> SpawnedMimic;
};
