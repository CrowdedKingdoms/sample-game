// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Managers/SamplePawnManager.h"
#include "Core/Structs/Game/FSampleActorUpdate.h"
#include "Interfaces/ReplicatedActor.h"


// Sets default values
ASamplePawnManager::ASamplePawnManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ASamplePawnManager::BeginPlay()
{
	Super::BeginPlay();
	
	// Since we're using a pooled approach, we initialize the actor pool at begin play
	InitializePool();
}

// Called every frame
void ASamplePawnManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// We perform these operations per tick
	ApplyPendingUpdates();
	UpdateMovementData();
	ApplyTransformData();
	ProcessRemovals();
}



void ASamplePawnManager::InitializePool()
{
	// Validate the class
	checkf(ReplicatedActorClass, TEXT("Replicated Actor Class is invalid."));

	// Take this actors own location as the spawn point
	const FVector SpawnLocation = GetActorLocation();
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 i = 0; i < PoolSize; i++)
	{
		// Spawn Actors
		AActor* Actor = GetWorld()->SpawnActor<AActor>(
			ReplicatedActorClass,
			SpawnLocation,
			FRotator::ZeroRotator,
			Params
		);

		if (!Actor)
			continue;

		// Deactivate them since nothing is in use yet
		DeactivateActor(Actor);

		// create a slot, fill data and append it to our bookkeeping
		FActorSlot Slot;
		Slot.Actor = Actor;
		Slot.bActive = false;

		Slots.Add(Slot);
	}
}

void ASamplePawnManager::ActivateActor(AActor* Actor, const FVector& Location, const FRotator& Rotation)
{
	if (!IsValid(Actor))
		return;

	Actor->SetActorLocationAndRotation(Location, Rotation);
	Actor->SetActorHiddenInGame(false);
	Actor->SetActorTickEnabled(true);
	Actor->SetActorEnableCollision(true);
}


void ASamplePawnManager::DeactivateActor(AActor* Actor)
{
	if (!IsValid(Actor))
		return;
	
	Actor->SetActorHiddenInGame(true);
	Actor->SetActorTickEnabled(false);
	Actor->SetActorEnableCollision(false);
}

AActor* ASamplePawnManager::GetPawnFromPool()
{
	// Scan through the slots and return desired actor
	for (FActorSlot& Slot : Slots)
	{
		if (!Slot.bActive && Slot.Actor.IsValid())
		{
			Slot.bActive = true;
			return Slot.Actor.Get();
		}
	}

	return nullptr;
}

void ASamplePawnManager::ReturnPawnToPool(AActor* Actor)
{
	
	if (!IsValid(Actor))
		return;

	// Scan through slots and return actor to the pool as it's no longer needed
	for (FActorSlot& Slot : Slots)
	{
		if (Slot.Actor.Get() == Actor)
		{
			Slot.bActive = false;
			DeactivateActor(Actor);
			return;
		}
	}

}

void ASamplePawnManager::AddInstance(const FGuid& UUID, const FSampleActorUpdate& Update)
{
	// If it's already added, we don't want to add it again so return early
	if (UUIDToSlot.Contains(UUID))
		return;

	// Get actor from pool
	AActor* Actor = GetPawnFromPool();
	
	// Validate it
	if (!IsValid(Actor))
		return;

	// Get a new slot ID
	const int32 SlotId = Slots.IndexOfByPredicate(
		[Actor](const FActorSlot& S)
		{
			return S.Actor.Get() == Actor;
		}
	);

	// If it's null, then return
	if (SlotId == INDEX_NONE)
		return;

	// Assign new slot UUID
	Slots[SlotId].UUID = UUID;

	// Activate the actor
	ActivateActor(
		Actor,
		Update.State.Location,
		Update.State.Rotation
	);

	// Add to bookkeeping
	UUIDToSlot.Add(UUID, SlotId);

	// Allocate SoA aligned buffers
	if (TransformSnapshots.Num() <= SlotId)
	{
		TransformSnapshots.SetNum(SlotId + 1);
		LastAppliedPositions.SetNum(SlotId + 1);
		LastAppliedRotations.SetNum(SlotId + 1);
	}

	TransformSnapshots[SlotId].Add(
		Update.State.Location,
		Update.State.Rotation,
		Update.ServerTimestamp
	);

}

