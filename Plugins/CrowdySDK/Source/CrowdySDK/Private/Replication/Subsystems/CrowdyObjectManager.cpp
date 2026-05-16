// Fill out your copyright notice in the Description page of Project Settings.


#include "Replication/Subsystems/CrowdyObjectManager.h"
#include "Core/UDP/Interfaces/ICrowdyMessage.h"
#include "Messages/GameObjects/FGameEventNotification.h"
#include "Replication/ObjectHandler/CrowdyObjectEventHandler.h"
#include "Subsystem/CrowdyGameSession.h"
#include "Subsystem/CrowdySDKSubsystem.h"
#include "Utils/CrowdySDKDeveloperSettings.h"
#include "Utils/HelperFunctions.h"

void UCrowdyObjectManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	const UWorld* World = GetWorld();
	
	checkf(IsValid(World), TEXT("World is invalid"));

	const UCrowdySDKSubsystem* SDK = World->GetGameInstance()->GetSubsystem<UCrowdySDKSubsystem>();
	const UCrowdyGameSession* GameSession = World->GetGameInstance()->GetSubsystem<UCrowdyGameSession>();
	
	
	checkf(IsValid(SDK), TEXT("CrowdySDKSubsystem is invalid"));
	checkf(IsValid(GameSession), TEXT("GameSession is invalid"));
	
	// TODO: Remove and fix the injection here
	SupportedGameEvents.Add(FName("SpawnObject"));
	LoadSupportedStructTypes();
	LoadConfig();
	
	if (!SDK->IsLayerRegistered(this))
		SDK->RegisterReceptionLayer(this);
	
	CachedLocalPlayerID = GameSession->GetID();
}

void UCrowdyObjectManager::Deinitialize()
{
	Super::Deinitialize();
}

bool UCrowdyObjectManager::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}

	const UWorld* World = Outer ? Outer->GetWorld() : nullptr;
	if (!World)
	{
		return false;
	}

	// PIE or Game only
	if (World->WorldType != EWorldType::PIE &&
		World->WorldType != EWorldType::Game)
	{
		return false;
	}
	
	return true;
}

void UCrowdyObjectManager::OnMessageReceived(TSharedRef<ICrowdyMessage> Message)
{
	switch (Message->GetType())
	{
	case ECrowdyMessageType::CLIENT_EVENT_NOTIFICATION:
		{
			UE_LOG(LogTemp, Log, TEXT("[CrowdyObjectManager]: Received Client Event Notification."));
			const FGameEventNotification& GameEvent =
				static_cast<const FGameEventNotification&>(*Message);

			const UScriptStruct* StructType = GameEvent.State.GetScriptStruct();
			if (!StructType) break;

			// Whitelist check, ignore anything we don't support
			if (!SupportedStructTypes.Contains(StructType)) break;

			// Route to handler, handler decides what to do with it entirely
			if (UCrowdyObjectEventHandler* Handler = FindHandler(GameEvent.State))
				Handler->OnObjectEvent(USerializationFunctionLibrary::ToGuid(GameEvent.UUID), GameEvent.State);
		}
		break;
	default:
		break;
	}
}

TArray<ECrowdyMessageType> UCrowdyObjectManager::GetSupportedResponseTypes() const
{
	return {ECrowdyMessageType::CLIENT_EVENT_NOTIFICATION};
}

TArray<FName> UCrowdyObjectManager::GetSupportedEventTypes() const
{
	return SupportedGameEvents;
}

void UCrowdyObjectManager::RegisterObject(const FGuid ObjectID, const FGuid& OwnerID, UPARAM(ref) FInstancedStruct& RegistrationPayload, AActor* Actor) const
{
	if (!IsValid(Actor)) return;
	
	FObjectEntry Entry;
	Entry.ObjectID = ObjectID;
	Entry.OwnerID = OwnerID;
	Entry.Actor = Actor;
	
	if (UCrowdyObjectEventHandler* Handler = FindHandler(RegistrationPayload))
		Handler->OnObjectRegistered(ObjectID, OwnerID, RegistrationPayload, Actor, IsLocallyOwned(ObjectID));
}

void UCrowdyObjectManager::UnregisterObject(const FGuid& ObjectID)
{
	const FObjectEntry* Entry = IDToObject.Find(ObjectID);
	
	if (!Entry) return;

	if (UCrowdyObjectEventHandler* Handler = FindHandler(Entry->Payload))
		Handler->OnObjectUnregistered(ObjectID);

	if (Entry->Actor.IsValid())
		ActorToID.Remove(Entry->Actor.Get());

	IDToObject.Remove(ObjectID);
}

void UCrowdyObjectManager::RouteObjectEvent(const FGuid& UUID, const FInstancedStruct& EventPayload)
{
	if (!IDToObject.Contains(UUID)) return;
	
	if (UCrowdyObjectEventHandler* Handler = FindHandler(EventPayload))
		Handler->OnObjectEvent(UUID, EventPayload);
}

