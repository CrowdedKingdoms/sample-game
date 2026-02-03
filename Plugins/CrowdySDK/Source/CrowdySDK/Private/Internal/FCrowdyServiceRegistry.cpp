#include "FCrowdyServiceRegistry.h"
#include "Core/UDP/Interfaces/ICrowdyMessage.h"
#include "Core/UDP/Interfaces/ICrowdyReceptionLayer.h"

void FCrowdyServiceRegistry::RegisterService(FName Name, ICrowdyService* Service)
{
	Services.Add(Name, Service);
}

void FCrowdyServiceRegistry::RegisterReceptionLayer(ICrowdyReceptionLayer* Layer)
{
	const auto& SupportedTypes = Layer->GetSupportedResponseTypes();
	for (const auto Type : SupportedTypes)
	{
		ReceptionLayersByType.Add(Type, Layer);
	}
}

void FCrowdyServiceRegistry::DispatchMessage(const TSharedRef<ICrowdyMessage, ESPMode::ThreadSafe>& Message)
{

	const auto ResponseType = Message->GetType();
	
	ICrowdyReceptionLayer* ReceptionLayer = ReceptionLayersByType.FindRef(ResponseType);
	
	if (!ReceptionLayer)
	{
		return;
	}
	
	ReceptionLayer->OnMessageReceived(Message);
}
