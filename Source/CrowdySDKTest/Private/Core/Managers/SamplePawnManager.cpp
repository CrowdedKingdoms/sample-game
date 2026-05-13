// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Managers/SamplePawnManager.h"
#include "Core/Structs/Game/FSampleActorUpdate.h"
#include "Interfaces/ReplicatedActor.h"
#include "Replication/Subsystems/CrowdyActorPoolSubsystem.h"
#include "Replication/Subsystems/CrowdyActorTracker.h"


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
	
	const UWorld* World = GetWorld();
	
	if (!IsValid(World))
	{
		UE_LOG(LogTemp, Error, TEXT("[Pawn Manager]: Invalid world."));
		return;
	}
	
	ActorPoolSubsystem = World->GetSubsystem<UCrowdyActorPoolSubsystem>();

	check(IsValid(ActorPoolSubsystem))
	
	if (!IsValid(ActorPoolSubsystem))
	{
		UE_LOG(LogTemp, Error, TEXT("[Pawn Manager]: Invalid actor pool subsystem."));
		return;
	}
	
	
}

// Called every frame
void ASamplePawnManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}



void ASamplePawnManager::ChangeAnimation(const FGuid& UUID, const ESampleAnimState NewAnimationState) const
{
	
	AActor* Actor = ActorPoolSubsystem->FindActor(UUID);
	
	if (!IsValid(Actor))
		return;
	
	// Dispatch the call to update the animation state inside the actor. Check BP_ReplicatedActor in Content/Replication for implementation
	IReplicatedActor::Execute_ChangeAnimationState(Actor, NewAnimationState);
}


