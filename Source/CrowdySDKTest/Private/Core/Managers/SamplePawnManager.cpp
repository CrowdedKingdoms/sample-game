// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Managers/SamplePawnManager.h"
#include "Core/Structs/Events/FChangeAnimState.h"
#include "Interfaces/ReplicatedActor.h"
#include "Utils/CrowdyUtilities.h"


// Sets default values
ASamplePawnManager::ASamplePawnManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ASamplePawnManager::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ASamplePawnManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void ASamplePawnManager::ChangeInstanceAnimation(const FChangeAnimState& NewAnimState)
{
	bool bIsValid = false;
	
	if (!bIsValid)
	{
		return;
	}
	
	IReplicatedActor::Execute_ChangeAnimationState(this, NewAnimState.NewState);
}


