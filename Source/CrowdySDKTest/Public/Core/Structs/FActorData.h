#pragma once
#include "CoreMinimal.h"
#include "FActorData.generated.h"

USTRUCT(BlueprintType)
struct FActorData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, Category="CrowdySDKTest|Actor Data")
	FVector Location;
	
	UPROPERTY(BlueprintReadWrite, Category="CrowdySDKTest|Actor Data")
	FRotator Rotation;
	
	UPROPERTY(BlueprintReadWrite, Category="CrowdySDKTest|Actor Data")
	FGuid ActorID;
	
	UPROPERTY(BlueprintReadWrite, Category="CrowdySDKTest|Actor Data")
	FVector Velocity;
};