#include "Sample/SampleBlueprintHelpers.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Replication/Subsystems/CrowdyActorManager.h"
#include "Sample/SampleTypes.h"

UScriptStruct* USampleBlueprintHelpers::GetSampleEntityStateStruct()
{
	return FSampleEntityState::StaticStruct();
}

void USampleBlueprintHelpers::RegisterCrowdyStateProxy(UObject* WorldContextObject, UScriptStruct* StateStruct, TSubclassOf<AActor> ProxyClass)
{
	if (!StateStruct || !ProxyClass)
	{
		return;
	}

	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
	if (!World)
	{
		return;
	}

	if (UCrowdyActorManager* Manager = World->GetSubsystem<UCrowdyActorManager>())
	{
		Manager->RegisterStateClass(StateStruct, ProxyClass);
	}
}
