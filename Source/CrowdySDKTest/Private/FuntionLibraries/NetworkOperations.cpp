// Fill out your copyright notice in the Description page of Project Settings.


#include "FuntionLibraries/NetworkOperations.h"

#include "Core/Enums/ESampleAnimState.h"
#include "Core/Enums/ESampleGameEvents.h"
#include "Core/Structs/Game/FSampleActorState.h"
#include "Messages/Actor/FActorUpdateRequestMessage.h"
#include "Messages/GameObjects/FGameEventRequest.h"
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

void UNetworkOperations::RequestAnimationStateChange(const UCrowdySDKSubsystem* CrowdySDK, const int64 ChunkX, const int64 ChunkY,
	const int64 ChunkZ, const FString& UUID, const ESampleAnimState NewAnimState)
{
	ensure(IsValid(CrowdySDK));
	
	if (!IsValid(CrowdySDK))
	{
		UE_LOG(LogTemp, Warning, TEXT("[UNetworkOperations][EnqueueActorUpdate]: CrowdySDK Reference is null."));
		return;
	}
	
	// Constructing Message
	FGameEventRequest AnimationEventRequestMessage;
	
	// Setting metadata
	AnimationEventRequestMessage.AppID = 2;
	AnimationEventRequestMessage.ChunkX = ChunkX;
	AnimationEventRequestMessage.ChunkY = ChunkY;
	AnimationEventRequestMessage.ChunkZ = ChunkZ;
	AnimationEventRequestMessage.DecayRate = ECrowdyDecayRate::No_Decay;
	AnimationEventRequestMessage.ReplicationDistance = ECrowdyReplicationDistance::Eight_Chunks;
	AnimationEventRequestMessage.UUID = UUID;
	
	// Setting Event Properties
	AnimationEventRequestMessage.EventType = static_cast<uint16>(ESampleGameEvent::ChangeAnimation);
	AnimationEventRequestMessage.StateSize = sizeof(ESampleAnimState);
	AnimationEventRequestMessage.StateBytes.Add(static_cast<uint8>(NewAnimState));
	
	
	// Enqueue Message for send
	CrowdySDK->SendMessage(AnimationEventRequestMessage);
}

void UNetworkOperations::DispatchObjectOperation(const UCrowdySDKSubsystem* CrowdySDK, const int64 ChunkX,
	const int64 ChunkY, const int64 ChunkZ, const FString& InstigatorUUID,
	const FSampleObjectOperationEvent ObjectOperationEvent)
{
	ensure(IsValid(CrowdySDK));
	
	if (!IsValid(CrowdySDK))
	{
		UE_LOG(LogTemp, Warning, TEXT("[UNetworkOperations][EnqueueActorUpdate]: CrowdySDK Reference is null."));
		return;
	}
	
	// Constructing Message
	FGameEventRequest ObjectOperation;
	
	// Setting metadata
	ObjectOperation.AppID = 2;
	ObjectOperation.ChunkX = ChunkX;
	ObjectOperation.ChunkY = ChunkY;
	ObjectOperation.ChunkZ = ChunkZ;
	ObjectOperation.DecayRate = ECrowdyDecayRate::No_Decay;
	ObjectOperation.ReplicationDistance = ECrowdyReplicationDistance::Eight_Chunks;
	ObjectOperation.UUID = InstigatorUUID;
	
	// Assigning Event Type
	ObjectOperation.EventType = static_cast<uint16>(ESampleGameEvent::ObjectOperation);
	
	// Getting State size
	ObjectOperation.StateSize = FSampleObjectOperationEvent::GetSizeByType(ObjectOperationEvent.OperationType);
	
	// Appending State
	ObjectOperation.StateBytes = FSampleObjectOperationEvent::Serialize(ObjectOperationEvent);
	
	// Dispatching Message
	CrowdySDK->SendMessage(ObjectOperation);
}
