#pragma once
#include "Data/EventPayloadType.h"
#include "UEventPayloadRegistry.generated.h"

UCLASS()
class CROWDYSDK_API UEventPayloadRegistry : public UObject
{
	GENERATED_BODY()

public:
	
	static UEventPayloadRegistry* Get()
	{
		if (!Instance.load(std::memory_order_acquire))
		{
			UEventPayloadRegistry* NewRegistry = NewObject<UEventPayloadRegistry>();
			NewRegistry->AddToRoot();

			UEventPayloadRegistry* Expected = nullptr;
			if (!Instance.compare_exchange_strong(Expected, NewRegistry, std::memory_order_release))
				NewRegistry->RemoveFromRoot();
		}
		return Instance.load(std::memory_order_acquire);
	}
	
	static void Shutdown()
	{
		UEventPayloadRegistry* Current = Instance.exchange(nullptr, std::memory_order_acq_rel);
		if (Current) Current->RemoveFromRoot();
	}
	
	void LoadFromDataAsset(const UEventPayloadType* DataAsset);
	bool GetID(const UScriptStruct* Struct, int32& OutID) const;
	bool GetName(const int32 ID, FName& OutName) const;
	UScriptStruct* Resolve(const int32 ID) const;
	
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
	TMap<int32, TObjectPtr<UScriptStruct>> IDToStruct;

	UPROPERTY()
	TMap<const UScriptStruct*, int32> StructToID;
	
	TMap<int32, FName> IDToName;
	
	std::atomic<bool> bLoaded { false };
	static std::atomic<UEventPayloadRegistry*> Instance;
	
};

