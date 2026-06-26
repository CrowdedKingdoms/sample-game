#pragma once

#include "CoreMinimal.h"
#include "Core/Enums/ESampleAnimState.h"
#include "SampleTypes.generated.h"

// Shared data the sample examples pass around.
//
// SDK v2 lets a CrowdyEvent carry any supported type directly (primitives, strings,
// enums, structs, object and class references, and arrays of those), so most of the
// RPC examples take their parameters straight rather than wrapping them in a payload
// struct. The structs below exist only where the SDK genuinely needs a reflected
// struct type: the continuous-state snapshot, the optional spawn payload, the saved
// progress record, and one inventory row used to show an array-of-structs parameter.

/**
 * The snapshot a Dynamic entity sends every replication interval. A Dynamic
 * UActorUpdateExecutor returns this struct from GetStateStruct, which is also what
 * registers the type for wire serialization.
 */
USTRUCT(BlueprintType)
struct FSampleEntityState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sample")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sample")
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sample")
	ESampleAnimState Anim = ESampleAnimState::Idle;
};

/**
 * Optional initial data handed to SpawnEntity. The SDK delivers it to every client
 * in OnCrowdySpawned, so both the owner and the remote proxies start from the same
 * values. Keep it small: it travels inside the spawn event.
 */
USTRUCT(BlueprintType)
struct FSampleSpawnInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sample")
	FLinearColor Tint = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sample")
	FString DisplayName;
};

/**
 * Saved and loaded through UCrowdyPersistenceSubsystem. The CrowdyPersistent marker
 * is baked into the registry at cook time so the runtime can discover the type
 * without reading editor-only metadata.
 */
USTRUCT(BlueprintType, meta=(CrowdyPersistent))
struct FSampleProgress
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sample")
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sample")
	FVector LastLocation = FVector::ZeroVector;
};

/** One inventory row, used to show an array-of-structs RPC parameter. */
USTRUCT(BlueprintType)
struct FSampleItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sample")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sample")
	int32 Count = 0;
};
