#include "Utils/UEventPayloadRegistry.h"

std::atomic<UEventPayloadRegistry*> UEventPayloadRegistry::Instance(nullptr);

void UEventPayloadRegistry::LoadFromDataAsset(const UEventPayloadType* DataAsset)
{
	ensure(!bLoaded.load());
	ensure(IsInGameThread());

	if (!DataAsset) return;

	for (const FEventPayloadTypeEntry& Entry : DataAsset->Entries)
	{
		if (!Entry.EventType)
		{
			UE_LOG(LogTemp, Warning, TEXT("[PayloadTypeRegistry] Entry with ID %d has null struct — skipped."), Entry.TypeID);
			continue;
		}

		if (IDToStruct.Contains(Entry.TypeID))
		{
			UE_LOG(LogTemp, Error, TEXT("[PayloadTypeRegistry] Duplicate TypeID %d for struct %s — skipped."),
				Entry.TypeID, *Entry.EventType->GetName());
			continue;
		}

		StructToID.Add(Entry.EventType->GetFName(), Entry.TypeID);
		IDToStruct.Add(Entry.TypeID, Entry.EventType);
		IDToName.Add(Entry.TypeID, Entry.EventName);

		UE_LOG(LogTemp, Log, TEXT("[PayloadTypeRegistry] Registered: ID=%d -> %s"),
			Entry.TypeID, *Entry.EventType->GetName());
	}

	bLoaded.store(true, std::memory_order_release);
}

bool UEventPayloadRegistry::GetID(const UScriptStruct* Struct, int32& OutID) const
{
	if (!bLoaded.load(std::memory_order_acquire)) return false;
	if (!Struct) return false;

	const int32* Found = StructToID.Find(Struct->GetFName());
	if (Found) { OutID = *Found; return true; }
	return false;
}

bool UEventPayloadRegistry::GetName(const int32 ID, FName& OutName) const
{
	if (!bLoaded.load(std::memory_order_acquire)) return false;

	const FName* Found = IDToName.Find(ID);
	if (!Found) return false;

	OutName = *Found;
	return true;
}

UScriptStruct* UEventPayloadRegistry::Resolve(const int32 ID) const
{
	if (!bLoaded.load(std::memory_order_acquire)) return nullptr;

	const TObjectPtr<UScriptStruct>* Found = IDToStruct.Find(ID);
	return Found ? Found->Get() : nullptr;
}
