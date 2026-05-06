// Fill out your copyright notice in the Description page of Project Settings.
#include "Utils/UActorUpdatePayloadRegistry.h"

std::atomic<UActorUpdatePayloadRegistry*> UActorUpdatePayloadRegistry::Instance(nullptr);

void UActorUpdatePayloadRegistry::LoadFromDataAsset(const UActorUpdatePayloadType* DataAsset)
{
	ensure(!bLoaded.load());
	ensure(IsInGameThread());
	
	if (!DataAsset) return;
	
	for (const FActorUpdatePayloadTypeEntry& Entry : DataAsset->Entries)
	{
		if (!Entry.ActorUpdateType)
		{
			UE_LOG(LogTemp, Warning, TEXT("[ActorUpdateRegistry] Entry with ID %d has null struct - skipped."), Entry.TypeID)
			continue;
		}
		
		if (IDToStruct.Contains(Entry.TypeID))
		{
			UE_LOG(LogTemp, Error, TEXT("[ActorUpdateRegistry] Duplicate TypeID %d for struct %s - skipped."),
				Entry.TypeID, *Entry.ActorUpdateType->GetName());
			continue;
		}
		
		StructToID.Add(Entry.ActorUpdateType, Entry.TypeID);
		IDToStruct.Add(Entry.TypeID, Entry.ActorUpdateType);
		IDToName.Add(Entry.TypeID, Entry.ActorUpdateName);
		
		UE_LOG(LogTemp, Log, TEXT("[ActorUpdateRegistry] Registered: ID=%d -> %s"), Entry.TypeID, *Entry.ActorUpdateType->GetName())
	}
	
	bLoaded.store(true, std::memory_order_release);
}

bool UActorUpdatePayloadRegistry::GetID(const UScriptStruct* Struct, uint8& OutID) const
{
	if (!bLoaded.load(std::memory_order_acquire))
		return false;
	
	if (!Struct) 
		return false;
	
	const uint8* Found = StructToID.Find(Struct);
	
	if (!Found)
		return false;
	
	OutID = *Found;
	return true;
}

bool UActorUpdatePayloadRegistry::GetName(const uint8 ID, FName& OutName) const
{
	if (!bLoaded.load(std::memory_order_acquire))
		return false;
	
	const FName* Found = IDToName.Find(ID);
	
	if (!Found)
		return false;
	
	OutName = *Found;
	return true;
}

UScriptStruct* UActorUpdatePayloadRegistry::Resolve(const uint8 ID) const
{
	if (!bLoaded.load(std::memory_order_acquire))
		return nullptr;
	
	const TObjectPtr<UScriptStruct>* Found = IDToStruct.Find(ID);
	return Found ? Found->Get() : nullptr;
}
