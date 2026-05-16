// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StructUtils/InstancedStruct.h"
#include "CrowdyObjectComponent.generated.h"


class UCrowdySDKSubsystem;
class UCrowdyObjectManager;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), meta=(DisplayName="Crowdy Object Component"))
class CROWDYSDK_API UCrowdyObjectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCrowdyObjectComponent();
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Crowdy SDK|Crowdy Object Component")
	FGuid GetObjectID() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Crowdy SDK|Crowdy Object Component")
	FGuid GetOwnerID() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Crowdy SDK|Crowdy Object Component")
	bool IsLocallyOwned() const;
	
	// C++ Only
	void DispatchEventForObject(FInstancedStruct& Event) const;
	
protected:
	
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	
	UPROPERTY(EditAnywhere, Category = "Crowdy SDK|Crowdy Object Component", meta=(ToolTip="Only set true for objects you'll be spawning"))
	bool bAutoRegister = false;
	
	UPROPERTY(EditAnywhere, Category = "Crowdy SDK|Crowdy Object Component", meta=(ToolTip="Only set true for objects you'll be spawning"))
	bool bUseDeterministicID = false;
	
	UPROPERTY(EditAnywhere, Category = "Crowdy SDK|Crowdy Object Component")
	int64 Seed = 0;
	
	UPROPERTY()
	FGuid ObjectID;
	
	UPROPERTY()
	FGuid OwnerID;
	
	UPROPERTY(EditAnywhere, Category = "Crowdy SDK|Crowdy Object Component")
	FInstancedStruct ObjectPayload;
	
	UPROPERTY()
	TObjectPtr<UCrowdyObjectManager> CrowdyObjectManager;
	
	UPROPERTY()
	TObjectPtr<UCrowdySDKSubsystem> SDK;
};
