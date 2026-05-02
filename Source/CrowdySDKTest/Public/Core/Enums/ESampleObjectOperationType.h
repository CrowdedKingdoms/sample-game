#pragma once
#include "CoreMinimal.h"
#include "ESampleObjectOperationType.generated.h"

UENUM(BlueprintType)
enum class ESampleObjectOperationType : uint8
{
	Spawn,
	Destroy,
	UpdateState,
};