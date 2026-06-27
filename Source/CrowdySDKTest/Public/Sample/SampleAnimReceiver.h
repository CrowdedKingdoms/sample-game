#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SampleAnimReceiver.generated.h"

UINTERFACE(BlueprintType, MinimalAPI)
class USampleAnimReceiver : public UInterface
{
	GENERATED_BODY()
};

/**
 * Implemented by a proxy character so the replication policy can hand it the latest
 * replicated movement inputs. The policy runs in C++; the player character is a
 * Blueprint, so this interface is the strongly-typed seam between them. The Blueprint
 * implements ApplyAnimSnapshot and stashes the values into its own AnimVelocity /
 * bAnimFalling variables, which the AnimBP reads.
 */
class ISampleAnimReceiver
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Sample|Anim")
	void ApplyAnimSnapshot(FVector Velocity, bool bIsFalling);
};
