// Fill out your copyright notice in the Description page of Project Settings.


#include "FuntionLibraries/NetworkOperations.h"
#include "Core/Structs/Game/FSampleActorState.h"
#include "Messages/Actor/FActorUpdateRequestMessage.h"
#include "Subsystem/CrowdySDKSubsystem.h"


void UNetworkOperations::EnqueueActorUpdate(const UCrowdySDKSubsystem* CrowdySDK, const int64 ChunkX, const int64 ChunkY, const int64 ChunkZ, const FString& UUID, const FSampleActorState& ActorState)
{
	ensure(IsValid(CrowdySDK));
	
	if (!IsValid(CrowdySDK))
	{
		UE_LOG(LogTemp, Warning, TEXT("[UNetworkOperations][EnqueueActorUpdate]: CrowdySDK Reference is null."));
		return;
	}
	
	// Construct an actor update message
	FActorUpdateRequestMessage ActorUpdateRequestMessage;
	ActorUpdateRequestMessage.AppID = 2;
	
	// Set the chunk coordinate
	ActorUpdateRequestMessage.ChunkX = ChunkX;
	ActorUpdateRequestMessage.ChunkY = ChunkY;
	ActorUpdateRequestMessage.ChunkZ = ChunkZ;
	
	// Configure the decay rate and replication distance (Up to 8 chunks max)
	ActorUpdateRequestMessage.DecayRate = ECrowdyDecayRate::No_Decay;
	ActorUpdateRequestMessage.ReplicationDistance = ECrowdyReplicationDistance::Eight_Chunks;

	// Assign UUID for this actor
	ActorUpdateRequestMessage.UUID = UUID;
	
	// Set the state size, helps in deserialization when receiving
	ActorUpdateRequestMessage.StateSize = FSampleActorState::GetStateSize();
	
	// Serialize the state
	ActorUpdateRequestMessage.StateBytes = FSampleActorState::Serialize(ActorState);
	
	// Enqueue Message for send
	CrowdySDK->SendMessage(ActorUpdateRequestMessage);
	
}
