#include "FCrowdyTransmissionLayerUDP.h"
#include "Subsystem/CrowdyGameSession.h"
#include "Utils/SerializationFunctionLibrary.h"

void FCrowdyTransmissionLayerUDP::SendBytes(TArray<uint8>&& Data)
{
	if (!IsValid(GameSession) || Data.IsEmpty())
		return;
	
	const int64 GameTokenID = GameSession->GetGameTokenID();
	TArray<uint8> GameTokenIDBytes;
	GameTokenIDBytes.SetNumUninitialized(8);
	FMemory::Memcpy(GameTokenIDBytes.GetData(), &GameTokenID, 8);
	const TArray<uint8> HMAC = USerializationFunctionLibrary::CalculateHMAC(Data, GameSession->GetGameToken());
	
	Data.Append(HMAC);
	Data.Append(GameTokenIDBytes);
	
	if (!GameSession->EnqueueMessageToSend(MoveTemp(Data)))
		UE_LOG(LogTemp, Warning, TEXT("[CrowdySDK] Failed to enqueue message to send queue in UDP Transmission Layer"));
	
}
