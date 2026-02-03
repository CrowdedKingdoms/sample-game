#pragma once
#include "Core/UDP/Enums/ECrowdyMessageType.h"
#include "Core/UDP/Interfaces/ICrowdyMessage.h"


struct FClientAudioNotificationFrame
{
	int32 FrameSize = 0;
	TArray<uint8> AudioData;
};

struct FClientAudioNotification : ICrowdyMessage
{
	int64 MapID;
	FInt64Vector ChunkCoordinates;
	FString UUID;
	
	int32 SampleRate;
	int32 NumChannels;
	
	TArray<FClientAudioNotificationFrame> Frames;
	
	virtual ECrowdyMessageType GetType() const override
	{
		return ECrowdyMessageType::CLIENT_AUDIO_NOTIFICATION;
	}
	
	virtual FName GetTypeName() const override
	{
		return "Client Audio Notification";
	}
	
	virtual TArray<uint8> Serialize() const override
	{
		return TArray<uint8>();
	}
	
	virtual void Deserialize(const TArray<uint8>& Data) override
	{
		int32 Offset = 0;

		// MapID
		if (Offset + sizeof(int64) > Data.Num()) return;
		FMemory::Memcpy(&MapID, Data.GetData() + Offset, sizeof(int64));
		Offset += sizeof(int64);

		// Chunk coords
		if (Offset + sizeof(int64) * 3 > Data.Num()) return;
		FMemory::Memcpy(&ChunkCoordinates.X, Data.GetData() + Offset, sizeof(int64));
		Offset += sizeof(int64);
		FMemory::Memcpy(&ChunkCoordinates.Y, Data.GetData() + Offset, sizeof(int64));
		Offset += sizeof(int64);
		FMemory::Memcpy(&ChunkCoordinates.Z, Data.GetData() + Offset, sizeof(int64));
		Offset += sizeof(int64);

		// UUID (32 bytes)
		if (Offset + 32 > Data.Num()) return;
		{
			const char* UUIDPtr = reinterpret_cast<const char*>(Data.GetData() + Offset);
			const FUTF8ToTCHAR UTF8Converter(UUIDPtr, 32);
			UUID = FString(UTF8Converter.Length(), UTF8Converter.Get());
			Offset += 32;
		}

		// SampleRate
		if (Offset + sizeof(int32) > Data.Num()) return;
		FMemory::Memcpy(&SampleRate, Data.GetData() + Offset, sizeof(int32));
		Offset += sizeof(int32);

		// NumChannels
		if (Offset + sizeof(int32) > Data.Num()) return;
		FMemory::Memcpy(&NumChannels, Data.GetData() + Offset, sizeof(int32));
		Offset += sizeof(int32);

		// Frame count
		if (Offset + sizeof(int32) > Data.Num()) return;
		int32 FrameCount = 0;
		FMemory::Memcpy(&FrameCount, Data.GetData() + Offset, sizeof(int32));
		Offset += sizeof(int32);

		if (FrameCount <= 0 || FrameCount > 100)
		{
			Frames.Empty();
			return;
		}

		Frames.Empty(FrameCount);

		for (int32 i = 0; i < FrameCount; ++i)
		{
			if (Offset + sizeof(int32) > Data.Num())
			{
				break;
			}

			FClientAudioNotificationFrame Frame;
			FMemory::Memcpy(&Frame.FrameSize, Data.GetData() + Offset, sizeof(int32));
			Offset += sizeof(int32);

			if (Frame.FrameSize <= 0 || Offset + Frame.FrameSize > Data.Num())
			{
				break;
			}

			Frame.AudioData.SetNumUninitialized(Frame.FrameSize);
			FMemory::Memcpy(Frame.AudioData.GetData(), Data.GetData() + Offset, Frame.FrameSize);
			Offset += Frame.FrameSize;

			Frames.Add(MoveTemp(Frame));
		}
	}
	
	virtual uint32 GetMessageSize() const override
	{
		uint32 Size = 0;

		Size += sizeof(int64);      // MapID
		Size += sizeof(int64) * 3;  // ChunkCoords
		Size += 32;                 // UUID
		Size += sizeof(int32);      // SampleRate
		Size += sizeof(int32);      // NumChannels
		Size += sizeof(int32);      // FrameCount
		
		return Size;
	}
};