#include "FCrowdyServiceRegistry.h"
#include "Core/UDP/Interfaces/ICrowdyMessage.h"
#include "Core/UDP/Interfaces/ICrowdyReceptionLayer.h"

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
}

void FCrowdyServiceRegistry::DeregisterAllReceptionLayers()
{
	ReceptionLayersByType.Empty();
	UE_LOG(LogTemp, Log, TEXT("Deregistered all reception layers."));
}


void FCrowdyServiceRegistry::DispatchMessage(const TSharedRef<ICrowdyMessage, ESPMode::ThreadSafe>& Message)
{

	const auto ResponseType = Message->GetType();

	if (TArray<ICrowdyReceptionLayer*>* Layers = ReceptionLayersByType.Find(ResponseType))
	{
		// Iterate safely
		for (ICrowdyReceptionLayer* Layer : *Layers)
		{
			if (Layer)
			{
				Layer->OnMessageReceived(Message);
			}
		}
	}
}
