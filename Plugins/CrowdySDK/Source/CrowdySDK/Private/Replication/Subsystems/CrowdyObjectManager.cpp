#include "Replication/Subsystems/CrowdyObjectManager.h"
#include "Core/UDP/Interfaces/ICrowdyMessage.h"
#include "Data/CrowdyObjectRegistry.h"
#include "Messages/GameObjects/FCrowdyObjectSpawnEvent.h"
#include "Messages/GameObjects/FGameEventNotification.h"
#include "Replication/ObjectHandler/CrowdyObjectEventHandler.h"
#include "Subsystem/CrowdyGameSession.h"
#include "Subsystem/CrowdySDKSubsystem.h"
#include "Utils/CrowdySDKDeveloperSettings.h"
#include "Utils/HelperFunctions.h"

void UCrowdyObjectManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("[CrowdyObjectManager]: Initialize — World=%s"), *GetNameSafe(GetWorld()));
    
    const UWorld* World = GetWorld();
    checkf(IsValid(World), TEXT("World is invalid"));
   
    UCrowdySDKSubsystem* SDK = World->GetGameInstance()->GetSubsystem<UCrowdySDKSubsystem>();
    UCrowdyGameSession* GameSession = World->GetGameInstance()->GetSubsystem<UCrowdyGameSession>();

    checkf(IsValid(SDK), TEXT("CrowdySDKSubsystem is invalid"));
    checkf(IsValid(GameSession), TEXT("GameSession is invalid"));

    CachedSDK = SDK;
    CachedLocalPlayerID = GameSession->GetID();
    
    SupportedGameEvents.Add(FName("CreateCrowdyGameObject"));
    SupportedGameEvents.Add(FName("DestroyCrowdyGameObject"));
    SupportedGameEvents.Add(FName("ChangeCrowdyGameObjectState"));

    LoadSupportedStructTypes();
    
    if (!LoadConfig()) return;
    
    SDK->RegisterReceptionLayer(this);
}

void UCrowdyObjectManager::Deinitialize()
{
    IDToObject.Empty();
    ActorToID.Empty();
    EventHandlers.Empty();
    TypeIDToHandler.Empty();
    SupportedStructTypes.Empty();
    ObjectRegistry = nullptr;
    CachedSDK = nullptr;
    Super::Deinitialize();
}

bool UCrowdyObjectManager::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer))
        return false;

    const UWorld* World = Outer ? Outer->GetWorld() : nullptr;
    if (!World) return false;

    return World->WorldType == EWorldType::PIE
        || World->WorldType == EWorldType::Game;
}

// ── Public API ───────────────────────────────────────────────────────────────

AActor* UCrowdyObjectManager::SpawnCrowdyObject(
    const TSubclassOf<AActor> ActorClass,
    const FTransform& SpawnTransform,
    const FInstancedStruct& InitialState)
{
    if (!ObjectRegistry)
    {
        UE_LOG(LogTemp, Error, TEXT("[CrowdyObjectManager]: ObjectRegistry is null."));
        return nullptr;
    }

    const FCrowdyObjectTypeEntry* Entry = ObjectRegistry->FindByClass(ActorClass);
    if (!Entry)
    {
        UE_LOG(LogTemp, Error, TEXT("[CrowdyObjectManager]: Class '%s' not found in registry."), *ActorClass->GetName());
        return nullptr;
    }

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AActor* Actor = GetWorld()->SpawnActor<AActor>(ActorClass, SpawnTransform, Params);
    if (!IsValid(Actor)) return nullptr;

    const FGuid ObjectID = FGuid::NewGuid();

    RegisterObjectInternal(ObjectID, CachedLocalPlayerID, Entry->TypeID, Actor);

    if (UCrowdyObjectEventHandler* Handler = FindHandler(Entry->TypeID))
        Handler->OnObjectSpawned(Actor, InitialState, true);

    FCrowdyObjectSpawnEvent SpawnEvent;
    SpawnEvent.ObjectID     = ObjectID;
    SpawnEvent.OwnerID      = CachedLocalPlayerID;
    SpawnEvent.TypeID       = Entry->TypeID;
    SpawnEvent.SpawnTransform = SpawnTransform;
    SpawnEvent.InitialState = InitialState;

    FInstancedStruct Payload;
    Payload.InitializeAs<FCrowdyObjectSpawnEvent>(SpawnEvent);
    BroadcastToNetwork(Actor, Payload);

    return Actor;
}