void UCrowdyObjectManager::RegisterEventHandler(UCrowdyObjectEventHandler* Handler)
{
	if (!IsValid(Handler)) return;

	TArray<UScriptStruct*> HandledTypes = Handler->GetHandledStructTypes();

	for (UScriptStruct* StructType : HandledTypes)
	{
		if (!IsValid(StructType)) continue;

		// Only register if this struct type is in our supported whitelist
		if (!SupportedStructTypes.Contains(StructType))
		{
			UE_LOG(LogTemp, Warning,
				TEXT("[CrowdyObjectManager]: Handler registered for '%s' but it's not in the supported event registry. Skipping."),
				*StructType->GetName());
			continue;
		}

		EventHandlers.Add(StructType, Handler);
	}
}

AActor* UCrowdyObjectManager::FindActor(const FGuid& ObjectID) const
{
	const FObjectEntry* Entry = IDToObject.Find(ObjectID);
	return Entry && Entry->Actor.IsValid() ? Entry->Actor.Get() : nullptr;
}

FGuid UCrowdyObjectManager::FindID(const AActor* Actor) const
{
	const FGuid* UUID = ActorToID.Find(Actor);
	return UUID ? *UUID : FGuid{};
}

bool UCrowdyObjectManager::IsLocallyOwned(const FGuid& ObjectID) const
{
	const FObjectEntry* Entry = IDToObject.Find(ObjectID);
	return Entry && Entry->OwnerID == CachedLocalPlayerID;
}

bool UCrowdyObjectManager::IsLocallyOwned(const AActor* Actor) const
{
	return IsLocallyOwned(FindID(Actor));
}

UCrowdyObjectEventHandler* UCrowdyObjectManager::FindHandler(const FInstancedStruct& Payload) const
{
	const UScriptStruct* StructType = Payload.GetScriptStruct();
	if (!StructType) return nullptr;

	const TObjectPtr<UCrowdyObjectEventHandler>* Handler = EventHandlers.Find(StructType);
	return Handler ? Handler->Get() : nullptr;
}

void UCrowdyObjectManager::LoadSupportedStructTypes()
{
	const UCrowdySDKDeveloperSettings* DevSettings = GetDefault<UCrowdySDKDeveloperSettings>();
	if (!DevSettings) return;
	
	const UEventPayloadType* Registry = DevSettings->EventPayloadDataAsset.LoadSynchronous();
	if (!Registry) return;
	
	for (const FEventPayloadTypeEntry& Entry : Registry->Entries)
	{
		// Only care about names we've been told to support
		if (!SupportedGameEvents.Contains(Entry.EventName))
			continue;

		if (!IsValid(Entry.EventType))
			continue;

		SupportedStructTypes.Add(Entry.EventType);
	}
}

void UCrowdyObjectManager::LoadConfig()
{
	const UCrowdySDKDeveloperSettings* DevSettings = GetDefault<UCrowdySDKDeveloperSettings>();
    if (!DevSettings) return;

    const UWorld* World = GetWorld();
    if (!IsValid(World)) return;

    FString CurrentMap = FPackageName::GetShortName(World->GetOutermost()->GetName());

#if WITH_EDITOR
    if (World->WorldType == EWorldType::Editor || World->WorldType == EWorldType::PIE)
        CurrentMap = World->RemovePIEPrefix(CurrentMap);
#endif

    // Load event name → struct type registry
    const UEventPayloadType* Registry = DevSettings->EventPayloadDataAsset.LoadSynchronous();
    if (!IsValid(Registry)) return;

    TMap<FName, UScriptStruct*> NameToStruct;
    for (const FEventPayloadTypeEntry& Entry : Registry->Entries)
    {
        if (Entry.EventName != NAME_None && IsValid(Entry.EventType))
            NameToStruct.Add(Entry.EventName, Entry.EventType);
    }

    // Find config for this map
    for (const auto& [WorldPtr, Config] : DevSettings->ObjectManagerConfigs)
    {
        if (WorldPtr.IsNull()) continue;

        const FString AllowedMap = FPackageName::GetShortName(WorldPtr.GetAssetName());
        if (AllowedMap != CurrentMap) continue;

        for (const FCrowdyObjectHandlerBinding& Binding : Config.HandlerBindings)
        {
        	if (Binding.EventName == NAME_None) continue;

        	UScriptStruct** StructType = NameToStruct.Find(Binding.EventName);
        	if (!StructType) continue;

        	// Load class and instantiate with this subsystem as outer
        	TSubclassOf<UCrowdyObjectEventHandler> HandlerClass = Binding.HandlerClass.LoadSynchronous();
        	if (!HandlerClass)
        	{
        		UE_LOG(LogTemp, Warning,
					TEXT("[CrowdyObjectManager]: HandlerClass is null for EventName '%s'. Skipping."),
					*Binding.EventName.ToString());
        		continue;
        	}

        	UCrowdyObjectEventHandler* Handler = NewObject<UCrowdyObjectEventHandler>(this, HandlerClass);
        	if (!IsValid(Handler)) continue;

        	SupportedStructTypes.Add(*StructType);
        	EventHandlers.Add(*StructType, Handler);
        }

        return;
    }

    UE_LOG(LogTemp, Warning,
        TEXT("[CrowdyObjectManager]: No ObjectManagerConfig found for map '%s'."), *CurrentMap);
}
