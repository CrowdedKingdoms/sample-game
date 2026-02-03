// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/HelperFunctions.h"


void UHelperFunctions::GetChunkCoordinatesAtWorldLocation(const FVector& WorldLocation, int64& ChunkX, int64& ChunkY,
	int64& ChunkZ)
{
	constexpr double ChunkSize = 1600.0f;
	
	ChunkX = static_cast<int64>(FMath::FloorToDouble(WorldLocation.X / ChunkSize));
	ChunkY = static_cast<int64>(FMath::FloorToDouble(WorldLocation.Y / ChunkSize));
	ChunkZ = static_cast<int64>(FMath::FloorToDouble(WorldLocation.Z / ChunkSize));
}

void UHelperFunctions::GetVoxelCoordinatesAtWorldLocation(const FVector& WorldLocation, int32& VoxelX, int32& VoxelY,
	int32& VoxelZ, const int32 ChunkSize)
{
	int32 X = static_cast<int32>(WorldLocation.X);
	int32 Y = static_cast<int32>(WorldLocation.Y);
	int32 Z = static_cast<int32>(WorldLocation.Z);
	
	X = X % ChunkSize;
	Y = Y % ChunkSize;
	Z = Z % ChunkSize;
	
	if (X < 0) X += ChunkSize;
	if (Y < 0) Y += ChunkSize;
	if (Z < 0) Z += ChunkSize;
	
	VoxelX = X / 100;
	VoxelY = Y / 100;
	VoxelZ = Z / 100;
}

FString UHelperFunctions::GetNewUUID()
{
	const FGuid NewGuid = FGuid::NewGuid();
	FString GuidStr = NewGuid.ToString(EGuidFormats::Digits);
	return GuidStr;
}
