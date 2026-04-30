// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NetworkOperations.generated.h"

struct FSampleActorState;
class UCrowdySDKSubsystem;
/**
 * 
 */
UCLASS(BlueprintType)
class CROWDYSDKTEST_API UNetworkOperations : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, Category="CrowdySDK Sample|Network Operations")
	static void EnqueueActorUpdate(const UCrowdySDKSubsystem* CrowdySDK, int64 ChunkX, int64 ChunkY, int64 ChunkZ, const FString& UUID, const FSampleActorState& ActorState);
	
	// TODO: Implement Various Game Events for showcase
	//static void EnqueueGameEvent();
	
};
