#pragma once
#include "CoreMinimal.h"
#include "FSampleActorState.generated.h"


USTRUCT(BlueprintType, meta=(CrowdyRep="ActorUpdate"))
struct FSampleActorState
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, Category="CrowdySDKTest|Actor Update")
	FVector Location = FVector::ZeroVector;
	
	UPROPERTY(BlueprintReadWrite, Category="CrowdySDKTest|Actor Update")
	FRotator Rotation = FRotator::ZeroRotator;

};