void UCrowdyObjectManager::ChangeCrowdyObjectState(AActor* TargetActor, const FInstancedStruct& NewState)
{
    const FGuid ObjectID = FindObjectID(TargetActor);
    if (!ObjectID.IsValid()) return;

    const FObjectEntry* Entry = IDToObject.Find(ObjectID);
    if (!Entry) return;

    if (UCrowdyObjectEventHandler* Handler = FindHandler(Entry->TypeID))
        Handler->OnObjectStateChanged(TargetActor, NewState);

    FCrowdyObjectStateEvent StateEvent;
    StateEvent.ObjectID = ObjectID;
    StateEvent.NewState = NewState;

    FInstancedStruct Payload;
    Payload.InitializeAs<FCrowdyObjectStateEvent>(StateEvent);
    BroadcastToNetwork(TargetActor, Payload);
}

void UCrowdyObjectManager::DestroyCrowdyObject(AActor* TargetActor)
{
    const FGuid ObjectID = FindObjectID(TargetActor);
    
    if (!ObjectID.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("[CrowdyObjectManager]: Object not found in registry."));
        return;
    }
    
    const FObjectEntry* Entry = IDToObject.Find(ObjectID);
    if (!Entry)
    {
        UE_LOG(LogTemp, Error, TEXT("[CrowdyObjectManager]: Object not found in registry."));
        return;
    };

    const int32 TypeID = Entry->TypeID;

    FCrowdyObjectDestroyEvent DestroyEvent;
    DestroyEvent.ObjectID = ObjectID;

    FInstancedStruct Payload;
    Payload.InitializeAs<FCrowdyObjectDestroyEvent>(DestroyEvent);
    BroadcastToNetwork(TargetActor, Payload);

    float Delay = 0.f;
    if (UCrowdyObjectEventHandler* Handler = FindHandler(TypeID))
        Delay = Handler->OnObjectDestroyed(TargetActor, true);

    UnregisterObjectInternal(ObjectID);

    if (Delay > 0.f)
    {
        FTimerHandle Handle;
        GetWorld()->GetTimerManager().SetTimer(Handle, [TargetActor]()
        {
            if (IsValid(TargetActor)) TargetActor->Destroy();
        }, Delay, false);
    }
    else if (IsValid(TargetActor))
    {
        TargetActor->Destroy();
    }
}

void UCrowdyObjectManager::RegisterStaticCrowdyObject(AActor* Actor, const bool bUseDeterministicID, const int64 Seed)
{
    if (!ObjectRegistry)
    {
        return;
    }
    
    if (!IsValid(Actor))
        return;
    
    const FCrowdyObjectTypeEntry* Entry = ObjectRegistry->FindByClass(Actor->GetClass());
    
    if (!Entry)
    {
        UE_LOG(LogTemp, Error, TEXT("[CrowdyObjectManager]: Class '%s' not found in registry."), *Actor->GetClass()->GetName());
    }
    
    const FGuid ObjectID = bUseDeterministicID ? UHelperFunctions::GetDeterministicID(Seed) : FGuid::NewGuid();
    
    RegisterObjectInternal(ObjectID, CachedLocalPlayerID, Entry->TypeID, Actor);
    
    if (UCrowdyObjectEventHandler* Handler = FindHandler(Entry->TypeID))
        Handler->OnObjectSpawned(Actor, FInstancedStruct(), true);
}

// Lookups

AActor* UCrowdyObjectManager::FindActor(const FGuid& ObjectID) const
{
    const FObjectEntry* Entry = IDToObject.Find(ObjectID);
    return Entry && Entry->Actor.IsValid() ? Entry->Actor.Get() : nullptr;
}

FGuid UCrowdyObjectManager::FindObjectID(const AActor* Actor) const
{
    const FGuid* ID = ActorToID.Find(Actor);
    return ID ? *ID : FGuid{};
}

bool UCrowdyObjectManager::IsLocallyOwned(const FGuid& ObjectID) const
{
    const FObjectEntry* Entry = IDToObject.Find(ObjectID);
    return Entry && Entry->OwnerID == CachedLocalPlayerID;
}

bool UCrowdyObjectManager::IsLocallyOwned(const AActor* Actor) const
{
    return IsLocallyOwned(FindObjectID(Actor));
}

// Network Reception 

