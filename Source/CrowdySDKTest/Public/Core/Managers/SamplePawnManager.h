// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Structs/Game/FSampleActorUpdate.h"
#include "GameFramework/Actor.h"
#include "Replication/Subsystems/CrowdyActorTracker.h"
#include "StructUtils/InstancedStruct.h"
#include "SamplePawnManager.generated.h"

class UCrowdyActorPoolSubsystem;
enum class ESampleAnimState : uint8;

UCLASS(BlueprintType)
class CROWDYSDKTEST_API ASamplePawnManager : public AActor
{
	GENERATED_BODY()

public:
	
	// Sets default values for this actor's properties
	ASamplePawnManager();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void ChangeAnimation(const FGuid& UUID, const ESampleAnimState NewAnimationState) const;
	
protected:
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
private:
	
	UPROPERTY()
	UCrowdyActorPoolSubsystem* ActorPoolSubsystem;
	
};
