// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HelperFunctions.generated.h"

/**
 * 
 */
UCLASS()
class CROWDYSDK_API UHelperFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CrowdySDK|Coordinates")
	static void GetChunkCoordinatesAtWorldLocation(const FVector& WorldLocation, int64& ChunkX, int64& ChunkY, int64& ChunkZ);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CrowdySDK|Coordinates")
	static void GetVoxelCoordinatesAtWorldLocation(const FVector& WorldLocation, int32& VoxelX, int32& VoxelY, int32& VoxelZ, const int32 ChunkSize = 1600);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CrowdySDK|Identifiers")
	static FString GetNewUUID();
};