void UCrowdyObjectManager::OnMessageReceived(TSharedRef<ICrowdyMessage> Message)
{
    if (Message->GetType() != ECrowdyMessageType::CLIENT_EVENT_NOTIFICATION)
        return;

     FGameEventNotification& GameEvent =
        static_cast<FGameEventNotification&>(*Message);

    AsyncTask(ENamedThreads::GameThread, [this, GameEvent = MoveTemp(GameEvent)]()
    {
        const UScriptStruct* StructType = GameEvent.State.GetScriptStruct();
        if (!StructType) return;

        // Lifecycle events
        if (StructType == FCrowdyObjectSpawnEvent::StaticStruct())
        {
            const FCrowdyObjectSpawnEvent* Event = GameEvent.State.GetPtr<FCrowdyObjectSpawnEvent>();
            if (Event) HandleRemoteSpawn(*Event);
            return;
        }
        if (StructType == FCrowdyObjectStateEvent::StaticStruct())
        {
            const FCrowdyObjectStateEvent* Event = GameEvent.State.GetPtr<FCrowdyObjectStateEvent>();
            if (Event) HandleRemoteStateChange(*Event);
            return;
        }
        if (StructType == FCrowdyObjectDestroyEvent::StaticStruct())
        {
            const FCrowdyObjectDestroyEvent* Event = GameEvent.State.GetPtr<FCrowdyObjectDestroyEvent>();
            if (Event) HandleRemoteDestroy(*Event);
            return;
        }

        // Custom events — whitelist check
        if (!SupportedStructTypes.Contains(StructType)) return;

        if (UCrowdyObjectEventHandler* Handler = FindHandler(GameEvent.State))
            Handler->OnObjectEvent(USerializationFunctionLibrary::ToGuid(GameEvent.UUID), GameEvent.State);
    });
}

TArray<ECrowdyMessageType> UCrowdyObjectManager::GetSupportedResponseTypes() const
{
    return { ECrowdyMessageType::CLIENT_EVENT_NOTIFICATION };
}

TArray<FName> UCrowdyObjectManager::GetSupportedEventTypes() const
{
    return SupportedGameEvents;
}

// ── Remote Handlers ──────────────────────────────────────────────────────────

void UCrowdyObjectManager::HandleRemoteSpawn(const FCrowdyObjectSpawnEvent& Event)
{
    
    if (IDToObject.Contains(Event.ObjectID))
    {
        UE_LOG(LogTemp, Error, TEXT("[CrowdyObjectManager]: Object already spawned on remote spawn."));
        return;
    }
    
    if (!ObjectRegistry.Get())
    {
        UE_LOG(LogTemp, Error, TEXT("[CrowdyObjectManager]: ObjectRegistry is null on remote spawn."));
        return;
    }

    const FCrowdyObjectTypeEntry* Entry = ObjectRegistry->FindByTypeID(Event.TypeID);
    if (!Entry)
    {
        UE_LOG(LogTemp, Error, TEXT("[CrowdyObjectManager]: Unknown TypeID %d in remote spawn."), Event.TypeID);
        return;
    }

    TSubclassOf<AActor> ActorClass = Entry->ActorClass.LoadSynchronous();
    if (!ActorClass) return;

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AActor* Actor = GetWorld()->SpawnActor<AActor>(ActorClass, Event.SpawnTransform, Params);
    if (!IsValid(Actor)) return;

    RegisterObjectInternal(Event.ObjectID, Event.OwnerID, Event.TypeID, Actor);

    if (UCrowdyObjectEventHandler* Handler = FindHandler(Event.TypeID))
        Handler->OnObjectSpawned(Actor, Event.InitialState, false);
}

void UCrowdyObjectManager::HandleRemoteStateChange(const FCrowdyObjectStateEvent& Event)
{
    const FObjectEntry* Entry = IDToObject.Find(Event.ObjectID);
    if (!Entry || !Entry->Actor.IsValid()) return;

    if (UCrowdyObjectEventHandler* Handler = FindHandler(Entry->TypeID))
        Handler->OnObjectStateChanged(Entry->Actor.Get(), Event.NewState);
}

