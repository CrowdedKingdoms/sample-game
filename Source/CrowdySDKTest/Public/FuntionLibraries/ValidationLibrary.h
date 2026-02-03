// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ValidationLibrary.generated.h"

/**
 * 
 */
UCLASS()
class CROWDYSDKTEST_API UValidationLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintCallable, Category = "CrowdySDKTest|Validation")
	static bool ValidateEmail(const FString Email);
};
