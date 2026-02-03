#include "Utils/SerializationFunctionLibrary.h"
#include <openssl/evp.h>
#include <openssl/sha.h>

FString USerializationFunctionLibrary::DeserializeString(const TArray<uint8>& Payload, int32 Offset, int32 Length)
{
	if (Offset + Length > Payload.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("Offset + Length exceeds Payload size"));
		return FString();
	}

	// Convert directly from the UTF-8 byte range with explicit length
	const char* Utf8Ptr = reinterpret_cast<const char*>(Payload.GetData() + Offset);
	const FUTF8ToTCHAR Converter(Utf8Ptr, Length);

	// Build an FString from the converted buffer
	return FString(Converter.Length(), Converter.Get());
}

int32 USerializationFunctionLibrary::DeserializeInt32(const TArray<uint8>& Payload, int32 Offset)
{
	int32 Value = 0;
	if (Offset + 4 <= Payload.Num())
	{
		FMemory::Memcpy(&Value, Payload.GetData() + Offset, 4);
	}
	return Value;
}

int64 USerializationFunctionLibrary::DeserializeInt64(const TArray<uint8>& Payload, int32 Offset)
{
	int64 Value = 0;
	if (Offset + sizeof(int64) <= Payload.Num())
	{
		FMemory::Memcpy(&Value, Payload.GetData() + Offset, sizeof(int64));
	}
	return Value;
}

float USerializationFunctionLibrary::DeserializeFloat(const TArray<uint8>& Payload, int32 Offset)
{
	if (Payload.Num() < Offset + sizeof(float))
	{
		UE_LOG(LogTemp, Error, TEXT("Buffer too small to deserialize float."));
		return 0.0f;
	}

	// Directly copy the bytes from the payload to the float
	float Value;
	FMemory::Memcpy(&Value, &Payload[Offset], sizeof(float));
	return Value;
}

TArray<uint8> USerializationFunctionLibrary::CalculateHMAC(const TArray<uint8>& Payload, const FString& GameToken)
{
	TArray<uint8> HMACResult;

	//Convert String to TArray uint8
	TArray<uint8> Key;
	Key.Append(reinterpret_cast<const uint8*>(TCHAR_TO_UTF8(*GameToken)), GameToken.Len());

	uint8 HMACBuffer[32] = {0};

	unsigned int OutLen = 0;
	HMAC(
		EVP_sha256(),
		Key.GetData(),
		Key.Num(),
		Payload.GetData(),
		Payload.Num(),
		HMACBuffer,
		&OutLen
	);

	HMACResult.Append(HMACBuffer, OutLen);

	return HMACResult;
}

bool USerializationFunctionLibrary::AuthenticateHMAC(const TArray<uint8>& ReceivedMessage, const FString& GameToken)
{
	return true;
	
	// if (ReceivedMessage.Num() < 37) // Minimum expected size (Payload + HVAC + Header)
	// {
	// 	UE_LOG(LogTemp, Error, TEXT("AuthenticateHVAC: Received message is too small to be valid."));
	// 	return false;
	// }

	// // HVAC is the last 32 bytes
	// constexpr int32 HMAC_Size = 32;

	// // The payload starts after the header and ends 32 bytes before the message end
	// const int32 PayloadSize = ReceivedMessage.Num() - HMAC_Size;

	// if (PayloadSize <= 0)
	// {
	// 	UE_LOG(LogTemp, Error, TEXT("AuthenticateHVAC: Invalid payload length."));
	// 	return false;
	// }

	// // Extract Payload (ignoring header and HVAC)
	// TArray<uint8> ExtractedPayload;
	// ExtractedPayload.Append(ReceivedMessage.GetData(), PayloadSize);

	// // Extract Received HVAC (last 32 bytes)
	// TArray<uint8> ReceivedHMAC;
	// ReceivedHMAC.Append(ReceivedMessage.GetData() + ReceivedMessage.Num() - HMAC_Size, HMAC_Size);

	// // Recalculate HVAC
	// const TArray<uint8> CalculatedHVAC = UFL_Serialization::CalculateHMAC(ExtractedPayload, GameToken);


	// // Compare both
	// if (ReceivedHMAC == CalculatedHVAC)
	// {
	// 	//UE_LOG(LogUDPService, Log, TEXT("AuthenticateHVAC: HVAC authentication successful."));
	// 	return true;
	// }

	// //UE_LOG(LogUDPService, Error, TEXT("AuthenticateHVAC: HVAC authentication failed."));
	// return false;
}