void UCrowdyObjectManager::HandleRemoteDestroy(const FCrowdyObjectDestroyEvent& Event)
{
    const FObjectEntry* Entry = IDToObject.Find(Event.ObjectID);
    if (!Entry)
    {
        UE_LOG(LogTemp, Error, TEXT("[CrowdyObjectManager]: Object already destroyed on remote destroy."));
        return;
    }
    
    AActor* Actor    = Entry->Actor.Get();
    const int32 TypeID = Entry->TypeID;

    float Delay = 0.f;
    if (UCrowdyObjectEventHandler* Handler = FindHandler(TypeID))
        Delay = Handler->OnObjectDestroyed(Actor, false);

    UnregisterObjectInternal(Event.ObjectID);

    if (Delay > 0.f)
    {
        FTimerHandle Handle;
        GetWorld()->GetTimerManager().SetTimer(Handle, [Actor]()
        {
            if (IsValid(Actor)) Actor->Destroy();
        }, Delay, false);
    }
    else if (IsValid(Actor))
    {
        Actor->Destroy();
    }
}

//Internal

void UCrowdyObjectManager::RegisterObjectInternal(
    const FGuid& ObjectID, const FGuid& OwnerID, const int32 TypeID, AActor* Actor)
{
    UE_LOG(LogTemp, Log, TEXT("[CrowdyObjectManager]: Registering Actor=%s, ID=%s"),
       *GetNameSafe(Actor), *ObjectID.ToString());
    
    FObjectEntry Entry;
    Entry.ObjectID = ObjectID;
    Entry.OwnerID  = OwnerID;
    Entry.TypeID   = TypeID;
    Entry.Actor    = Actor;

    IDToObject.Add(ObjectID, Entry);
    ActorToID.Add(Actor, ObjectID);
}

void UCrowdyObjectManager::UnregisterObjectInternal(const FGuid& ObjectID)
{
    const FObjectEntry* Entry = IDToObject.Find(ObjectID);
    if (!Entry) return;

    if (Entry->Actor.IsValid())
        ActorToID.Remove(Entry->Actor.Get());

    IDToObject.Remove(ObjectID);
}

void UCrowdyObjectManager::BroadcastToNetwork(const AActor* Actor, FInstancedStruct& Payload)
{
    if (!IsValid(CachedSDK.Get()) || !IsValid(Actor)) return;

    int64 ChunkX, ChunkY, ChunkZ;
    UHelperFunctions::GetChunkCoordinatesAtWorldLocation(Actor->GetActorLocation(), ChunkX, ChunkY, ChunkZ);

    CachedSDK->DispatchGameEvent(
        ChunkX, ChunkY, ChunkZ,
        ECrowdyDecayRate::No_Decay,
        ECrowdyReplicationDistance::Eight_Chunks,
        CachedLocalPlayerID,
        Payload,
        true);
}

UCrowdyObjectEventHandler* UCrowdyObjectManager::FindHandler(const FInstancedStruct& Payload) const
{
    const UScriptStruct* StructType = Payload.GetScriptStruct();
    if (!StructType) return nullptr;

    const TObjectPtr<UCrowdyObjectEventHandler>* Handler = EventHandlers.Find(StructType);
    return Handler ? Handler->Get() : nullptr;
}

UCrowdyObjectEventHandler* UCrowdyObjectManager::FindHandler(int32 TypeID) const
{
    const TObjectPtr<UCrowdyObjectEventHandler>* Handler = TypeIDToHandler.Find(TypeID);
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
        if (!SupportedGameEvents.Contains(Entry.EventName)) continue;
        if (!IsValid(Entry.EventType)) continue;
        SupportedStructTypes.Add(Entry.EventType);
    }
}

