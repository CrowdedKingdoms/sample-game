// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "UObject/Object.h"
#include "CrowdyObjectEventHandler.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable, EditInlineNew)
class CROWDYSDK_API UCrowdyObjectEventHandler : public UObject
{
	GENERATED_BODY()
	
public:
	
	/**
	 * Called after an actor is spawned and registered.
	 * Apply the initial visual state here — materials, mesh, etc.
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Crowdy Object")
	void OnObjectSpawned(AActor* Actor, const FInstancedStruct& InitialState, bool bIsLocallyOwned);
	virtual void OnObjectSpawned_Implementation(AActor* Actor, const FInstancedStruct& InitialState, bool bIsLocallyOwned) {}

	/**
	 * Apply a state change to an existing object.
	 * Could be health, color, animation state — anything.
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Crowdy Object")
	void OnObjectStateChanged(AActor* Actor, const FInstancedStruct& NewState);
	virtual void OnObjectStateChanged_Implementation(AActor* Actor, const FInstancedStruct& NewState) {}

	/**
	 * Called before the actor is destroyed.
	 * Play dissolve, spawn loot, etc.
	 * Return the delay in seconds before actual destroy — 0 for immediate.
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Crowdy Object")
	float OnObjectDestroyed(AActor* Actor, bool bIsLocallyOwned);
	virtual float OnObjectDestroyed_Implementation(AActor* Actor, bool bIsLocallyOwned) { return 0.f; }
	
	UFUNCTION(BlueprintNativeEvent, Category="Crowdy Object")
	void OnObjectEvent(const FGuid& InstigatorID, const FInstancedStruct& EventPayload);
	virtual void OnObjectEvent_Implementation(const FGuid& InstigatorID, const FInstancedStruct& EventPayload) {}
};
