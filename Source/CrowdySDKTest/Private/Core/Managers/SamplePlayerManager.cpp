// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Managers/SamplePlayerManager.h"

#include "Core/Enums/ESampleGameEvents.h"
#include "Core/Managers/SamplePawnManager.h"
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
	PrimaryActorTick.bCanEverTick = true;
	SetupUpdateQueues();
}

// Called when the game starts or when spawned
void ASamplePlayerManager::BeginPlay()
{
	Super::BeginPlay();

	// Get Reference to SDK, since it's a Game Instance Subsystem it's available system-wide
	CrowdySDK = GetWorld()->GetGameInstance()->GetSubsystem<UCrowdySDKSubsystem>();

	// Get Reference to Crowdy Game Session
	CrowdyGameSession = GetWorld()->GetGameInstance()->GetSubsystem<UCrowdyGameSession>();

	// Get Reference to Worker Thread Subsystem
	WorkerThreadsSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UCrowdyWorkerThreadsSubsystem>();

	// Validation Checks
	const bool bIsSDKValid = IsValid(CrowdySDK);
	const bool bIsGameSessionValid = IsValid(CrowdyGameSession);
	const bool bIsPawnManagerValid = IsValid(PawnManager);
	const bool bIsWorkerThreadSubsystemValid = IsValid(WorkerThreadsSubsystem);

	// Assertion to check if SDK reference is valid, since we cannot proceed without this
	check(bIsSDKValid)
	check(bIsGameSessionValid)

	// Return early if any system is invalid
	if (!bIsSDKValid || !bIsGameSessionValid)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("%s is invalid. Returning."),
			!bIsSDKValid ? TEXT("CrowdySDK") : TEXT("Crowdy Game Session")
		);
		return;
	}

	if (!bIsPawnManagerValid)
		return;

	if (!bIsWorkerThreadSubsystemValid)
		return;


	// This informs parser in advance the size of State to expect. This can be set per map/game mode
	// In this case, we are expecting to receive FSampleActorUpdate so we just get it's size directly as that is 
	// what we pass in the payload 
	CrowdySDK->SetExpectedActorUpdateStateSize(sizeof(FSampleActorState));

	// This registers this actor as a reception layer for messages. Without this, the SDK doesn't dispatch messages to this actor
	CrowdySDK->RegisterReceptionLayer(this);

	GetWorldTimerManager().SetTimer(TimeoutCheckTimerHandle, this, &ASamplePlayerManager::CheckForActorTimeouts, 5.0f,
	                                true);
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

void ASamplePlayerManager::SetupUpdateQueues()
{
	// We'll use a minimum of 1 if the resultant goes 0 or negative
	NumberOfWorkerThreads = FMath::Max(1, FPlatformMisc::NumberOfCores() - 2);

	// Setting equal to number of threads available
	UpdateQueues.SetNum(NumberOfWorkerThreads);
	WorkerTaskScheduledFlags.SetNum(NumberOfWorkerThreads);

	// Initialization
	for (int32 i = 0; i < NumberOfWorkerThreads; i++)
	{
		UpdateQueues[i] = MakeUnique<TQueue<FSampleActorUpdate, EQueueMode::Mpsc>>();
		WorkerTaskScheduledFlags[i].store(false);
	}

	// We expect there to be at least 200 Actors 
	LastUpdateTimes.Reserve(200);
}

void ASamplePlayerManager::OnMessageReceived(TSharedRef<ICrowdyMessage> Message)
{
	switch (Message->GetType())
	{
	case ECrowdyMessageType::ACTOR_UPDATE_NOTIFICATION:
		{
			const auto& ActorUpdateNotificationMessage = static_cast<const FActorUpdateNotificationMessage&>(*Message);

			FSampleActorState State;

			if (!FSampleActorState::Deserialize(ActorUpdateNotificationMessage.StateBytes, State))
			{
				UE_LOG(LogTemp, Warning,
				       TEXT("[SamplePlayerManager][OnMessageReceived]: Failed to deserialize update"));
				return;
			}

			// We check to see if it's our own update
			// We must trigger a UDP Heartbeat, that ensures our connection is alive.
			if (ActorUpdateNotificationMessage.UUID == CrowdyGameSession->GetUUID())
			{
				// Trigger the UDP Heartbeat
				CrowdySDK->TriggerUdpHeartbeat();

				// If Owner Updates are disabled, we return early and don't enqueue owner updates for processing
				if (!bEnableOwnerGhost)
					return;
			}

			// We construct an update of this format, since timestamp, UUID are required for tracking in the replicated actor manager
			// We only do bookkeeping and dispatching here in Player Manager
			// So we extract state, timestamp and UUID. (Converted to FGuid for faster comparison.)
			FSampleActorUpdate Update;
			Update.State = State;
			Update.ServerTimestamp = ActorUpdateNotificationMessage.Timestamp;
			Update.UUID = USerializationFunctionLibrary::ToGuid(ActorUpdateNotificationMessage.UUID);

			// We pass the update data to the handler
			HandleActorUpdateMessage(Update);

			break;
		}
	case ECrowdyMessageType::ACTOR_UPDATE_RESPONSE:
		{
			UE_LOG(LogTemp, Error, TEXT("Actor Update Response Error received."));
			break;
		}
	case ECrowdyMessageType::CLIENT_EVENT_NOTIFICATION:
		{
			// We perform a simple type cast and dispatch the game event to the HandleGameEvent()
			const auto& GameEventNotification = static_cast<const FGameEventNotification&>(*Message);
			HandleGameEvent(GameEventNotification);
			break;
		}

	// Added a default case in case some other message leaks here
	default:
		UE_LOG(LogTemp, Error, TEXT("Unknown message type. %s"), *Message->GetTypeName().ToString());
		break;
	}
}

