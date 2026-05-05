// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/UDP/Interfaces/ICrowdyReceptionLayer.h"
#include "GameFramework/Actor.h"
#include "SampleObjectManager.generated.h"

struct FSampleSetObjectRotation;
struct FSampleSetObjectLocation;
struct FSampleDestroyObject;
struct FSampleSpawnObject;
struct FGameEventNotification;

UCLASS()
class CROWDYSDKTEST_API ASampleObjectManager : public AActor, public ICrowdyReceptionLayer
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASampleObjectManager();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	virtual void OnMessageReceived(TSharedRef<ICrowdyMessage> Message) override;
	virtual TArray<ECrowdyMessageType> GetSupportedResponseTypes() const override;
	virtual TArray<FName> GetSupportedEventTypes() const override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	
	UPROPERTY(EditAnywhere)
	TMap<FName, TSubclassOf<AActor>> ActorsToSpawn;
	
	UPROPERTY()
	TMap<FGuid, AActor*> ObjectToActorMap;
	
	void HandleGameEvent(const FGameEventNotification& Event);
	void HandleSpawnObject(const FSampleSpawnObject& SpawnObject);
	void HandleDestroyObject(const FSampleDestroyObject& DestroyObject);
	void HandleSetObjectLocation(const FSampleSetObjectLocation& SetObjectLocation);
	void HandleSetObjectRotation(const FSampleSetObjectRotation& SetObjectRotation);
	
};
