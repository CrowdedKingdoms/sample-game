#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SampleMirrorSource.generated.h"

class APawn;

UINTERFACE(BlueprintType, MinimalAPI)
class USampleMirrorSource : public UInterface
{
	GENERATED_BODY()
};

/**
 * A mimic entity implements this so USampleMimicExecutor can read what to mirror without
 * knowing the concrete class. The C++ ASampleMimicEntity implements it natively; a pure
 * Blueprint mimic entity implements the two getters in its graph. Either way the same
 * executor reads the target pawn and the mirror plane.
 */
class ISampleMirrorSource
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Sample|Mirror")
	APawn* GetMirrorTarget() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Sample|Mirror")
	FTransform GetMirrorPlaneTransform() const;
};
