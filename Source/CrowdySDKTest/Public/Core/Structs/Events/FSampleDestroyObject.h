#pragma once
#include "CoreMinimal.h"
#include "FSampleDestroyObject.generated.h"

USTRUCT(BlueprintType)
struct FSampleDestroyObject
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	FGuid ObjectID;
};