TArray<ECrowdyMessageType> ASamplePlayerManager::GetSupportedResponseTypes() const
{
	/* 
	* This tells the SDK which messages should be dispatched to this particular actor/object
	* This doesn't mean that only this actor will get these message types, multiple actors/objects can subscribe to same
	* message types.
	* In this sample we're only aiming at this class to handle the Actor Updates, so we're only providing implementation here.
	* In addition to the actor updates, we are also processing Game Events related to the actors here, so we are also expecing 
	* those messages to be processed in here.
	*/
	return TArray
	{
		ECrowdyMessageType::ACTOR_UPDATE_NOTIFICATION,
		ECrowdyMessageType::ACTOR_UPDATE_RESPONSE,
		ECrowdyMessageType::CLIENT_EVENT_NOTIFICATION
	};
}

void ASamplePlayerManager::SetOwnerGhostEnabled(const bool bEnable)
{
	bEnableOwnerGhost = bEnable;
}

void ASamplePlayerManager::HandleActorUpdateMessage(const FSampleActorUpdate& Update)
{
	/* This returns a deterministic index so that workers only pull 
	 * from their own queues since it's a single consumer model
	*/
	const int32 WorkerIndex = GetTypeHash(Update.UUID) % NumberOfWorkerThreads;

	// Simply Enqueue the update
	UpdateQueues[WorkerIndex]->Enqueue(Update);

	bool bExpected = false;

	if (WorkerTaskScheduledFlags[WorkerIndex].compare_exchange_strong(bExpected, true))
	{
		// Create tasks
		TArray<TFunction<void()>> Tasks;
		Tasks.Add([this, WorkerIndex]()
		{
			ProcessUpdateQueue(WorkerIndex);
			WorkerTaskScheduledFlags[WorkerIndex] = false;
		});

		// Submit to Crowdy Worker Pool to execute
		WorkerThreadsSubsystem->EnqueueTasks(Tasks);
	}
}

void ASamplePlayerManager::HandleGameEvent(const FGameEventNotification& GameEventNotification) const
{
	// We do a simple type cast and check if the event is of our defined type -- see ESampleGameEvents.h
	const ESampleGameEvent EventType = static_cast<ESampleGameEvent>(GameEventNotification.EventType);

	switch (EventType)
	{
	case ESampleGameEvent::ChangeAnimation:
		{
			// We first take the UUID of the instigator, the convention is to use it as the target UUID
			const FGuid TargetUUID = USerializationFunctionLibrary::ToGuid(GameEventNotification.UUID);
			
			// We take the first byte of the payload, since that represents the anim state and cast it to our Enum
			const ESampleAnimState NewAnimState = static_cast<ESampleAnimState>(GameEventNotification.StateBytes[0]);
			
			// We launch the task to the game thread, since Change Animation Operation is not thread-safe
			UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, TargetUUID, NewAnimState]
			{
				PawnManager->ChangeAnimation(TargetUUID, NewAnimState);
			}, LowLevelTasks::ETaskPriority::BackgroundNormal, UE::Tasks::EExtendedTaskPriority::GameThreadNormalPri);

			break;
		}
	default:
		UE_LOG(LogTemp, Error, TEXT("Unknown Event Type received %d"), EventType);
	}
}

