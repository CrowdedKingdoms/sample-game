#include "FCrowdyServiceRegistry.h"
#include "Core/UDP/Interfaces/ICrowdyMessage.h"
#include "Core/UDP/Interfaces/ICrowdyReceptionLayer.h"
#include "Messages/GameObjects/FGameEventNotification.h"
#include "Utils/UEventPayloadRegistry.h"

void FCrowdyServiceRegistry::RegisterService(FName Name, ICrowdyService* Service)
{
	Services.Add(Name, Service);
}

void FCrowdyServiceRegistry::RegisterReceptionLayer(ICrowdyReceptionLayer* Layer)
{
	
	if (!Layer)
		return;
	
	if (!Layer) return;

	const auto& SupportedTypes = Layer->GetSupportedResponseTypes();
	
	for (ECrowdyMessageType Type : SupportedTypes)
	{
		// Preallocate small buffer to avoid reallocations
		if (!ReceptionLayersByType.Contains(Type))
		{
			ReceptionLayersByType.Add(Type, TArray<ICrowdyReceptionLayer*>());
			ReceptionLayersByType[Type].Reserve(8);
		}

		ReceptionLayersByType[Type].Add(Layer);
	}
	
	const auto& SubscribedEvents = Layer->GetSupportedEventTypes();
	
	if (SubscribedEvents.IsEmpty())
	{
		UnfilteredEventLayers.Add(Layer);
		return;
	}
	
	for (auto& Events : SubscribedEvents)
	{
		if (!SubscribedEventLayers.Contains(Events))
		{
			SubscribedEventLayers.Add(Events, TArray<ICrowdyReceptionLayer*>());
			SubscribedEventLayers[Events].Reserve(8);
		}
		
		SubscribedEventLayers[Events].Add(Layer);
	}
}

void FCrowdyServiceRegistry::DeregisterAllReceptionLayers()
{
	ReceptionLayersByType.Empty();
	UnfilteredEventLayers.Empty();
	SubscribedEventLayers.Empty();
	UE_LOG(LogTemp, Log, TEXT("Deregistered all reception layers."));
}

bool FCrowdyServiceRegistry::IsLayerRegistered(const ICrowdyReceptionLayer* Layer) const
{
	if (!Layer)
		return false;
	
	const auto& SupportedTypes = Layer->GetSupportedResponseTypes();
	
	for (const ECrowdyMessageType Type : SupportedTypes)
	{
		if (!ReceptionLayersByType.Contains(Type))
		{
			UE_LOG(LogTemp, Warning, TEXT("[CrowdyServiceRegistry]: Layer Not registered"));
			return false;
		}
	}
	
	return true;
}


void FCrowdyServiceRegistry::DispatchMessage(const TSharedRef<ICrowdyMessage, ESPMode::ThreadSafe>& Message)
{

	const auto ResponseType = Message->GetType();
	
	if (ResponseType == ECrowdyMessageType::CLIENT_EVENT_NOTIFICATION)
	{
		DispatchEventNotification(Message);
		return;
	}
	
	DispatchToLayers(ResponseType, Message);
}

void FCrowdyServiceRegistry::DispatchToLayers(const ECrowdyMessageType ResponseType,
	const TSharedRef<ICrowdyMessage, ESPMode::ThreadSafe>& Message)
{
	const TArray<ICrowdyReceptionLayer*>* Layers = ReceptionLayersByType.Find(ResponseType);
	if (!Layers)
		return;

	for (ICrowdyReceptionLayer* Layer : *Layers)
	{
		if (Layer)
			Layer->OnMessageReceived(Message);
	}
}

void FCrowdyServiceRegistry::DispatchEventNotification(const TSharedRef<ICrowdyMessage, ESPMode::ThreadSafe>& Message)
{
	const auto& EventMessage = static_cast<const FGameEventNotification&>(*Message);

	FName EventName;
	
	const bool bIsRegisteredEvent = UEventPayloadRegistry::Get()->GetName(EventMessage.EventType, EventName);
	
	if (!bIsRegisteredEvent)
	{
		// Legacy layers — always receive all events
		for (ICrowdyReceptionLayer* Layer : UnfilteredEventLayers)
		{
			if (Layer) Layer->OnMessageReceived(Message);
		}
		return;
	}
	
	// New layers — filtered by event name
	if (const TArray<ICrowdyReceptionLayer*>* SubscribedLayers = SubscribedEventLayers.Find(EventName))
	{
		for (ICrowdyReceptionLayer* Layer : *SubscribedLayers)
		{
			if (Layer) Layer->OnMessageReceived(Message);
		}
	}

	
}
