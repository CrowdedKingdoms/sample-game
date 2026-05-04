// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Managers/SampleObjectManager.h"
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