void ASamplePlayerManager::ProcessUpdateQueue(const int32 WorkerIndex)
{
	TArray<FSampleActorUpdate> LocalUpdateBatch;
	LocalUpdateBatch.Reserve(MaxUpdatesPerBatch);

	const double BatchStartTime = FPlatformTime::Seconds();

	while (true)
	{
		FSampleActorUpdate Update;

		// Drain queue
		while (LocalUpdateBatch.Num() < MaxUpdatesPerBatch &&
			UpdateQueues[WorkerIndex]->Dequeue(Update))
		{
			LocalUpdateBatch.Add(MoveTemp(Update));
		}

		const double ElapsedTime = FPlatformTime::Seconds() - BatchStartTime;

		// Exit conditions
		const bool bBatchFull = LocalUpdateBatch.Num() >= MaxUpdatesPerBatch;
		const bool bHasWork = !LocalUpdateBatch.IsEmpty();
		const bool bTimeout = ElapsedTime >= MaxBatchWaitTime;

		if (bBatchFull || (bHasWork && bTimeout))
		{
			break;
		}

		if (!bHasWork && bTimeout)
		{
			return;
		}

		FPlatformProcess::SleepNoStats(0.0005f);
	}

	if (LocalUpdateBatch.IsEmpty())
		return;

	TArray<const FSampleActorUpdate*> UpdatesForExisting;
	TArray<const FSampleActorUpdate*> NewUpdates;

	{
		FReadScopeLock R(UUIDLock);

		for (const FSampleActorUpdate& Update : LocalUpdateBatch)
		{
			const FGuid& UUID = Update.UUID;

			if (ReplicatedPlayers.Contains(UUID))
				UpdatesForExisting.Add(&Update);
			else if (!PendingSpawns.Contains(UUID))
				NewUpdates.Add(&Update);
		}
	}

	for (const FSampleActorUpdate* Update : UpdatesForExisting)
	{
		PawnManager->AppendInstanceUpdate(*Update);
	}

	if (!NewUpdates.IsEmpty())
	{
		TArray<FSampleActorUpdate> NewUpdatesCopy;
		NewUpdatesCopy.Reserve(NewUpdates.Num());
		{
			FWriteScopeLock W(UUIDLock);

			for (const FSampleActorUpdate* Update : NewUpdates)
			{
				PendingSpawns.Add(Update->UUID);
				NewUpdatesCopy.Add(*Update);
			}
		}

		UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, Updates = MoveTemp(NewUpdatesCopy)]
		{
			for (const FSampleActorUpdate& Update : Updates)
			{
				const FGuid& UUID = Update.UUID;
				PawnManager->AddInstance(UUID, Update);
			}

			{
				FWriteScopeLock W(UUIDLock);
				for (const FSampleActorUpdate& Update : Updates)
				{
					PendingSpawns.Remove(Update.UUID);
					ReplicatedPlayers.Add(Update.UUID);
				}
			}
		}, LowLevelTasks::ETaskPriority::Normal, UE::Tasks::EExtendedTaskPriority::GameThreadNormalPri);
	}

	{
		const double CurrentTime = FPlatformTime::Seconds();
		FWriteScopeLock W(LastUpdateLock);

		for (const FSampleActorUpdate& Update : LocalUpdateBatch)
			LastUpdateTimes.FindOrAdd(Update.UUID) = CurrentTime;
	}
}

void ASamplePlayerManager::CheckForActorTimeouts()
{
	const float CurrentTime = FPlatformTime::Seconds();
	TArray<FGuid> TimedOutActors;
	TimedOutActors.Reserve(100);

	{
		FReadScopeLock R(TimeoutLock);
		for (const auto& Pair : LastUpdateTimes)
		{
			if (CurrentTime - Pair.Value > ActorTimeoutThreshold)
				TimedOutActors.Add(Pair.Key);
		}
	}

	if (TimedOutActors.IsEmpty())
		return;

	{
		FWriteScopeLock W(TimeoutLock);
		for (const FGuid& UUID : TimedOutActors)
			LastUpdateTimes.Remove(UUID);
	}

	ProcessTimedOutActors(TimedOutActors);
}

void ASamplePlayerManager::ProcessTimedOutActors(const TArray<FGuid>& TimedOutActors)
{
	if (!IsValid(this))
		return;

	{
		FWriteScopeLock W(UUIDLock);
		ReplicatedPlayers.Empty();
	}

	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, TimedOutActors]
	{
		for (int32 Index = 0; Index < TimedOutActors.Num(); Index++)
		{
			PawnManager->DestroyInstance(TimedOutActors[Index]);
		}
	}, LowLevelTasks::ETaskPriority::Normal, UE::Tasks::EExtendedTaskPriority::GameThreadNormalPri);
}
