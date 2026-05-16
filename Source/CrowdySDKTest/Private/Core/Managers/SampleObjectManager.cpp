// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Managers/SampleObjectManager.h"
#include "Core/Structs/Events/FSampleDestroyObject.h"
#include "Core/Structs/Events/FSampleSetObjectLocation.h"
#include "Core/Structs/Events/FSampleSetObjectRotation.h"
#include "Core/Structs/Events/FSampleSpawnObject.h"
#include "Messages/GameObjects/FGameEventNotification.h"
#include "Subsystem/CrowdySDKSubsystem.h"
#include "Utils/UEventPayloadRegistry.h"


// Sets default values
ASampleObjectManager::ASampleObjectManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void ASampleObjectManager::OnMessageReceived(TSharedRef<ICrowdyMessage> Message)
{
	switch (Message->GetType())
	{
	case ECrowdyMessageType::CLIENT_EVENT_NOTIFICATION:
		{
			UE_LOG(LogTemp, Log, TEXT("[SampleObjectManager][OnMessageReceived]: Received Client Event Notification."));
			const auto& ClientEventNotificationMessage = static_cast<const FGameEventNotification&>(*Message);
			HandleGameEvent(ClientEventNotificationMessage);
		}
	default:
		break;
	}
}

TArray<ECrowdyMessageType> ASampleObjectManager::GetSupportedResponseTypes() const
{
	return {ECrowdyMessageType::CLIENT_EVENT_NOTIFICATION};
}

TArray<FName> ASampleObjectManager::GetSupportedEventTypes() const
{
	// Must match entry names in the data asset check DA_SampleEvents
	return TArray{
		FName("SpawnObject"), 
		FName("SetObjectLocation"), 
		FName("SetObjectRotation"), 
		FName("DestroyObject")};
}

// Called when the game starts or when spawned
void ASampleObjectManager::BeginPlay()
{
	Super::BeginPlay();

	const UCrowdySDKSubsystem* CrowdySDK = GetWorld()->GetGameInstance()->GetSubsystem<UCrowdySDKSubsystem>();
	//CrowdySDK->RegisterReceptionLayer(this);
}

// Called every frame
void ASampleObjectManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASampleObjectManager::HandleGameEvent(const FGameEventNotification& Event)
{
	int32 EventType = Event.EventType;
	
	FInstancedStruct ObjectOperation;

	if (!USerializationFunctionLibrary::DeserializeEventState(Event.StateBytes, ObjectOperation))
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("[SampleObjectManager][HandleGameEvent]: Failed to deserialize payload."));
		return;
	}

	const UScriptStruct* StructType = ObjectOperation.GetScriptStruct();

	if (!StructType)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SampleObjectManager][HandleGameEvent]: Invalid script struct."));
		return;
	}

	if (StructType == FSampleSpawnObject::StaticStruct())
	{
		FSampleSpawnObject Data = ObjectOperation.Get<FSampleSpawnObject>(); // copy (safer)
		AsyncTask(ENamedThreads::GameThread, [this, Data]()
		{
			HandleSpawnObject(Data);
		});
		return;
	}

	if (StructType == FSampleDestroyObject::StaticStruct())
	{
		FSampleDestroyObject Data = ObjectOperation.Get<FSampleDestroyObject>();
		AsyncTask(ENamedThreads::GameThread, [this, Data]()
		{
			HandleDestroyObject(Data);
		});
		return;
	}

	if (StructType == FSampleSetObjectLocation::StaticStruct())
	{
		FSampleSetObjectLocation Data = ObjectOperation.Get<FSampleSetObjectLocation>();
		AsyncTask(ENamedThreads::GameThread, [this, Data]()
		{
			HandleSetObjectLocation(Data);
		});
		return;
	}

	if (StructType == FSampleSetObjectRotation::StaticStruct())
	{
		FSampleSetObjectRotation Data = ObjectOperation.Get<FSampleSetObjectRotation>();
		AsyncTask(ENamedThreads::GameThread, [this, Data]()
		{
			HandleSetObjectRotation(Data);
		});
	}
}


void ASampleObjectManager::HandleSpawnObject(const FSampleSpawnObject& SpawnObject)
{
	if (ObjectToActorMap.Contains(SpawnObject.ObjectID))
	{
		UE_LOG(LogTemp, Warning, TEXT("[SampleObjectManager][HandleSpawnObject]: Object already exists."));
		return;
	}

	const TSubclassOf<AActor>* FoundClass = ActorsToSpawn.Find(SpawnObject.ObjectType);

	if (!FoundClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SampleObjectManager][HandleSpawnObject]: No class found for type %s"),
		       *SpawnObject.ObjectType.ToString());
	}

	const FActorSpawnParameters SpawnParams;
	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(
		*FoundClass,
		SpawnObject.SpawnLocation,
		SpawnObject.SpawnRotation,
		SpawnParams
	);

	if (SpawnedActor)
	{
		ObjectToActorMap.Add(SpawnObject.ObjectID, SpawnedActor);
	}
}

void ASampleObjectManager::HandleDestroyObject(const FSampleDestroyObject& DestroyObject)
{
	if (!ObjectToActorMap.Contains(DestroyObject.ObjectID))
	{
		UE_LOG(LogTemp, Warning, TEXT("[SampleObjectManager][HandleDestroyObject]: Object doesn't exist."));
		return;
	}

	AActor* Actor = ObjectToActorMap.FindRef(DestroyObject.ObjectID);
	if (Actor)
	{
		Actor->Destroy();
		ObjectToActorMap.Remove(DestroyObject.ObjectID);
	}
}

void ASampleObjectManager::HandleSetObjectLocation(const FSampleSetObjectLocation& SetObjectLocation)
{
	if (!ObjectToActorMap.Contains(SetObjectLocation.ObjectID))
	{
		UE_LOG(LogTemp, Warning, TEXT("[SampleObjectManager][HandleSetObjectLocation]: Object doesn't exist."));
		return;
	}

	AActor* Actor = ObjectToActorMap.FindRef(SetObjectLocation.ObjectID);
	if (Actor)
	{
		Actor->SetActorLocation(SetObjectLocation.Location);
	}
}

void ASampleObjectManager::HandleSetObjectRotation(const FSampleSetObjectRotation& SetObjectRotation)
{
	if (!ObjectToActorMap.Contains(SetObjectRotation.ObjectID))
	{
		UE_LOG(LogTemp, Warning, TEXT("[SampleObjectManager][HandleSetObjectRotation]: Object doesn't exist."));
		return;
	}

	AActor* Actor = ObjectToActorMap.FindRef(SetObjectRotation.ObjectID);
	if (Actor)
	{
		Actor->SetActorRotation(SetObjectRotation.Rotation);
	}
}