void ASamplePawnManager::DestroyInstance(const FGuid& UUID)
{
	// Search for Slot in the map
	const int32* SlotPtr = UUIDToSlot.Find(UUID);
	
	// Return if it's null
	if (!SlotPtr)
		return;

	// deference the pointer
	const int32 SlotId = *SlotPtr;

	// remove from bookkeeping
	UUIDToSlot.Remove(UUID);

	// Scan and return the actor to the pool since it's no longer needed
	if (Slots.IsValidIndex(SlotId))
	{
		AActor* Actor = Slots[SlotId].Actor.Get();

		if (IsValid(Actor))
		{
			ReturnPawnToPool(Actor);
		}

		Slots[SlotId].bActive = false;
	}

	PendingRemovals.Add(SlotId);
}

void ASamplePawnManager::AppendInstanceUpdate(const FSampleActorUpdate& Update)
{
	UpdateQueue.Enqueue(Update);
}

void ASamplePawnManager::ChangeAnimation(const FGuid& UUID, const ESampleAnimState NewAnimationState)
{
	// Same as before, get the point, validate it and move on
	const int32* SlotPtr = UUIDToSlot.Find(UUID);
	if (!SlotPtr)
		return;

	const int32 SlotId = *SlotPtr;

	if (!Slots.IsValidIndex(SlotId))
		return;
	
	AActor* TargetActor = Slots[SlotId].Actor.Get();
	
	if (!IsValid(TargetActor))
		return;

	// Dispatch the call to update the animation state inside actor. Check BP_ReplicatedActor in Content/Replication for implementation
	IReplicatedActor::Execute_ChangeAnimationState(TargetActor, NewAnimationState);
}

void ASamplePawnManager::ApplyPendingUpdates()
{
	// Return early if queue is empty
	if (UpdateQueue.IsEmpty())
		return;
	
	FSampleActorUpdate Update;

	// Drain the queue
	while (UpdateQueue.Dequeue(Update))
	{
		// Update server time offset with latest timestamp
		UpdateServerTimeOffset(Update.ServerTimestamp);

		// Find slot pointer and validate it
		const int32* SlotPtr = UUIDToSlot.Find(Update.UUID);
		if (!SlotPtr)
			continue;

		const int32 SlotId = *SlotPtr;
		
		if (!TransformSnapshots.IsValidIndex(SlotId))
			continue;

		// Add new data to Buffer Snapshots
		TransformSnapshots[SlotId].Add(
			Update.State.Location,
			Update.State.Rotation,
			Update.ServerTimestamp
		);
	}
}

FTransform ASamplePawnManager::InterpolateTransform(const FTransformSnapshot& Buffer, const int64 RenderTime)
{
	if (Buffer.Count == 0)
		return FTransform::Identity;

	if (Buffer.Count == 1)
	{
		const int32 Idx = Buffer.IndexOf(0);

		const FRotator& R = Buffer.Rotations[Idx];
		const FVector& P = Buffer.Positions[Idx];

		return FTransform(FQuat(R), FVector(P));
	}

	// Binary search since timestamps are sorted (ring buffer preserves order on insert)
	int32 Lo = 0, Hi = Buffer.Count - 2;
	while (Lo <= Hi)
	{
		const int32 Mid = (Lo + Hi) / 2;
		const int32 IdxA = Buffer.IndexOf(Mid);
		const int32 IdxB = Buffer.IndexOf(Mid + 1);
		const int64 TimeA = Buffer.Timestamps[IdxA];
		const int64 TimeB = Buffer.Timestamps[IdxB];

		if (RenderTime < TimeA)
		{
			Hi = Mid - 1;
			continue;
		}
		if (RenderTime > TimeB)
		{
			Lo = Mid + 1;
			continue;
		}

		const float Alpha = FMath::Clamp(
			static_cast<float>(RenderTime - TimeA) /
			static_cast<float>(TimeB - TimeA),
			0.f, 1.f
		);

		const auto& RotationA = Buffer.Rotations[IdxA];
		const auto& RotationB = Buffer.Rotations[IdxB];

		const auto& PositionA = Buffer.Positions[IdxA];
		const auto& PositionB = Buffer.Positions[IdxB];

		const FQuat QuatA = RotationA.Quaternion();
		const FQuat QuatB = RotationB.Quaternion();

		const FQuat FinalRotation = FQuat::Slerp(QuatA, QuatB, Alpha).GetNormalized();
		const FVector FinalPosition = FMath::Lerp(PositionA, PositionB, Alpha);

		return FTransform(FinalRotation, FinalPosition);
	}

	// Past newest — extrapolate
	const int32 LastIdx = Buffer.IndexOf(Buffer.Count - 1);
	const int32 PrevIdx = Buffer.IndexOf(Buffer.Count - 2);

	const float Dt = FMath::Max(
		static_cast<float>(Buffer.Timestamps[LastIdx] - Buffer.Timestamps[PrevIdx]) / 1000.f,
		0.001f
	);

	const FVector Velocity =
		(Buffer.Positions[LastIdx] - Buffer.Positions[PrevIdx]) / Dt;

	const float TimePast =
		static_cast<float>(RenderTime - Buffer.Timestamps[LastIdx]) / 1000.f;

	const FVector Extrapolated =
		Buffer.Positions[LastIdx] + Velocity * FMath::Min(TimePast, 0.2f);

	const FQuat QuatA = Buffer.Rotations[PrevIdx].Quaternion();
	const FQuat QuatB = Buffer.Rotations[LastIdx].Quaternion();

	const FQuat Delta = QuatB * QuatA.Inverse();

	const float Step = Dt > 0.f ? TimePast / Dt : 0.f;
	const FQuat ExtrapolatedRot = FQuat::Slerp(QuatB, Delta * QuatB, Step).GetNormalized();

	return FTransform(
		FQuat(ExtrapolatedRot),
		FVector(Extrapolated)
	);

}