bool UCrowdyObjectManager::LoadConfig()
{
    const UCrowdySDKDeveloperSettings* DevSettings = GetDefault<UCrowdySDKDeveloperSettings>();
    if (!IsValid(DevSettings))
    {
        UE_LOG(LogTemp, Error, TEXT("[CrowdyObjectManager]: DeveloperSettings is null."));
        return false;
    }

    // ─────────────────────────────────────────────────────────────
    // Resolve registry for current map
    // ─────────────────────────────────────────────────────────────

    const UWorld* World = GetWorld();
    if (!IsValid(World))
    {
        UE_LOG(LogTemp, Error, TEXT("[CrowdyObjectManager]: World is null in LoadConfig."));
        return false;
    }

    // Strip the UEDPIE_X_ prefix so PIE worlds match their editor asset path
    const FString WorldPath = UWorld::RemovePIEPrefix(World->GetPathName());

    const TSoftObjectPtr<UCrowdyObjectRegistry>* RegistryPtr = nullptr;
    for (const auto& Pair : DevSettings->ObjectRegistryDataAssetMap)
    {
        if (Pair.Key.ToSoftObjectPath().ToString() == WorldPath)
        {
            RegistryPtr = &Pair.Value;
            break;
        }
    }

    if (!RegistryPtr || RegistryPtr->IsNull())
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[CrowdyObjectManager]: No ObjectRegistry mapped for world '%s', skipping."),
            *WorldPath);
        return false;
    }

    ObjectRegistry = RegistryPtr->LoadSynchronous();
    if (!IsValid(ObjectRegistry.Get()))
    {
        UE_LOG(LogTemp, Error,
            TEXT("[CrowdyObjectManager]: ObjectRegistry failed to load for world '%s'. Asset path: %s"),
            *WorldPath,
            *RegistryPtr->ToString());
        return false;
    }

    // ─────────────────────────────────────────────────────────────
    // Build TypeID -> Handler map
    // ─────────────────────────────────────────────────────────────

    TypeIDToHandler.Empty();

    for (const FCrowdyObjectTypeEntry& Entry : ObjectRegistry->ObjectTypes)
    {
        if (Entry.TypeID < 0)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("[CrowdyObjectManager]: Invalid TypeID on '%s'."),
                *Entry.DebugTypeName.ToString());
            continue;
        }

        TSubclassOf<UCrowdyObjectEventHandler> HandlerClass =
            Entry.HandlerClass.LoadSynchronous();

        if (!HandlerClass)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("[CrowdyObjectManager]: Missing HandlerClass for '%s'."),
                *Entry.DebugTypeName.ToString());
            continue;
        }

        if (TypeIDToHandler.Contains(Entry.TypeID))
        {
            UE_LOG(LogTemp, Warning,
                TEXT("[CrowdyObjectManager]: Duplicate TypeID '%d' on '%s', skipping."),
                Entry.TypeID,
                *Entry.DebugTypeName.ToString());
            continue;
        }

        UCrowdyObjectEventHandler* Handler =
            NewObject<UCrowdyObjectEventHandler>(this, HandlerClass);

        if (!IsValid(Handler))
        {
            UE_LOG(LogTemp, Warning,
                TEXT("[CrowdyObjectManager]: Failed to create handler for TypeID '%d'."),
                Entry.TypeID);
            continue;
        }

        TypeIDToHandler.Add(Entry.TypeID, Handler);
    }

    // ─────────────────────────────────────────────────────────────
    // Build StructType -> Handler map for custom payload events
    // ─────────────────────────────────────────────────────────────

    const UEventPayloadType* Registry =
        DevSettings->EventPayloadDataAsset.LoadSynchronous();

    if (!IsValid(Registry))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[CrowdyObjectManager]: EventPayloadDataAsset failed to load. Asset path: %s"),
            *DevSettings->EventPayloadDataAsset.ToString());
        return false;
    }

    EventHandlers.Empty();

    for (const FEventPayloadTypeEntry& Entry : Registry->Entries)
    {
        if (!SupportedGameEvents.Contains(Entry.EventName)) continue;
        if (!IsValid(Entry.EventType)) continue;

        if (EventHandlers.Contains(Entry.EventType))
        {
            UE_LOG(LogTemp, Warning,
                TEXT("[CrowdyObjectManager]: Duplicate EventType '%s', skipping."),
                *Entry.EventType->GetName());
            continue;
        }

        const FCrowdyObjectTypeEntry* ObjectEntry = ObjectRegistry->FindByEventType(Entry.EventType);
        if (!ObjectEntry)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("[CrowdyObjectManager]: No ObjectRegistry entry for payload type '%s'."),
                *Entry.EventType->GetName());
            continue;
        }

        UCrowdyObjectEventHandler* Handler = FindHandler(ObjectEntry->TypeID);
        if (!Handler)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("[CrowdyObjectManager]: No handler found for TypeID '%d' (payload '%s')."),
                ObjectEntry->TypeID,
                *Entry.EventType->GetName());
            continue;
        }

        EventHandlers.Add(Entry.EventType, Handler);
    }

    UE_LOG(LogTemp, Log,
        TEXT("[CrowdyObjectManager]: LoadConfig complete. World='%s', TypeIDToHandler=%d, ObjectRegistry=%s"),
        *WorldPath,
        TypeIDToHandler.Num(),
        *GetNameSafe(ObjectRegistry));

    return true;
}