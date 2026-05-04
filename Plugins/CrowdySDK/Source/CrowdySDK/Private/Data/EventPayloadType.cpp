// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/EventPayloadType.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"

void UEventPayloadType::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	TMap<int32, int32> SeenIDs; // TypeID -> first index that used it

	for (int32 i = 0; i < Entries.Num(); i++)
	{
		const int32 ID = Entries[i].TypeID;

		if (ID <= 0)
		{
			// Auto-assign the next available ID instead of leaving 0
			int32 Next = 1;
			while (SeenIDs.Contains(Next)) Next++;
			Entries[i].TypeID = Next;
			SeenIDs.Add(Next, i);
			continue;
		}

		if (SeenIDs.Contains(ID))
		{
			// Duplicate found — auto-increment until unique
			int32 Next = ID + 1;
			while (SeenIDs.Contains(Next)) Next++;

			UE_LOG(LogTemp, Warning,
				TEXT("[UEventPayloadType]: Duplicate TypeID %d at index %d — reassigned to %d."),
				ID, i, Next);

			Entries[i].TypeID = Next;
			SeenIDs.Add(Next, i);
		}
		else
		{
			SeenIDs.Add(ID, i);
		}
	}
}

EDataValidationResult UEventPayloadType::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result =  Super::IsDataValid(Context);
	
	TMap<int32, int32> SeenIDs;
	TSet<const UScriptStruct*> SeenStructs;

	for (int32 i = 0; i < Entries.Num(); i++)
	{
		const FEventPayloadTypeEntry& Entry = Entries[i];

		if (Entry.TypeID <= 0)
		{
			Context.AddError(FText::FromString(
				FString::Printf(TEXT("Entry[%d] has invalid TypeID %d — must be > 0."), i, Entry.TypeID)));
			Result = EDataValidationResult::Invalid;
		}

		if (!Entry.EventType)
		{
			Context.AddError(FText::FromString(
				FString::Printf(TEXT("Entry[%d] has no StructType assigned."), i)));
			Result = EDataValidationResult::Invalid;
			continue;
		}

		if (SeenIDs.Contains(Entry.TypeID))
		{
			Context.AddError(FText::FromString(
				FString::Printf(TEXT("Entry[%d] TypeID %d is already used by Entry[%d]."),
					i, Entry.TypeID, SeenIDs[Entry.TypeID])));
			Result = EDataValidationResult::Invalid;
		}
		else
		{
			SeenIDs.Add(Entry.TypeID, i);
		}

		if (SeenStructs.Contains(Entry.EventType))
		{
			Context.AddError(FText::FromString(
				FString::Printf(TEXT("Entry[%d] EventType '%s' is already registered under a different ID."),
					i, *Entry.EventType->GetName())));
			Result = EDataValidationResult::Invalid;
		}
		else
		{
			SeenStructs.Add(Entry.EventType);
		}
	}

	return Result;
}
#endif
