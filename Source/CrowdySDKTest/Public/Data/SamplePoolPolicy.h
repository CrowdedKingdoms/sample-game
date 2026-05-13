// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/CrowdyActorPoolPolicy.h"
#include "SamplePoolPolicy.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class CROWDYSDKTEST_API USamplePoolPolicy : public UCrowdyActorPoolPolicy
{
	GENERATED_BODY()
public:
	
	virtual void OnActorActivated_Implementation(AActor* Actor, const FInstancedStruct& InitialState) override;
	virtual void OnActorDeactivated_Implementation(AActor* Actor) override;
	virtual void OnActorPooled_Implementation(AActor* Actor) override;

};
