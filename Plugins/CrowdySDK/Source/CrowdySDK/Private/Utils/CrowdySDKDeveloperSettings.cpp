// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/CrowdySDKDeveloperSettings.h"
#if WITH_EDITOR
void UCrowdySDKDeveloperSettings::ValidateObjectHandlerBindings()
{
	// Load the registry
	const UEventPayloadType* Registry = EventPayloadDataAsset.LoadSynchronous();
	if (!IsValid(Registry)) return;

	// Build valid name set from registry
	TSet<FName> ValidEventNames;
	for (const FEventPayloadTypeEntry& Entry : Registry->Entries)
	{
		if (Entry.EventName != NAME_None)
			ValidEventNames.Add(Entry.EventName);
	}

	// Validate every binding across all maps
	for (auto& [WorldPtr, Config] : ObjectManagerConfigs)
	{
		for (FCrowdyObjectHandlerBinding& Binding : Config.HandlerBindings)
		{
			if (Binding.EventName == NAME_None)
				continue;

			if (!ValidEventNames.Contains(Binding.EventName))
			{
				UE_LOG(LogTemp, Warning,
					TEXT("[CrowdySDK]: ObjectHandler binding EventName '%s' does not exist in EventPayloadType registry. Did you mistype it?"),
					*Binding.EventName.ToString());
			}
		}
	}
}

void UCrowdySDKDeveloperSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.GetPropertyName();

	if (PropertyName == GET_MEMBER_NAME_CHECKED(FCrowdyObjectHandlerBinding, EventName))
	{
		ValidateObjectHandlerBindings();
	}
}
#endif
