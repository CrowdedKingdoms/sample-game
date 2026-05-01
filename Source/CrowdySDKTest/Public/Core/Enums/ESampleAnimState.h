#pragma once
#include "CoreMinimal.h"

UENUM(BlueprintType, meta=(DisplayName="Sample Anim State"))
enum class ESampleAnimState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Walk UMETA(DisplayName = "Walk"),
	Run UMETA(DisplayName = "Run"),
	Jump UMETA(DisplayName = "Jump"),
	Fall UMETA(DisplayName = "Fall"),
	Land UMETA(DisplayName = "Land"),
};
