#include "Sample/SampleTransformReplicationPolicy.h"

#include "GameFramework/Actor.h"
#include "Sample/SampleAnimReceiver.h"
#include "Sample/SampleTypes.h"

bool USampleTransformReplicationPolicy::ExtractFields(const FInstancedStruct& State, int64 ServerTimestampMs, int32 SlotId)
{
	if (State.GetScriptStruct() != FSampleEntityState::StaticStruct())
		return false;

	const FSampleEntityState& S = State.Get<FSampleEntityState>();
	EnsureSlot(SlotId);
	Positions[SlotId].Push(S.Location, ServerTimestampMs);
	Rotations[SlotId].Push(S.Rotation, ServerTimestampMs);
	Velocities[SlotId] = S.Velocity;
	Falling[SlotId] = S.bIsFalling;
	return true;
}

void USampleTransformReplicationPolicy::ApplyToActor(AActor* Actor, int32 SlotId, int64 RenderTimeMs)
{
	if (!Positions.IsValidIndex(SlotId) || !Rotations.IsValidIndex(SlotId))
		return;

	const FVector Pos = Positions[SlotId].Sample(RenderTimeMs,
		[](const FVector& A, const FVector& B, float T)
		{
			return FMath::Lerp(A, B, T);
		});

	const FRotator Rot = Rotations[SlotId].Sample(RenderTimeMs,
		[](const FRotator& A, const FRotator& B, float T)
		{
			return FQuat::Slerp(A.Quaternion(), B.Quaternion(), FMath::Clamp(T, 0.f, 1.2f))
				.GetNormalized().Rotator();
		});

	Actor->SetActorLocationAndRotation(Pos, Rot);

	// Hand the latched movement inputs to the proxy character. Its AnimBP reads these
	// (via AnimVelocity / bAnimFalling) instead of the non-simulating movement component.
	if (Actor->Implements<USampleAnimReceiver>() && Velocities.IsValidIndex(SlotId))
	{
		ISampleAnimReceiver::Execute_ApplyAnimSnapshot(Actor, Velocities[SlotId], Falling[SlotId]);
	}
}

void USampleTransformReplicationPolicy::OnInstanceDeactivated(int32 SlotId)
{
	if (Positions.IsValidIndex(SlotId)) Positions[SlotId] = {};
	if (Rotations.IsValidIndex(SlotId)) Rotations[SlotId] = {};
	if (Velocities.IsValidIndex(SlotId)) Velocities[SlotId] = FVector::ZeroVector;
	if (Falling.IsValidIndex(SlotId)) Falling[SlotId] = false;
}

void USampleTransformReplicationPolicy::EnsureSlot(int32 SlotId)
{
	if (SlotId >= Positions.Num())
	{
		Positions.SetNum(SlotId + 1);
		Rotations.SetNum(SlotId + 1);
		Velocities.SetNum(SlotId + 1);
		Falling.SetNum(SlotId + 1);
	}
}
