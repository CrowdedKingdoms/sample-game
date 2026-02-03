#pragma once
#include "Core/UDP/Enums/ECrowdyMessageType.h"
#include "Core/UDP/Interfaces/ICrowdyMessage.h"
#include "Utils/SerializationFunctionLibrary.h"

struct FTextMessageNotification : ICrowdyMessage
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
		return ECrowdyMessageType::CLIENT_TEXT_NOTIFICATION;
	}
	
	virtual FName GetTypeName() const override
	{
		return "Text Message Notification";
	}
	
	virtual TArray<uint8> Serialize() const override
	{
		return TArray<uint8>();
	}
	
	virtual void Deserialize(const TArray<uint8>& Data) override
	{
		int32 Offset = 0;
		USerializationFunctionLibrary::DeserializeValue(Data, MapID, Offset);
		Offset += sizeof(MapID);
		
		USerializationFunctionLibrary::DeserializeValue(Data, ChunkX, Offset);
		Offset += sizeof(ChunkX);
		USerializationFunctionLibrary::DeserializeValue(Data, ChunkY, Offset);
		Offset += sizeof(ChunkY);
		USerializationFunctionLibrary::DeserializeValue(Data, ChunkZ, Offset);
		Offset += sizeof(ChunkZ);
		
		UUID = USerializationFunctionLibrary::DeserializeString(Data, Offset, 32);
		Offset += 32;
		
		USerializationFunctionLibrary::DeserializeValue(Data, UserID, Offset);
		Offset += sizeof(UserID);
		
		if (Offset + sizeof(int32) <= Data.Num())
		{
			int32 UsernameLength;
			FMemory::Memcpy(&UsernameLength, Data.GetData() + Offset, sizeof(int32));
			Offset += sizeof(int32);
			
			if (UsernameLength > 0 && Offset + UsernameLength <= Data.Num())
			{
				TArray<ANSICHAR> UsernameBuffer;
				UsernameBuffer.SetNum(UsernameLength + 1);
				FMemory::Memcpy(UsernameBuffer.GetData(), Data.GetData() + Offset, UsernameLength);
				UsernameBuffer[UsernameLength] = '\0';
				Username = FString(UTF8_TO_TCHAR(UsernameBuffer.GetData()));
				Offset += UsernameLength;
			}
		}
		
		if (Offset + sizeof(int32) <= Data.Num())
		{
			int32 MessageLength;
			FMemory::Memcpy(&MessageLength, Data.GetData() + Offset, sizeof(int32));
			Offset += sizeof(int32);
			
			if (MessageLength > 0 && Offset + MessageLength <= Data.Num())
			{
				TArray<ANSICHAR> MessageBuffer;
				MessageBuffer.SetNum(MessageLength + 1);
				FMemory::Memcpy(MessageBuffer.GetData(), Data.GetData() + Offset, MessageLength);
				MessageBuffer[MessageLength] = '\0';
				Message = FString(UTF8_TO_TCHAR(MessageBuffer.GetData()));
			}
		}
		
	}
	
	virtual uint32 GetMessageSize() const override
	{
		return DataCopy.Num();
	}
	
private:
	TArray<uint8> DataCopy; 	
};