bool USerializationFunctionLibrary::ExtractChunkCoordinates(const TSharedPtr<FJsonObject>& JsonObj, int64& X, int64& Y,
	int64& Z)
{
	const TSharedPtr<FJsonObject>* Coordinates;

	if (!JsonObj->TryGetObjectField(TEXT("coordinates"), Coordinates))
	{
		UE_LOG(LogTemp, Warning, TEXT("Coordinates not found in updateChunk object"));
		return false;
	}

	FString sX, sY, sZ;
	if (!(*Coordinates)->TryGetStringField(TEXT("x"), sX))
	{
		UE_LOG(LogTemp, Warning, TEXT("X coord not found in coordinates object or failed to extract coord"));
		return false;
	}

	if (!(*Coordinates)->TryGetStringField(TEXT("y"), sY))
	{
		UE_LOG(LogTemp, Warning, TEXT("Y coord not found in coordinates object or failed to extract coord"));
		return false;
	}

	if (!(*Coordinates)->TryGetStringField(TEXT("z"), sZ))
	{
		UE_LOG(LogTemp, Warning, TEXT("Z coord not found in coordinates object or failed to extract coord"));
		return false;
	}

	X = FCString::Atoi64(*sX);
	Y = FCString::Atoi64(*sY);
	Z = FCString::Atoi64(*sZ);

	return true;
}

FGuid USerializationFunctionLibrary::ToGuid(const FString& String)
{
	// Direct character pointer access for speed
	const TCHAR* Data = *String;

	// Fast hex character to value conversion
	auto GetHexValue = [](const TCHAR c) -> uint8
	{
		if (c >= '0' && c <= '9') return c - '0';
		if (c >= 'A' && c <= 'F') return c - 'A' + 10;
		if (c >= 'a' && c <= 'f') return c - 'a' + 10;
		return 255; // Invalid
	};

	// Inline hex parsing function
	auto ParseHex = [&](const TCHAR* Ptr, const int32 Len) -> uint32
	{
		uint32 Result = 0;
		for (int32 i = 0; i < Len; ++i)
		{
			const uint8 Value = GetHexValue(Ptr[i]);
			if (Value == 255) return UINT32_MAX; // Invalid character
			Result = (Result << 4) | Value;
		}
		return Result;
	};

	// Parse components directly without string operations
	const uint32 A = ParseHex(Data, 8);
	const uint32 B = ParseHex(Data + 8, 8);
	const uint32 C = ParseHex(Data + 16, 8);
	const uint32 D = ParseHex(Data + 24, 8);
	
	return FGuid(A, B, C, D);
}

FString USerializationFunctionLibrary::GenerateVoxelID(int64 ChunkX, int64 ChunkY, int64 ChunkZ, int32 VoxelX,
	int32 VoxelY, int32 VoxelZ)
{
	const FString Input = FString::Printf(
		TEXT("%lld,%lld,%lld,%d,%d,%d"),
		ChunkX, ChunkY, ChunkZ,
		VoxelX, VoxelY, VoxelZ);

	// Convert to UTF-8 for hashing
	const FTCHARToUTF8 UTF8String(*Input);
	
	// Calculate SHA256 hash using OpenSSL
	unsigned char HashBytes[SHA256_DIGEST_LENGTH];
	SHA256(reinterpret_cast<const unsigned char*>(UTF8String.Get()), UTF8String.Length(), HashBytes);
	
	// Convert hash to hex string
	FString Result;
	for (int32 i = 0; i < SHA256_DIGEST_LENGTH; ++i)
	{
		Result += FString::Printf(TEXT("%02x"), HashBytes[i]);
	}
	
	return Result;
}