void ASamplePawnManager::UpdateMovementData()
{
	const int32 Count = Slots.Num();

	if (Count == 0)
		return;

	TransformBuffer.SetNum(Count);

	const int64 ServerTime = GetEstimatedServerTimeMs();

	ParallelFor(Count, [this, ServerTime](int32 i)
	{
		if (i >= TransformSnapshots.Num())
			return;

		if (!Slots.IsValidIndex(i) || !Slots[i].bActive)
			return;

		const int64 RenderTime = ServerTime - InterpolationDelayMs;

		TransformBuffer[i] =
			InterpolateTransform(
				TransformSnapshots[i],
				RenderTime
			);
	});
}

void ASamplePawnManager::ApplyTransformData()
{
	const int32 Count = Slots.Num();

	for (int32 i = 0; i < Count; i++)
	{
		if (!Slots[i].bActive)
			continue;

		AActor* Actor = Slots[i].Actor.Get();
		
		if (!IsValid(Actor))
			continue;

		if (!TransformBuffer.IsValidIndex(i))
			continue;

		const FTransform& T = TransformBuffer[i];

		Actor->SetActorTransform(T);

		if (LastAppliedPositions.IsValidIndex(i))
			LastAppliedPositions[i] = T.GetLocation();

		if (LastAppliedRotations.IsValidIndex(i))
			LastAppliedRotations[i] = T.GetRotation().Rotator();
	}

}

void ASamplePawnManager::UpdateServerTimeOffset(const int64 ServerTimestampMs)
{
	const int64 ClientNowMs = (FDateTime::UtcNow() - FDateTime(1970, 1, 1)).GetTotalMilliseconds();
	const int64 EstimatedOffset = ServerTimestampMs - ClientNowMs;

	if (!bHasInitialOffset)
	{
		ServerTimeOffsetMs = EstimatedOffset;
		BestOffsetMs = EstimatedOffset;
		bHasInitialOffset = true;
		return;
	}

	if (EstimatedOffset > BestOffsetMs)
		BestOffsetMs = EstimatedOffset;

	if (FMath::Abs(EstimatedOffset - ServerTimeOffsetMs) > 200)
		return;

	ServerTimeOffsetMs = static_cast<int64>(FMath::Lerp(
		static_cast<double>(ServerTimeOffsetMs),
		static_cast<double>(BestOffsetMs),
		0.05
	));

}

int64 ASamplePawnManager::GetEstimatedServerTimeMs() const
{
	const int64 ClientNowMs = (FDateTime::UtcNow() - FDateTime(1970, 1, 1)).GetTotalMilliseconds();
	return ClientNowMs + ServerTimeOffsetMs;

}

void ASamplePawnManager::ProcessRemovals()
{
	PendingRemovals.Empty();
}

