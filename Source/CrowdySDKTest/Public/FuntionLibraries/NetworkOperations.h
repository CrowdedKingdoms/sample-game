// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Enums/ESampleObjectOperationType.h"
#include "Core/Structs/Game/FSampleObjectOperationEvent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NetworkOperations.generated.h"

enum class ESampleAnimState : uint8;
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
	static void EnqueueActorUpdate(const UCrowdySDKSubsystem* CrowdySDK, 
		int64 ChunkX, int64 ChunkY, int64 ChunkZ, 
		const FString& UUID, 
		const FSampleActorState& ActorState);
	
	UFUNCTION(BlueprintCallable, Category="CrowdySDK Sample|Network Operations")
	static void RequestAnimationStateChange(const UCrowdySDKSubsystem* CrowdySDK, 
		int64 ChunkX, int64 ChunkY, int64 ChunkZ, 
		const FString& UUID, 
		const ESampleAnimState NewAnimState);
	
	UFUNCTION(BlueprintCallable, Category="CrowdySDK Sample|Network Operations")
	static void DispatchObjectOperation(const UCrowdySDKSubsystem* CrowdySDK,
		const int64 ChunkX, const int64 ChunkY, const int64 ChunkZ,
		const FString& InstigatorUUID,
		const FSampleObjectOperationEvent ObjectOperationEvent);
};
