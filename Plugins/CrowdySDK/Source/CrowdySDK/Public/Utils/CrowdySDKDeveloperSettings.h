// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/ActorUpdatePayloadType.h"
#include "Data/EventPayloadType.h"
#include "Engine/DeveloperSettings.h"
#include "CrowdySDKDeveloperSettings.generated.h"

/**
 * 
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Crowdy SDK"))
class CROWDYSDK_API UCrowdySDKDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	
	virtual FName GetCategoryName() const override { return "Plugins"; }
	virtual FName GetSectionName() const override { return "Crowdy SDK"; }
	
	UPROPERTY(Config, EditAnywhere, Category="CrowdySDK|Developer|Generic")
	int64 AppID = 1;
	
	UPROPERTY(Config, EditAnywhere, Category="CrowdySDK|Developer|Events")
	TSoftObjectPtr<UEventPayloadType> EventPayloadDataAsset;
	
	UPROPERTY(Config, EditAnywhere, Category="CrowdySDK|Developer|Actor Updates")
	TSoftObjectPtr<UActorUpdatePayloadType> ActorUpdatePayloadDataAsset;
};
