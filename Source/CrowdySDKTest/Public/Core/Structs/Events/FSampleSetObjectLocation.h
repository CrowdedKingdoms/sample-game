#pragma once
#include "CoreMinimal.h"
#include "FSampleSetObjectLocation.generated.h"

USTRUCT(BlueprintType)
struct FSampleSetObjectLocation
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	FGuid ObjectID;
	
	UPROPERTY(BlueprintReadWrite)
	FVector Location;
};