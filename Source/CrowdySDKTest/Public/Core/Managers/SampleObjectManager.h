// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/UDP/Interfaces/ICrowdyReceptionLayer.h"
#include "GameFramework/Actor.h"
#include "SampleObjectManager.generated.h"

UCLASS()
class CROWDYSDKTEST_API ASampleObjectManager : public AActor, public ICrowdyReceptionLayer
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASampleObjectManager();
	
	virtual void OnMessageReceived(TSharedRef<ICrowdyMessage> Message) override;
	virtual TArray<ECrowdyMessageType> GetSupportedResponseTypes() const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> ActorClassToSpawn;
};
