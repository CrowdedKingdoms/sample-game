#pragma once
#include "CoreMinimal.h"
#include "FSampleGameEventStateBase.generated.h"

USTRUCT(BlueprintType)
struct FSampleGameEventStateBase
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, Category="CrowdySDKTest|Game Event")
	FGuid ObjectID;
	
};
