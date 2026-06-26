#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SampleInteractable.generated.h"

UINTERFACE(BlueprintType, MinimalAPI)
class USampleInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * The interact contract shared by every example switch. A pawn that walks into a
 * switch (or a Blueprint that calls Interact) triggers the switch's behaviour. It is
 * a BlueprintNativeEvent so the sample's Blueprint switches can implement it in a
 * graph while the C++ switches implement it in code, from the same entry point.
 */
class ISampleInteractable
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Sample|Interaction")
	void Interact(APawn* Interactor);
};
