// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/CrowdyRepApplicationPolicy.h"
#include "Data/TInterpolatedField.h"
#include "SampleTransformReplicationPolicy.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class CROWDYSDKTEST_API USampleTransformReplicationPolicy : public UCrowdyRepApplicationPolicy
{
	GENERATED_BODY()
	
public:
	
	virtual bool ExtractFields(const FInstancedStruct& State, int64 ServerTimestampMs, int32 SlotId) override;
	virtual void ApplyToActor(AActor* Actor, int32 SlotId, int64 RenderTimeMs) override;
	virtual void OnInstanceDeactivated(int32 SlotId) override;
	virtual int32 GetExpectedSlotCount() const override { return 128; }

private:
	
	// SoA buffers — indexed by SlotId
	// TInterpolatedField owns the ring buffer per slot
	TArray<TInterpolatedField<FVector>>   Positions;
	TArray<TInterpolatedField<FRotator>>  Rotations;

	void EnsureSlot(int32 SlotId);
};
