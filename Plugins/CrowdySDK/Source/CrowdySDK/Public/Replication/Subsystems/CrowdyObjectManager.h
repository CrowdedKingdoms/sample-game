#pragma once

#include "CoreMinimal.h"
#include "Core/UDP/Interfaces/ICrowdyReceptionLayer.h"
#include "StructUtils/InstancedStruct.h"
#include "Subsystems/WorldSubsystem.h"
#include "Data/CrowdyObjectRegistry.h"
#include "CrowdyObjectManager.generated.h"

struct FCrowdyObjectDestroyEvent;
struct FCrowdyObjectStateEvent;
struct FCrowdyObjectSpawnEvent;

class UCrowdySDKSubsystem;
class UCrowdyGameSession;
class UCrowdyObjectEventHandler;

UCLASS(BlueprintType, meta=(DisplayName="Crowdy Object Manager"))
class CROWDYSDK_API UCrowdyObjectManager : public UWorldSubsystem, public ICrowdyReceptionLayer
{
    GENERATED_BODY()

public:
   
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    virtual void OnMessageReceived(TSharedRef<ICrowdyMessage> Message) override;
    virtual TArray<ECrowdyMessageType> GetSupportedResponseTypes() const override;
    virtual TArray<FName> GetSupportedEventTypes() const override;

    UFUNCTION(BlueprintCallable, Category="Crowdy SDK|Crowdy Object Manager", meta=(AutoCreateRefTerm="InitialState"))
    AActor* SpawnCrowdyObject(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform, const FInstancedStruct& InitialState);

    UFUNCTION(BlueprintCallable, Category="Crowdy SDK|Crowdy Object Manager", meta=(AutoCreateRefTerm="NewState"))
    void ChangeCrowdyObjectState(AActor* TargetActor, const FInstancedStruct& NewState);

    UFUNCTION(BlueprintCallable, Category="Crowdy SDK|Crowdy Object Manager")
    void DestroyCrowdyObject(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category="Crowdy SDK|Crowdy Object Manager")
    void RegisterStaticCrowdyObject(AActor* Actor, bool bUseDeterministicID, int64 Seed);
    
    UFUNCTION(BlueprintCallable, Category="Crowdy SDK|Crowdy Object Manager")
    AActor* FindActor(const FGuid& ObjectID) const;

    UFUNCTION(BlueprintCallable, Category="Crowdy SDK|Crowdy Object Manager")
    FGuid FindObjectID(const AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category="Crowdy SDK|Crowdy Object Manager")
    bool IsLocallyOwned(const AActor* Actor) const;
    
    bool IsLocallyOwned(const FGuid& ObjectID) const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crowdy SDK|Crowdy Object Manager")
    TArray<FName> SupportedGameEvents;

private:
   
    struct FObjectEntry
    {
        FGuid                  ObjectID;
        FGuid                  OwnerID;
        int32                  TypeID = 0;
        TWeakObjectPtr<AActor> Actor;
    };

    TMap<FGuid, FObjectEntry>  IDToObject;

    UPROPERTY()
    TMap<AActor*, FGuid> ActorToID;

    UPROPERTY()
    TObjectPtr<UCrowdyGameSession> CrowdyGameSession;

    UPROPERTY()
    FGuid CachedLocalPlayerID;

    UPROPERTY()
    TMap<UScriptStruct*, TObjectPtr<UCrowdyObjectEventHandler>> EventHandlers;

    // TypeID → handler for lifecycle routing
    UPROPERTY()
    TMap<int32, TObjectPtr<UCrowdyObjectEventHandler>> TypeIDToHandler;

    UPROPERTY()
    TSet<TObjectPtr<UScriptStruct>> SupportedStructTypes;

    UPROPERTY()
    TObjectPtr<UCrowdyObjectRegistry> ObjectRegistry;

    UPROPERTY()
    TObjectPtr<UCrowdySDKSubsystem> CachedSDK;

private:
    
    void RegisterObjectInternal(const FGuid& ObjectID, const FGuid& OwnerID, int32 TypeID, AActor* Actor);
    void UnregisterObjectInternal(const FGuid& ObjectID);
    void BroadcastToNetwork(const AActor* Actor, FInstancedStruct& Payload);

    void HandleRemoteSpawn(const FCrowdyObjectSpawnEvent& Event);
    void HandleRemoteStateChange(const FCrowdyObjectStateEvent& Event);
    void HandleRemoteDestroy(const FCrowdyObjectDestroyEvent& Event);

    UCrowdyObjectEventHandler* FindHandler(const FInstancedStruct& Payload) const;
    UCrowdyObjectEventHandler* FindHandler(int32 TypeID) const;

    void LoadSupportedStructTypes();
    bool LoadConfig();
};