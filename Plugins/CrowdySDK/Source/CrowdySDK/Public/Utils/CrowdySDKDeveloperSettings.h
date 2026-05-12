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
	
	UPROPERTY(Config, EditAnywhere, Category="Crowdy SDK|Developer|Generic")
	int64 AppID = 1;
	
	UPROPERTY(Config, EditAnywhere, Category="Crowdy SDK|Developer|Events")
	TSoftObjectPtr<UEventPayloadType> EventPayloadDataAsset;
	
	UPROPERTY(Config, EditAnywhere, Category="Crowdy SDK|Developer|Actor Updates")
	TSoftObjectPtr<UActorUpdatePayloadType> ActorUpdatePayloadDataAsset;
	
	UPROPERTY(Config, EditAnywhere, Category="Crowdy SDK|Developer|Actor Updates")
	bool bUseAutoReplicator = true;
	
	UPROPERTY(Config, EditAnywhere, Category="Crowdy SDK|Developer|Actor Updates", meta=(EditCondition="bUseAutoReplicator", ClampMin=10, ClampMax=20, DisplayName="Replication Interval (Hertz)"));
	int32 ReplicationIntervalHz = 10;
	
	UPROPERTY(Config, EditAnywhere, Category="Crowdy SDK|Developer|Actor Updates", meta=(EditCondition="bUseAutoReplicator"))
	TSet<TSoftObjectPtr<UWorld>> LevelsToUseAutoReplicator;
	
	UPROPERTY(Config, EditAnywhere, Category="Crowdy SDK|Developer|Actor Tracker")
	bool bUseCrowdyActorTracker = true;
	
	UPROPERTY(Config, EditAnywhere, Category="Crowdy SDK|Developer|Actor Tracker", meta=(EditCondition="bUseCrowdyActorTracker"))
	bool bDispatchUpdatesOnGameThread = false;
	
	UPROPERTY(Config, EditAnywhere, Category="Crowdy SDK|Developer|Actor Tracker", meta=(EditCondition="bUseCrowdyActorTracker"))
	bool bEnableOwnerTracking = true;
	
	UPROPERTY(Config, EditAnywhere, Category="Crowdy SDK|Developer|Actor Tracker", meta=(EditCondition="bUseCrowdyActorTracker"))
	float ActorTimeoutThreshold = 5.0f;
	
	UPROPERTY(Config, EditAnywhere, Category="Crowdy SDK|Developer|Actor Tracker", meta=(EditCondition="bUseCrowdyActorTracker", ClampMin=1))
	int32 MaxTrackedActors = 1000;
	
	UPROPERTY(Config, EditAnywhere, Category="Crowdy SDK|Developer|Actor Tracker", meta=(EditCondition="bUseCrowdyActorTracker", ClampMin=1))
	int32 MaxUpdatesPerBatch = 100;
	
	UPROPERTY(Config, EditAnywhere, Category="Crowdy SDK|Developer|Actor Tracker", meta=(EditCondition="bUseCrowdyActorTracker", ClampMin=0.001f, ClampMax=0.1f))
	float MaxBatchWaitTime = 0.005f;
	
	UPROPERTY(Config, EditAnywhere, Category="Crowdy SDK|Developer|Actor Tracker", meta=(EditCondition="bUseCrowdyActorTracker"))
	TSet<TSoftObjectPtr<UWorld>> LevelsToUseActorTracker;
	
};
