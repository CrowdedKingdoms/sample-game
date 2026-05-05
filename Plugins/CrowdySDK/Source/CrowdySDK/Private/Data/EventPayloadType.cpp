// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/EventPayloadType.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"

void UEventPayloadType::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    
    constexpr int32 RegistryMin = 50;
    
    TMap<int32, int32> SeenIDs;
    TSet<FName> SeenNames;

    for (int32 i = 0; i < Entries.Num(); i++)
    {
        // --- TypeID resolution ---
        const int32 ID = Entries[i].TypeID;

        if (ID < RegistryMin)
        {
            // Find next available ID >= RegistryMin
            int32 Next = RegistryMin;
            while (SeenIDs.Contains(Next)) Next++;

            UE_LOG(LogTemp, Warning,
                TEXT("[UEventPayloadType]: TypeID %d at index %d is reserved for legacy events (< %d) — reassigned to %d."),
                ID, i, RegistryMin, Next);

            Entries[i].TypeID = Next;
            SeenIDs.Add(Next, i);
        }
        else if (SeenIDs.Contains(ID))
        {
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

        // --- EventName resolution ---
        if (Entries[i].EventName.IsNone())
        {
            Entries[i].EventName = FName(*FString::Printf(TEXT("Event_%d"), Entries[i].TypeID));
        }

        if (SeenNames.Contains(Entries[i].EventName))
        {
            FName Unique = FName(*FString::Printf(TEXT("%s_%d"), *Entries[i].EventName.ToString(), i));

            UE_LOG(LogTemp, Warning,
                TEXT("[UEventPayloadType]: Duplicate EventName '%s' at index %d — reassigned to '%s'."),
                *Entries[i].EventName.ToString(), i, *Unique.ToString());

            Entries[i].EventName = Unique;
        }

        SeenNames.Add(Entries[i].EventName);
    }
}

EDataValidationResult UEventPayloadType::IsDataValid(FDataValidationContext& Context) const
{
    EDataValidationResult Result = Super::IsDataValid(Context);

    constexpr int32 RegistryMin = 50;

    TMap<int32, int32> SeenIDs;
    TSet<const UScriptStruct*> SeenStructs;
    TMap<FName, int32> SeenNames;

    for (int32 i = 0; i < Entries.Num(); i++)
    {
        const FEventPayloadTypeEntry& Entry = Entries[i];

        // --- TypeID range ---
        if (Entry.TypeID < RegistryMin)
        {
            Context.AddError(FText::FromString(
                FString::Printf(TEXT("Entry[%d] TypeID %d is reserved for legacy events — must be >= %d."),
                    i, Entry.TypeID, RegistryMin)));
            Result = EDataValidationResult::Invalid;
        }

        // --- EventType struct ---
        if (!Entry.EventType)
        {
            Context.AddError(FText::FromString(
                FString::Printf(TEXT("Entry[%d] has no StructType assigned."), i)));
            Result = EDataValidationResult::Invalid;
            continue;
        }

        // --- Duplicate TypeID ---
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

        // --- Duplicate EventType struct ---
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

        // --- EventName ---
        if (Entry.EventName.IsNone())
        {
            Context.AddError(FText::FromString(
                FString::Printf(TEXT("Entry[%d] has no EventName assigned."), i)));
            Result = EDataValidationResult::Invalid;
        }
        else if (SeenNames.Contains(Entry.EventName))
        {
            Context.AddError(FText::FromString(
                FString::Printf(TEXT("Entry[%d] EventName '%s' is already used by Entry[%d]."),
                    i, *Entry.EventName.ToString(), SeenNames[Entry.EventName])));
            Result = EDataValidationResult::Invalid;
        }
        else
        {
            SeenNames.Add(Entry.EventName, i);
        }
    }

    return Result;
}
#endif
