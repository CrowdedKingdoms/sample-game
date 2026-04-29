#pragma once
#include "Templates/SharedPointer.h"
#include "HAL/PlatformAtomics.h"

enum class ECrowdyMessageType : uint8;
class ICrowdyService;
class ICrowdyReceptionLayer;
class ICrowdyMessage;

/**
 * Internal-only registry.
 * Owns services and dispatches messages to reception layers.
 */
class CROWDYSDK_API FCrowdyServiceRegistry
{
public:
	FCrowdyServiceRegistry() = default;
	~FCrowdyServiceRegistry() = default;
	
	void RegisterService(FName Name, ICrowdyService* Service);
	
	void RegisterReceptionLayer(ICrowdyReceptionLayer* Layer);
	
	void DeregisterAllReceptionLayers();
	
	void DispatchMessage(const TSharedRef<ICrowdyMessage, ESPMode::ThreadSafe>& Message);
	
private:
	
	/** Named Serviced **/
	TMap<FName, ICrowdyService*> Services;
	
	/** All Message Listeners **/
	TArray<ICrowdyReceptionLayer*> ReceptionLayers;
	
	/** Map of message type to reception layers */
	TMap<ECrowdyMessageType, TArray<ICrowdyReceptionLayer*>> ReceptionLayersByType;
	
};
