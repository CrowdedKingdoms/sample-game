#pragma once

#include "CoreMinimal.h"
#include "Sample/SampleSwitchBase.h"
#include "SampleHostSwitch.generated.h"

class ASampleObjectEntity;

/**
 * Spawns a shared world object, but only when this client is the elected host.
 * Spawning world entities is host-convention work: the host owns them, and every
 * other client receives the spawn over the network. Non-hosts that interact here do
 * nothing, which is what CrowdyHasAuthority is for.
 */
UCLASS()
class CROWDYSDKTEST_API ASampleHostSwitch : public ASampleSwitchBase
{
	GENERATED_BODY()

public:

	ASampleHostSwitch();

protected:

	virtual void Interact_Implementation(APawn* Interactor) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sample")
	TSubclassOf<ASampleObjectEntity> WorldObjectClass;
	
	UPROPERTY()
	TObjectPtr<AActor> SpawnedEntity;
};
