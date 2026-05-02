// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Managers/SampleObjectManager.h"

#include "Core/Enums/ESampleGameEvents.h"
#include "Core/Structs/Game/FSampleObjectOperationEvent.h"
#include "Messages/GameObjects/FGameEventNotification.h"
#include "Subsystem/CrowdySDKSubsystem.h"


// Sets default values
ASampleObjectManager::ASampleObjectManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void ASampleObjectManager::OnMessageReceived(TSharedRef<ICrowdyMessage> Message)
{
	switch(Message->GetType())
	{
	case ECrowdyMessageType::CLIENT_EVENT_NOTIFICATION:
		{
			const auto& ClientEventNotificationMessage = static_cast<const FGameEventNotification&>(*Message);
			
			ESampleGameEvent EventType = static_cast<ESampleGameEvent>(ClientEventNotificationMessage.EventType);
			
			// TODO: Cleanup later
			switch (EventType)
			{
				case ESampleGameEvent::ObjectOperation:
					{
						FSampleObjectOperationEvent ObjectOperationEvent;
						
						if (!FSampleObjectOperationEvent::Deserialize(ClientEventNotificationMessage.StateBytes, ObjectOperationEvent))
						{
							UE_LOG(LogTemp, Warning, TEXT("[ASampleObjectManager][OnMessageReceived]: Failed to deserialize Object Operation Event"));
						}
						
						AsyncTask(ENamedThreads::GameThread, [this, ObjectOperationEvent]()
						{
							const FVector SpawnLoc = ObjectOperationEvent.Location;
							FRotator SpawnRot = ObjectOperationEvent.Rotation;
							const FActorSpawnParameters SpawnParams;
							AActor* Actor = GetWorld()->SpawnActor<AActor>(
								ActorClassToSpawn,
								SpawnLoc,
								FRotator::ZeroRotator,
								SpawnParams
								);
						});
						
					}
				break;
				
				default:
				break;
			}
			
		}
		break;
		
		default:
		break;
	}
}

TArray<ECrowdyMessageType> ASampleObjectManager::GetSupportedResponseTypes() const
{
	return {ECrowdyMessageType::CLIENT_EVENT_NOTIFICATION};
}

// Called when the game starts or when spawned
void ASampleObjectManager::BeginPlay()
{
	Super::BeginPlay();

	const UCrowdySDKSubsystem* CrowdySDK = GetWorld()->GetGameInstance()->GetSubsystem<UCrowdySDKSubsystem>();
	CrowdySDK->RegisterReceptionLayer(this);
	
}

// Called every frame
void ASampleObjectManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

