// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Replication/ObjectHandler/CrowdyObjectEventHandler.h"
#include "CrowdyObjectRegistry.generated.h"



USTRUCT(BlueprintType)
struct FCrowdyObjectTypeEntry
{
	GENERATED_BODY()

	/** Numeric type ID — this is what travels over the wire. Compact and fast. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly)
	int32 TypeID = 0;

	/** Human-readable name for debugging */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly)
	FName DebugTypeName;

	/** The actor class to spawn for this object type */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<AActor> ActorClass;

	/** Handler that knows how to apply state to this object type */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<UCrowdyObjectEventHandler> HandlerClass;
};


/**
 * 
 */
UCLASS(BlueprintType, meta=(DisplayName="Crowdy Object Registry"))
class CROWDYSDK_API UCrowdyObjectRegistry : public UDataAsset
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Crowdy SDK|Object Registry")
	TArray<FCrowdyObjectTypeEntry> ObjectTypes;
	
	const FCrowdyObjectTypeEntry* FindByTypeID(const int32 TypeID) const
	{
		for (const FCrowdyObjectTypeEntry& Entry : ObjectTypes)
		{
			if (Entry.TypeID == TypeID)
			{
				return &Entry;
			}
		}
		
		return nullptr;
	};
	
	const FCrowdyObjectTypeEntry* FindByClass(TSubclassOf<AActor> Class) const
	{
		for (const FCrowdyObjectTypeEntry& Entry : ObjectTypes)
		{
			if (Entry.ActorClass.Get() == Class)
			{
				return &Entry;
			}
		}
		
		return nullptr;
	};
	
	const FCrowdyObjectTypeEntry* FindByEventType(const TObjectPtr<UScriptStruct> EventType) const
	{
		for (const FCrowdyObjectTypeEntry& Entry : ObjectTypes)
		{
			if (Entry.HandlerClass.Get() == EventType.GetClass())
			{
				return &Entry;
			}
		}
		return nullptr;
	}
	
	
};
