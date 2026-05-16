// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/ActorUpdatePayloadType.h"
#include "Data/CrowdyActorManagementConfig.h"
#include "Data/CrowdyRepApplicationPolicy.h"
#include "Data/EventPayloadType.h"
#include "Data/FCrowdyObjectManagerConfig.h"
#include "Engine/DeveloperSettings.h"
#include "Replication/Subsystems/CrowdyActorPoolSubsystem.h"
#include "CrowdySDKDeveloperSettings.generated.h"

class UCrowdyRepApplicationPolicy;
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
	
	UPROPERTY(Config, EditAnywhere, Category="Crowdy SDK|Developer|Actor Management")
	TMap<TSoftObjectPtr<UWorld>, TSoftObjectPtr<UCrowdyActorManagementConfig>> ActorManagementConfigs;
	
	UPROPERTY(EditAnywhere, Category="Crowdy SDK|Developer|Object Management")
	TMap<TSoftObjectPtr<UWorld>, FCrowdyObjectManagerConfig> ObjectManagerConfigs;
	
#if WITH_EDITOR
	void ValidateObjectHandlerBindings();
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
};
