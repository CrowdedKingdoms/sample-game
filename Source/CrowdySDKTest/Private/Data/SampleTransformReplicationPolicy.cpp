// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/SampleTransformReplicationPolicy.h"

#include "Core/Structs/Game/FSampleActorState.h"

bool USampleTransformReplicationPolicy::ExtractFields(const FInstancedStruct& State, int64 ServerTimestampMs,
                                                      int32 SlotId)
{
	// Type check comes from developer settings; we know what struct to expect.
	// Example: const UScriptStruct* Expected = UMyDevSettings::Get()->ReplicatedStateStruct;
	if (State.GetScriptStruct() != FSampleActorState::StaticStruct())
		return false;

	const FSampleActorState& S = State.Get<FSampleActorState>();
	EnsureSlot(SlotId);
	Positions[SlotId].Push(S.Location, ServerTimestampMs);
	Rotations[SlotId].Push(S.Rotation, ServerTimestampMs);
	return true;
}

void USampleTransformReplicationPolicy::ApplyToActor(AActor* Actor, int32 SlotId, int64 RenderTimeMs)
{
	if (!Positions.IsValidIndex(SlotId)) return;

	const FVector Pos = Positions[SlotId].Sample(RenderTimeMs,
		[](const FVector& A, const FVector& B, float T) {
			return FMath::Lerp(A, B, T);
		});

	const FRotator Rot = Rotations[SlotId].Sample(RenderTimeMs,
		[](const FRotator& A, const FRotator& B, float T) {
			// Slerp via quaternion, handles >1 alpha for extrapolation gracefully
			return FQuat::Slerp(A.Quaternion(), B.Quaternion(), FMath::Clamp(T, 0.f, 1.2f))
					   .GetNormalized().Rotator();
		});

	Actor->SetActorLocationAndRotation(Pos, Rot);
}

void USampleTransformReplicationPolicy::OnInstanceDeactivated(int32 SlotId)
{
	if (Positions.IsValidIndex(SlotId)) Positions[SlotId] = {};
	if (Rotations.IsValidIndex(SlotId)) Rotations[SlotId] = {};
}

void USampleTransformReplicationPolicy::EnsureSlot(int32 SlotId)
{
	if (SlotId >= Positions.Num())
	{
		Positions.SetNum(SlotId + 1);
		Rotations.SetNum(SlotId + 1);
	}
}
