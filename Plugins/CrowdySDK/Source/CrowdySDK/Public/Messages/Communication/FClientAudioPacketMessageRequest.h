#pragma once

#include "CoreMinimal.h"
#include "Core/UDP/Enums/ECrowdyMessageType.h"
#include "Core/UDP/Interfaces/ICrowdyMessage.h"

struct FClientAudioFrame
{
	TArray<uint8> AudioData;
};

struct FClientAudioPacketMessageRequest : ICrowdyMessage
{
	int64 MapID;
	FInt64Vector ChunkCoordinates;
	FString UUID;
	int32 SampleRate;
	int32 NumChannels;
	TArray<FClientAudioFrame> Frames;
	
	virtual ECrowdyMessageType GetType() const override
	{
		return ECrowdyMessageType::CLIENT_AUDIO_PACKET;
	}
	
	virtual FName GetTypeName() const override
	{
		return "Client Audio Packet Message Request";
	}
	
	virtual TArray<uint8> Serialize() const override
	{
		TArray<uint8> Data;
		
		Data.Add(static_cast<uint32>(GetType()) & 0xFF);
		
		Data.Append(reinterpret_cast<const uint8*>(&MapID), sizeof(int64));
		
		Data.Append(reinterpret_cast<const uint8*>(&ChunkCoordinates.X), sizeof(int64));
		Data.Append(reinterpret_cast<const uint8*>(&ChunkCoordinates.Y), sizeof(int64));
		Data.Append(reinterpret_cast<const uint8*>(&ChunkCoordinates.Z), sizeof(int64));
		
		const FTCHARToUTF8 UTF8String(*UUID);
		check(UTF8String.Length() == 32);
		Data.Append(reinterpret_cast<const uint8*>(UTF8String.Get()), 32);
		
		Data.Append(reinterpret_cast<const uint8*>(&SampleRate), sizeof(int32));
		Data.Append(reinterpret_cast<const uint8*>(&NumChannels), sizeof(int32));
		
		const int32 FrameCount = Frames.Num();
		Data.Append(reinterpret_cast<const uint8*>(&FrameCount), sizeof(int32));
		
		for (const FClientAudioFrame& Frame : Frames)
		{
			const int32 Lcl_FrameSize = Frame.AudioData.Num();
			Data.Append(reinterpret_cast<const uint8*>(&Lcl_FrameSize), sizeof(int32));
			Data.Append(Frame.AudioData);
		}
		
		return Data;
	}
	
	virtual void Deserialize(const TArray<uint8>& Data) override
	{
		
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
	
	int32 GetAccumulatedMessageSize()
	{
		int32 Size = 0;

		Size += sizeof(int64);           // MapID
		Size += sizeof(int64) * 3;       // ChunkCoords
		Size += 32;                      // UUID
		Size += sizeof(int32);           // SampleRate
		Size += sizeof(int32);           // NumChannels
		Size += sizeof(int32);           // FrameCount
		for (const FClientAudioFrame& Frame : Frames)
		{
			Size += sizeof(int32);        // FrameSize field
			Size += Frame.AudioData.Num();// Frame bytes
		}
		return Size;
	}
};