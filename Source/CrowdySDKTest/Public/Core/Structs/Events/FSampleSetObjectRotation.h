#pragma once
#include "CoreMinimal.h"
#include "FSampleSetObjectRotation.generated.h"

USTRUCT(BlueprintType)
struct FSampleSetObjectRotation
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	FGuid ObjectID;
	
	UPROPERTY(BlueprintReadWrite)
	FRotator Rotation;
};