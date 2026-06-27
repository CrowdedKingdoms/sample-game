#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SampleBlueprintHelpers.generated.h"

/**
 * Two small bridges the Blueprint mimic needs that the SDK does not expose directly:
 * a node that yields the FSampleEntityState struct type (Blueprint has no struct-type
 * literal), and a node that re-points the dynamic-pool's state-struct-proxy-class
 * mapping (UCrowdyActorManager::RegisterStateClass is not BlueprintCallable).
 */
UCLASS()
class CROWDYSDKTEST_API USampleBlueprintHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/** The struct the player pawn and the mimic both stream. Feed it to Register Crowdy State Proxy. */
	UFUNCTION(BlueprintPure, Category="Sample|Mimic")
	static UScriptStruct* GetSampleEntityStateStruct();

	/**
	 * Make the actor pool spawn ProxyClass for entities whose continuous state is StateStruct.
	 * Call this on the owner right after spawning the mimic so the reflection materializes as
	 * the player character instead of the invisible source actor.
	 */
	UFUNCTION(BlueprintCallable, Category="Sample|Mimic", meta=(WorldContext="WorldContextObject"))
	static void RegisterCrowdyStateProxy(UObject* WorldContextObject, UScriptStruct* StateStruct, TSubclassOf<AActor> ProxyClass);
};
