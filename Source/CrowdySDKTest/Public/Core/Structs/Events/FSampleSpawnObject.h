#pragma once
#include "CoreMinimal.h"
#include "FSampleSpawnObject.generated.h"

USTRUCT(BlueprintType)
struct FSampleSpawnObject
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	FGuid ObjectID;
	
	UPROPERTY(BlueprintReadWrite)
	FName ObjectType;
	
	UPROPERTY(BlueprintReadWrite)
	FVector SpawnLocation;
	
	UPROPERTY(BlueprintReadWrite)
	FRotator SpawnRotation;
};