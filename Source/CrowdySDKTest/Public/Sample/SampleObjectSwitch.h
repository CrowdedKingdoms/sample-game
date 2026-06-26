#pragma once

#include "CoreMinimal.h"
#include "Sample/SampleSwitchBase.h"
#include "SampleObjectSwitch.generated.h"

class ASampleObjectEntity;

/**
 * Walks an object through its whole life cycle on repeated interactions:
 * spawn, then move, then rotate, then destroy, then back to spawn. It shows a
 * spawned entity receiving targeted RPCs and finally being destroyed.
 */
UCLASS()
class CROWDYSDKTEST_API ASampleObjectSwitch : public ASampleSwitchBase
{
	GENERATED_BODY()

public:

	ASampleObjectSwitch();

protected:

	virtual void Interact_Implementation(APawn* Interactor) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sample")
	TSubclassOf<ASampleObjectEntity> ObjectClass;

	// Where the object appears relative to this switch.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sample")
	FVector SpawnOffset = FVector(200.f, 0.f, 100.f);

private:

	UPROPERTY()
	TWeakObjectPtr<ASampleObjectEntity> SpawnedObject;

	int32 Step = 0;
};
