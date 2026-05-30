#pragma once
#include "CoreMinimal.h"
#include "FSampleSetObjectLocation.generated.h"

USTRUCT(BlueprintType, meta=(CrowdyRep="Event"))
struct FSampleSetObjectLocation
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	FVector Location;
};