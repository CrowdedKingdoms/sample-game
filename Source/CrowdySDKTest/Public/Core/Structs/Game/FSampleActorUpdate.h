#pragma once
#include "FSampleActorState.h"

// We are going to store a maximum of 4 snapshots per actor to interpolate between
constexpr int32 SnapshotBufferSize = 10;

// Simple Update Struct
struct FSampleActorUpdate
{
	FSampleActorState State;
	FGuid UUID;
	int64 ServerTimestamp;
};

// A Snapshot of transform data for interpolation
struct FTransformSnapshot
{
	FVector Positions[SnapshotBufferSize];
	FRotator Rotations[SnapshotBufferSize];
	int64 Timestamps[SnapshotBufferSize];
	
	int32 Head = 0;
	int32 Count = 0;
	
	void Add(const FVector& Position, const FRotator& Rotation, const int64 TimestampMs)
	{
		Positions[Head] = Position;
		Rotations[Head] = Rotation;
		Timestamps[Head] = TimestampMs;
		Head = (Head + 1) % SnapshotBufferSize;
		Count = FMath::Min(Count+1, SnapshotBufferSize);
	}
	
	int32 IndexOf(const int32 I) const
	{
		const int32 Oldest = (Count == SnapshotBufferSize) ? Head : 0;
		return (Oldest + I) % SnapshotBufferSize;
	}
};

// Slot Data Structure for SIMD/SoA approach
struct FActorSlot
{
	FGuid UUID;
	TWeakObjectPtr<AActor> Actor;
	bool bActive = false;
};