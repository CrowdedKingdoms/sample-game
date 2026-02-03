#pragma once
#include "CoreMinimal.h"
#include "Templates/SharedPointer.h"

class ICrowdyMessage;

class CROWDYSDK_API FCrowdyMessageParser
{
	
public:
	
	[[nodiscard]] TSharedRef<ICrowdyMessage, ESPMode::ThreadSafe> ParseMessage(const TArray<uint8>& Data);
};
