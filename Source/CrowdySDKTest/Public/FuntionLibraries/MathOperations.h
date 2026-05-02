// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MathOperations.generated.h"

/**
 * 
 */
UCLASS()
class CROWDYSDKTEST_API UMathOperations : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintCallable, Category = "CrowdySDKTest|Math")
	static void GetMimicTransform(
		const FVector& SourceLocation,
		const FRotator& SourceRotation,
		const FVector& CenterLocation,
		const FRotator& CenterRotation,
		const float ForwardOffset,
		FVector& OutLocation,
		FRotator& OutRotation);
};
