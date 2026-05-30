#pragma once
#include "CoreMinimal.h"
#include "FSampleSetObjectRotation.generated.h"

USTRUCT(BlueprintType, meta=(CrowdyRep="Event"))
struct FSampleSetObjectRotation
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	FRotator Rotation;
};