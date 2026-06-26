// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SamplePawnManager.generated.h"

struct FChangeAnimState;
enum class ESampleAnimState : uint8;

UCLASS(BlueprintType)

// Just add this interface "ICrowdyEventReceiver", so the registry can detect this actor for event reception
class CROWDYSDKTEST_API ASamplePawnManager : public AActor
{
	GENERATED_BODY()

public:
	
	// Sets default values for this actor's properties
	ASamplePawnManager();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Marking this event as Crowdy Event so that it can be picked up by the registry at runtime. 
	// Currently, there is no filtering for actors, so that has to be done manually. We currently include the Actor Identification data 
	// in the payload and the manually check against it in the function definition.
	UFUNCTION()
	void ChangeInstanceAnimation(const FChangeAnimState& NewAnimState);
	
protected:
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
};
