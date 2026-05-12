// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Replication/Executor/ActorUpdateExecutor.h"
#include "StructUtils/InstancedStruct.h"
#include "CrowdyActorUpdateComponent.generated.h"

class UActorUpdateExecutor;
class UCrowdyAutoReplicator;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent, DisplayName="Crowdy Actor Update Component"))
class CROWDYSDK_API UCrowdyActorUpdateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCrowdyActorUpdateComponent();
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;
	
	
	UFUNCTION(BlueprintCallable, Category="Crowdy SDK|Crowdy Actor Update Component")
	void StartReplication();
	
	UFUNCTION(BlueprintCallable, Category="Crowdy SDK|Crowdy Actor Update Component")
	void StopReplication();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Crowdy SDK|Crowdy Actor Update Component|Data")
	FString GetUUID()
	{
		return UUID;
	}

	[[nodiscard]] FORCEINLINE const FString& GetUUID() const
	{
		return UUID;
	}
	
	[[nodiscard]] FORCEINLINE const FInstancedStruct& GetActorState() const
	{
		
		CachedActorState = ActorUpdateExecutor->GetActorState(this);
		return CachedActorState;
	}
	
	[[nodiscard]] FORCEINLINE AActor* GetCachedOwner() const
	{
		return CachedOwner;
	}


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	
	UPROPERTY(EditAnywhere, Category = "Crowdy SDK|Crowdy Actor Update Component|Data")
	bool bAutoStartReplication = true;
	
	UPROPERTY()
	FString UUID;
	
	UPROPERTY()
	mutable FInstancedStruct CachedActorState;
	
	UPROPERTY()
	AActor* CachedOwner;
	
	UPROPERTY()
	UCrowdyAutoReplicator* AutoReplicator;
	
	UPROPERTY(EditAnywhere, Instanced, Category = "Crowdy SDK|Crowdy Actor Update Component|Data")
	TObjectPtr<UActorUpdateExecutor> ActorUpdateExecutor;
	
};
