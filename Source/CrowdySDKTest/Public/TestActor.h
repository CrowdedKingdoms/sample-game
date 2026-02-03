// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/UDP/Interfaces/ICrowdyReceptionLayer.h"
#include "Messages/Actor/FActorUpdateNotificationMessage.h"

#include "TestActor.generated.h"

class UCrowdySDKSubsystem;

UCLASS(Blueprintable, BlueprintType)
class CROWDYSDKTEST_API ATestActor : public AActor, public ICrowdyReceptionLayer
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATestActor();
	
	UFUNCTION(BlueprintCallable, Category="Test Actor Updates")
	void SendActorUpdates(const FVector Location) const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	FString MyActorUUID; 
	
	void ReceiveActorUpdates(const FActorUpdateNotificationMessage& ActorUpdateNotify);
	
	virtual void OnMessageReceived(TSharedRef<ICrowdyMessage> Message) override;
	virtual TArray<ECrowdyMessageType> GetSupportedResponseTypes() const override;
	
	UPROPERTY()
	UCrowdySDKSubsystem* SDK; 

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
