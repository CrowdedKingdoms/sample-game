// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Enums/ESampleAnimState.h"
#include "UObject/Interface.h"
#include "ReplicatedActor.generated.h"

// This class does not need to be modified.
UINTERFACE(BlueprintType)
class UReplicatedActor : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class CROWDYSDKTEST_API IReplicatedActor
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	// Function that will be called on the actor when we receive an animation update message for it
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category= "Sample Replicated Actor Interface")
	void ChangeAnimationState(const ESampleAnimState NewAnimationState);
};
