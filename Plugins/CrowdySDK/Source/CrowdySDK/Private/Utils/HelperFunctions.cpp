// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/HelperFunctions.h"

#include "Replication/Components/CrowdyObjectComponent.h"
#include "Replication/Subsystems/CrowdyObjectManager.h"
#include "Subsystem/CrowdyGameSession.h"
#include "Subsystem/CrowdySDKSubsystem.h"


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

FGuid UHelperFunctions::GetDeterministicID(const int64 Seed)
{
	const uint32 A = GetTypeHash(Seed);
	const uint32 B = HashCombine(A, 0x9E3779B9);
	const uint32 C = HashCombine(B, 0x85EBCA6B);
	const uint32 D = HashCombine(C, 0xC2B2AE35);
	
	return FGuid(A, B, C, D);
}

FGuid UHelperFunctions::GetNewID()
{
	return FGuid::NewGuid();
}

void UHelperFunctions::DispatchEventForObject(UObject* WorldContextObject, const AActor* Object,
                                              FInstancedStruct EventPayload)
{
	if (!IsValid(WorldContextObject)) return;
	
	if (!IsValid(Object)) return;

	if (!IsValid(EventPayload.GetScriptStruct()))
		return;

	const UCrowdyObjectComponent* ObjectComponent = Cast<UCrowdyObjectComponent>(Object->GetComponentByClass(UCrowdyObjectComponent::StaticClass()));
	
	if (!IsValid(ObjectComponent))
		return;
	
	ObjectComponent->DispatchEventForObject(EventPayload);
}

