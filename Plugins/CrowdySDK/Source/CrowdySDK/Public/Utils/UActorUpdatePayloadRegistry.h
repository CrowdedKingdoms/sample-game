// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/ActorUpdatePayloadType.h"
#include "UObject/Object.h"
#include "UActorUpdatePayloadRegistry.generated.h"

/**
 * 
 */
UCLASS()
class CROWDYSDK_API UActorUpdatePayloadRegistry : public UObject
{
	GENERATED_BODY()
	
public:
	
	static UActorUpdatePayloadRegistry* Get()
	{
		if (!Instance.load(std::memory_order_acquire))
		{
			UActorUpdatePayloadRegistry* NewRegistry = NewObject<UActorUpdatePayloadRegistry>();
			NewRegistry->AddToRoot();
			
			UActorUpdatePayloadRegistry* Expected = nullptr;
			
			if (!Instance.compare_exchange_strong(Expected, NewRegistry, std::memory_order_release))
				NewRegistry->RemoveFromRoot();
		}
		return Instance.load(std::memory_order_acquire);
	}
	
	static void Shutdown()
	{
		UActorUpdatePayloadRegistry* Current = Instance.exchange(nullptr, std::memory_order_acq_rel);
		if (Current) Current->RemoveFromRoot();
	}
	
	void LoadFromDataAsset(const UActorUpdatePayloadType* DataAsset);
	bool GetID(const UScriptStruct* Struct, uint8& OutID) const;
	bool GetName(uint8 ID, FName& OutName) const;
	UScriptStruct* Resolve(uint8 ID) const;
	
	bool IsLoaded() const { return bLoaded.load(std::memory_order_acquire); }
	
	void Reset()
	{
		ensure(IsInGameThread());
		IDToStruct.Reset();
		StructToID.Reset();
		IDToName.Reset();
		bLoaded.store(false, std::memory_order_release);
	}
	
private:
	
	UPROPERTY()
	TMap<uint8, TObjectPtr<UScriptStruct>> IDToStruct;

	UPROPERTY()
	TMap<const UScriptStruct*, uint8> StructToID;
	
	TMap<int32, FName> IDToName;
	
	std::atomic<bool> bLoaded { false };
	static std::atomic<UActorUpdatePayloadRegistry*> Instance;
};
