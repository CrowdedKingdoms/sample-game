// Fill out your copyright notice in the Description page of Project Settings.


#include "Replication/Components/CrowdyObjectComponent.h"
#include "Replication/Subsystems/CrowdyObjectManager.h"
#include "Utils/HelperFunctions.h"
#include "Subsystem//CrowdyGameSession.h"
#include "Subsystem/CrowdySDKSubsystem.h"


// Sets default values for this component's properties
UCrowdyObjectComponent::UCrowdyObjectComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UCrowdyObjectComponent::BeginPlay()
{
	Super::BeginPlay();

	const UWorld* World = GetWorld();
	
	checkf(IsValid(World), TEXT("[CrowdySDK][CrowdyObjectComponent]: World is invalid."))
	
	CrowdyObjectManager = World->GetSubsystem<UCrowdyObjectManager>();
	
	checkf(IsValid(CrowdyObjectManager.Get()), TEXT("[CrowdySDK][CrowdyObjectComponent]: CrowdyObjectManager is invalid."))
	
	SDK = World->GetGameInstance()->GetSubsystem<UCrowdySDKSubsystem>();
	
	checkf(IsValid(SDK.Get()), TEXT("[CrowdySDK][CrowdyObjectComponent]: CrowdySDKSubsystem is invalid."))
	
	if (!bAutoRegister)
		return;
	
	const TObjectPtr<UCrowdyGameSession> GameSession = World->GetGameInstance()->GetSubsystem<UCrowdyGameSession>();
	
	checkf(IsValid(GameSession.Get()), TEXT("[CrowdySDK][CrowdyObjectComponent]: GameSession is invalid."));

	ObjectID = bUseDeterministicID ? UHelperFunctions::GetDeterministicID(Seed) : UHelperFunctions::GetNewID();
	
	OwnerID = GameSession->GetID();
	
	//CrowdyObjectManager->RegisterObject(ObjectID, OwnerID, ObjectPayload, GetOwner());
}


// Called every frame
void UCrowdyObjectComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                           FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

FGuid UCrowdyObjectComponent::GetObjectID() const
{
	return ObjectID;
}

FGuid UCrowdyObjectComponent::GetOwnerID() const
{
	return OwnerID;
}

bool UCrowdyObjectComponent::IsLocallyOwned() const
{
	return CrowdyObjectManager->IsLocallyOwned(ObjectID);
}

void UCrowdyObjectComponent::DispatchEventForObject(FInstancedStruct& Event) const
{
	const AActor* Actor = GetOwner();
	int64 ChunkX, ChunkY, ChunkZ;
	UHelperFunctions::GetChunkCoordinatesAtWorldLocation(Actor->GetActorLocation(), ChunkX, ChunkY, ChunkZ);
	
	SDK->DispatchGameEvent(ChunkX, ChunkY, ChunkZ, 
		ECrowdyDecayRate::No_Decay, 
		ECrowdyReplicationDistance::Eight_Chunks, 
		OwnerID, 
		Event, 
		true);
}

void UCrowdyObjectComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	//CrowdyObjectManager->UnregisterObject(ObjectID);
	Super::EndPlay(EndPlayReason);
}

