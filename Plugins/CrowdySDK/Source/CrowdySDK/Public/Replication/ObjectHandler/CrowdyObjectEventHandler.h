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
	 * Return the struct type this handler responds to.
	 * Subsystem uses this to route events to the right handler.
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Crowdy Object")
	TArray<UScriptStruct*> GetHandledStructTypes() const;
	virtual TArray<UScriptStruct*> GetHandledStructTypes_Implementation() const { return {}; }

	/**
	 * Called when a new object with this payload type is registered.
	 * Use this to spawn your visual representation.
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Crowdy Object")
	void OnObjectRegistered(const FGuid& UUID, const FGuid& OwnerID, const FInstancedStruct& Payload, AActor* Actor, bool bIsLocallyOwned);
	virtual void OnObjectRegistered_Implementation(const FGuid& UUID, const FGuid& OwnerID, const FInstancedStruct& Payload, AActor* Actor, bool bIsLocallyOwned) {}

	/**
	 * Called when an object event is dispatched for this payload type.
	 * Fully user-defined — could be health change, color change, state change, etc.
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Crowdy Object")
	void OnObjectEvent(const FGuid& InstigatorID, const FInstancedStruct& EventPayload);
	virtual void OnObjectEvent_Implementation(const FGuid& InstigatorID, const FInstancedStruct& EventPayload) {}

	/**
	 * Called when the object is unregistered.
	 * Use this to destroy your visual representation.
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Crowdy Object")
	void OnObjectUnregistered(const FGuid& UUID);
	virtual void OnObjectUnregistered_Implementation(const FGuid& UUID) {}
};
