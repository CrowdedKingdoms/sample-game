#pragma once
#include "CoreMinimal.h"
#include "Core/Enums/ESampleAnimState.h"
#include "FChangeAnimState.generated.h"


USTRUCT(BlueprintType)
struct FChangeAnimState
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	FGuid TargetID;
	
	UPROPERTY(BlueprintReadWrite)
	ESampleAnimState NewState;
};