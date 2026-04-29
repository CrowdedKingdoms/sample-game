#pragma once
#include "CoreMinimal.h"
#include "Templates/SharedPointer.h"

class FCrowdyServiceRegistry;
class UCrowdyUDPSubsystem;
class ICrowdyMessage;

class CROWDYSDK_API FCrowdyMessageParser
{
	
public:
	
	FCrowdyMessageParser(FCrowdyServiceRegistry* InServiceRegistry, UCrowdyUDPSubsystem* InUDPSubsystem);
	~FCrowdyMessageParser() = default;
	
	[[nodiscard]] TSharedRef<ICrowdyMessage, ESPMode::ThreadSafe> ParseMessage(const TArray<uint8>& Data);
	void SetExpectedActorStateSize(const int32 NewSize);
private:
	FCrowdyServiceRegistry* ServiceRegistry;
	UCrowdyUDPSubsystem* UDPSubsystem;
	int32 ExpectedActorStateSize = 300;
};
