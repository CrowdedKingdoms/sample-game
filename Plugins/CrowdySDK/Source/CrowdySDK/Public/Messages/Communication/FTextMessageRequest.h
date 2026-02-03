#pragma once
#include "Core/UDP/Enums/ECrowdyMessageType.h"
#include "Core/UDP/Interfaces/ICrowdyMessage.h"

struct FTextMessageRequest : ICrowdyMessage
{
	int64 MapID;
	int64 ChunkX;
	int64 ChunkY;
	int64 ChunkZ;
	int64 UserID;
	FString UUID;
	FString Username;
	FString Message;
	
	virtual ECrowdyMessageType GetType() const override
	{
		return ECrowdyMessageType::CLIENT_TEXT_PACKET;
	}
	
	virtual FName GetTypeName() const override
	{
		return "Text Message Request";
	}
	
	virtual TArray<uint8> Serialize() const override
	{
		TArray<uint8> Data;
		
		Data.Append(reinterpret_cast<const uint8*>(&MapID), sizeof(int64));
		Data.Append(reinterpret_cast<const uint8*>(&ChunkX), sizeof(int64));
		Data.Append(reinterpret_cast<const uint8*>(&ChunkY), sizeof(int64));
		Data.Append(reinterpret_cast<const uint8*>(&ChunkZ), sizeof(int64));

		// Convert FString to UTF-8 bytes
		const FTCHARToUTF8 UTF8String(*UUID);

		// Add the UTF-8 bytes to the payload
		const TArray UUIDBytes(reinterpret_cast<const uint8*>(UTF8String.Get()), UTF8String.Length());
		Data.Append(UUIDBytes);

	
		// Write UserID
		Data.Append(reinterpret_cast<const uint8*>(&UserID), sizeof(int64));
    
		// Write Username
		const FTCHARToUTF8 ConvertedUsername(*Username);
		const int32 UsernameLen = ConvertedUsername.Length();  // Get actual length
		Data.Append(reinterpret_cast<const uint8*>(&UsernameLen), sizeof(int32));
		Data.Append(reinterpret_cast<const uint8*>(ConvertedUsername.Get()), UsernameLen);
    
		// Write Message
		const FTCHARToUTF8 ConvertedMessage(*Message);
		const int32 MessageLen = ConvertedMessage.Length();  // Get actual length
		Data.Append(reinterpret_cast<const uint8*>(&MessageLen), sizeof(int32));
		Data.Append(reinterpret_cast<const uint8*>(ConvertedMessage.Get()), MessageLen);
		
		return Data;
	}
	
	virtual void Deserialize(const TArray<uint8>& Data) override
	{
		
	}
	
	virtual uint32 GetMessageSize() const override
	{
		return sizeof(MapID) + sizeof(int64)*3 + 32 + sizeof(int32)*2 + Message.Len();
	}
	
};