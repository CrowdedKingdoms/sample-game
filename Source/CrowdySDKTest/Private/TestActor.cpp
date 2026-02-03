// Fill out your copyright notice in the Description page of Project Settings.


#include "CrowdySDKTest/Public/TestActor.h"

#include "Kismet/GameplayStatics.h"
#include "Messages/Actor/FActorUpdateRequestMessage.h"
#include "Messages/Actor/FActorUpdateResponseMessage.h"
#include "Subsystem/CrowdySDKSubsystem.h"
#include "Utils/HelperFunctions.h"


// Sets default values
ATestActor::ATestActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ATestActor::BeginPlay()
{
	Super::BeginPlay();
	
	SDK = GetWorld()->GetGameInstance()->GetSubsystem<UCrowdySDKSubsystem>();
	
	
	SDK->RegisterReceptionLayer(this);
	
	MyActorUUID = UHelperFunctions::GetNewUUID();
	UE_LOG(LogTemp, Log, TEXT("Generated UUID %s"), *MyActorUUID)
	
}

void ATestActor::SendActorUpdates(const FVector Location) const
{
	FActorUpdateRequestMessage ActorUpdateRequestMessage;
	UE_LOG(LogTemp, Log, TEXT("Captured Location: %s"), *Location.ToCompactString());
	ActorUpdateRequestMessage.MapID = 1;
	ActorUpdateRequestMessage.ChunkX = FMath::RandRange(-5, 5);
	ActorUpdateRequestMessage.ChunkY = FMath::RandRange(-5, 5);
	ActorUpdateRequestMessage.ChunkZ = 2;
	ActorUpdateRequestMessage.ActorUUID = MyActorUUID;
	ActorUpdateRequestMessage.StateSize = sizeof(Location);
	TArray<uint8> StateBytes;
	StateBytes.SetNumUninitialized(sizeof(Location));
	FMemory::Memcpy(StateBytes.GetData(), &Location, sizeof(Location));
	ActorUpdateRequestMessage.StateBytes = StateBytes;
	
	SDK->SendMessage(ActorUpdateRequestMessage);
}

void ATestActor::ReceiveActorUpdates(const FActorUpdateNotificationMessage& ActorUpdateNotify)
{
	UE_LOG(LogTemp, Warning, TEXT("Received Actor Updates"));

	const int64 ChunkX = ActorUpdateNotify.ChunkX;
	const int64 ChunkY = ActorUpdateNotify.ChunkY;
	const int64 ChunkZ = ActorUpdateNotify.ChunkZ;
	const FString UUID = ActorUpdateNotify.UUID.ToString();
	
	if (UUID == MyActorUUID)
		SDK->TriggerUdpHeartbeat();
	
	FVector Location;  
	FMemory::Memcpy(&Location, ActorUpdateNotify.StateBytes.GetData(), sizeof(Location));
	
	UE_LOG(LogTemp, Log, TEXT("\n\nChunk (%lld %lld %lld)\nUUID (%s)\nLocation:%s"), ChunkX, ChunkY, ChunkZ, *UUID,
	       *Location.ToCompactString());
}

void ATestActor::OnMessageReceived(TSharedRef<ICrowdyMessage> Message)
{
	switch (Message->GetType())
	{
	case ECrowdyMessageType::ACTOR_UPDATE_NOTIFICATION:
		{
			const FActorUpdateNotificationMessage ActorUpdateNotificationMessage = static_cast<FActorUpdateNotificationMessage&>(*Message);
			ReceiveActorUpdates(ActorUpdateNotificationMessage);
			break;
		}
	case ECrowdyMessageType::ACTOR_UPDATE_RESPONSE:
		{
			FActorUpdateResponseMessage ActorUpdateResponseMessage = static_cast<FActorUpdateResponseMessage&>(*Message);
			UE_LOG(LogTemp, Warning, TEXT("Received actor update response"));
			break;
		}
	default: 
		UE_LOG(LogTemp, Warning, TEXT("Received unknown message type %s"), *Message->GetTypeName().ToString());
	}
}

TArray<ECrowdyMessageType> ATestActor::GetSupportedResponseTypes() const
{
	return {
		ECrowdyMessageType::ACTOR_UPDATE_NOTIFICATION,
		ECrowdyMessageType::ACTOR_UPDATE_RESPONSE
	};
}

// Called every frame
void ATestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

