#include "FCrowdyMessageParser.h"
#include "Core/UDP/Enums/ECrowdyMessageType.h"
#include "Messages/FDefaultMessage.h"
#include "Messages/Actor/FActorUpdateNotificationMessage.h"
#include "Messages/Actor/FActorUpdateResponseMessage.h"
#include "Messages/Communication/FClientAudioNotification.h"
#include "Messages/GameObjects/FGameEventNotification.h"
#include "Messages/GameObjects/FGameObjectActivationNotification.h"
#include "Messages/Voxel/FVoxelUpdateNotificationMessage.h"
#include "Messages/Voxel/FVoxelUpdateResponseMessage.h"

TSharedRef<ICrowdyMessage, ESPMode::ThreadSafe> FCrowdyMessageParser::ParseMessage(const TArray<uint8>& Data)
{
	if (Data.Num() < 1)
	{
		return MakeShared<FDefaultMessage>();
	}
	
	const ECrowdyMessageType MessageType = static_cast<ECrowdyMessageType>(Data[0]);
	
	if (MessageType == ECrowdyMessageType::CLIENT_AUDIO_NOTIFICATION)
		UE_LOG(LogTemp, Log, TEXT("Voice Packet Received"));

	// Handle MESSAGE_BUNDLE before normal dispatch
	if (MessageType == ECrowdyMessageType::MESSAGE_BUNDLE)
	{
		int32 Offset = 1; // Skip bundle message type byte

		// We can have multiple messages in the bundle; for now we return the first valid one.
		while (Offset < Data.Num())
		{
			// Need at least 2 bytes for message length (uint16)
			if (Offset + 2 > Data.Num())
			{
				UE_LOG(LogTemp, Warning, TEXT("[CrowdySDK] MESSAGE_BUNDLE: Not enough bytes for MessageLength at offset %d"), Offset);
				return MakeShared<FDefaultMessage, ESPMode::ThreadSafe>();
			}

			// Read length (little-endian uint16)
			uint16 MessageLength = 0;
			FMemory::Memcpy(&MessageLength, Data.GetData() + Offset, 2);
			Offset += 2;

			// Check we have the full message
			if (Offset + MessageLength > Data.Num())
			{
				UE_LOG(LogTemp, Warning, TEXT("[CrowdySDK] MESSAGE_BUNDLE: Message length %d exceeds remaining bytes at offset %d"),
					MessageLength, Offset);
				return MakeShared<FDefaultMessage, ESPMode::ThreadSafe>();
			}

			if (MessageLength > 0)
			{
				// Copy out this embedded message
				TArray<uint8> EmbeddedMessage;
				EmbeddedMessage.SetNumUninitialized(MessageLength);
				FMemory::Memcpy(EmbeddedMessage.GetData(), Data.GetData() + Offset, MessageLength);

				// Recursively parse the embedded message
				return ParseMessage(EmbeddedMessage);
			}

			// Zero-length message: skip it and continue
			Offset += MessageLength;
		}

		// If we got here, a bundle was empty or only contained zero-length messages
		UE_LOG(LogTemp, Warning, TEXT("[CrowdySDK] MESSAGE_BUNDLE: No valid embedded messages found."));
		return MakeShared<FDefaultMessage>();
	}
	
	
	const TArray Payload(Data.GetData() + 1, Data.Num() - 1);

	switch (MessageType)
	{
	case ECrowdyMessageType::ACTOR_UPDATE_NOTIFICATION:
		{
			TSharedRef<FActorUpdateNotificationMessage, ESPMode::ThreadSafe> Message = MakeShared<FActorUpdateNotificationMessage>();
			Message->Deserialize(Payload);
			return Message;
		}
	case ECrowdyMessageType::ACTOR_UPDATE_RESPONSE:
		{
			TSharedRef<FActorUpdateResponseMessage> Message = MakeShared<FActorUpdateResponseMessage>();
			Message->Deserialize(Payload);
			return Message;
		}
	case ECrowdyMessageType::CLIENT_EVENT_NOTIFICATION:
		{
			TSharedRef<FGameEventNotification> Message = MakeShared<FGameEventNotification>();
			Message->Deserialize(Payload);
			return Message;
		}
	case ECrowdyMessageType::VOXEL_UPDATE_NOTIFICATION:
		{
			TSharedRef<FVoxelUpdateNotificationMessage> Message = MakeShared<FVoxelUpdateNotificationMessage>();
			Message->Deserialize(Payload);
			return Message;
		}
	case ECrowdyMessageType::VOXEL_UPDATE_RESPONSE:
		{
			TSharedRef<FVoxelUpdateResponseMessage> Message = MakeShared<FVoxelUpdateResponseMessage>();
			Message->Deserialize(Payload);
			return Message;
		}
	case ECrowdyMessageType::CLIENT_AUDIO_NOTIFICATION:
		{
			TSharedRef<FClientAudioNotification> Message = MakeShared<FClientAudioNotification>();
			Message->Deserialize(Payload);
			return Message;
		}
		
	default:
		return MakeShared<FDefaultMessage>();
	}
	
}
