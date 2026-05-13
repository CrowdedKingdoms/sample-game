// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Managers/SamplePlayerManager.h"
#include "Core/Managers/SamplePawnManager.h"
#include "Core/Structs/Events/FChangeAnimState.h"
#include "Core/Structs/Game/FSampleActorState.h"
#include "Core/Structs/Game/FSampleActorUpdate.h"
#include "Core/UDP/Interfaces/ICrowdyMessage.h"
#include "Messages/Actor/FActorUpdateNotificationMessage.h"
#include "Messages/GameObjects/FGameEventNotification.h"
#include "Subsystem/CrowdyGameSession.h"
#include "Subsystem/CrowdySDKSubsystem.h"
#include "Subsystem/CrowdyWorkerThreadsSubsystem.h"


// Sets default values
ASamplePlayerManager::ASamplePlayerManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ASamplePlayerManager::BeginPlay()
{
	Super::BeginPlay();

	// Get Reference to SDK; since it's a Game Instance Subsystem, it's available system-wide
	CrowdySDK = GetWorld()->GetGameInstance()->GetSubsystem<UCrowdySDKSubsystem>();

	
	// Validation Checks
	const bool bIsSDKValid = IsValid(CrowdySDK);
	const bool bIsPawnManagerValid = IsValid(PawnManager);
	

	// Assertion to check if SDK reference is valid, since we cannot proceed without this
	check(bIsSDKValid)
	
	// Return early if any system is invalid
	if (!bIsSDKValid)
	{
		UE_LOG(LogTemp, Error, TEXT("[SamplePlayerManager]: Invalid SDK reference."));
		return;
	}

	if (!bIsPawnManagerValid)
		return;
	
	// This registers this actor as a reception layer for messages. Without this, the SDK doesn't dispatch messages to this actor
	CrowdySDK->RegisterReceptionLayer(this);
}

void ASamplePlayerManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}


// Called every frame
void ASamplePlayerManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASamplePlayerManager::OnMessageReceived(TSharedRef<ICrowdyMessage> Message)
{
	switch (Message->GetType())
	{
	case ECrowdyMessageType::CLIENT_EVENT_NOTIFICATION:
		{
			// We perform a simple type cast and dispatch the game event to the HandleGameEvent()
			const auto& GameEventNotification = static_cast<const FGameEventNotification&>(*Message);
			HandleGameEvent(GameEventNotification);
			break;
		}

	// Added a default case in case some other message leaks here
	default:
		break;
	}
}

TArray<ECrowdyMessageType> ASamplePlayerManager::GetSupportedResponseTypes() const
{
	/* 
	* This tells the SDK which messages should be dispatched to this particular actor/object
	* This doesn't mean that only this actor will get these message types; multiple actors/objects can subscribe to the same
	* message types.
	* In addition to the actor updates, we are also processing Game Events related to the actors here, so we are also expecting 
	* those messages to be processed in here.
	*/
	return TArray
	{
		ECrowdyMessageType::CLIENT_EVENT_NOTIFICATION
	};
}


// This override tells the SDK that I want to handle this specific type of game event from the registered ones in the Data Asset
// Check DA_SampleEvents
// The Names must match in the data asset and here, otherwise it will fail
TArray<FName> ASamplePlayerManager::GetSupportedEventTypes() const
{
	return TArray{FName("ChangeAnimState")};
}


void ASamplePlayerManager::HandleGameEvent(const FGameEventNotification& GameEventNotification) const
{
	// "State" is now deserialized FInstancedStruct ready to be used. So we get it's type 
	const UScriptStruct* StructType = GameEventNotification.State.GetScriptStruct();
	
	// If type is invalid we return
	if (!StructType)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SamplePlayerManager][HandleGameEvent]: Invalid script struct."));
		return;
	}
	
	// Since we're only handling FChangeAnimState in here, we ensure that it's the one we receive.
	if (StructType == FChangeAnimState::StaticStruct())
	{
		FChangeAnimState Data = GameEventNotification.State.Get<FChangeAnimState>();
		
		FGuid UUID = Data.TargetID;
		ESampleAnimState AnimState = Data.NewState;
		
		AsyncTask(ENamedThreads::GameThread, [this, UUID, AnimState]()
		{
			PawnManager->ChangeAnimation(UUID, AnimState);
		});
	}
}