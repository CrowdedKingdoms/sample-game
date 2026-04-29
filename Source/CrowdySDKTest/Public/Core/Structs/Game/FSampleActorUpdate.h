#pragma once
#include "CoreMinimal.h"
#include "FSampleActorUpdate.generated.h"


USTRUCT(BlueprintType)
struct FSampleActorUpdate
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, Category="CrowdySDKTest|Actor Update")
	FVector Location = FVector::ZeroVector;
	
	UPROPERTY(BlueprintReadWrite, Category="CrowdySDKTest|Actor Update")
	FRotator Rotation = FRotator::ZeroRotator;
	
	static TArray<uint8> Serialize(const FSampleActorUpdate& ActorUpdate)
	{
		TArray<uint8> Result;
		Result.Reserve(sizeof(FVector) + sizeof(FRotator));
		
		Result.Append(reinterpret_cast<const uint8*>(&ActorUpdate.Location), sizeof(FVector));
		Result.Append(reinterpret_cast<const uint8*>(&ActorUpdate.Rotation), sizeof(FRotator));
		
		return Result;
	}
	
	static bool Deserialize(const TArray<uint8>& Bytes, FSampleActorUpdate& ActorUpdate)
	{
		FSampleActorUpdate Result;
		
		if (Bytes.Num() != GetStateSize())
		{
			UE_LOG(LogTemp, Warning, TEXT("[FSampleActorUpdate]: Received Byte Payload has a size mismatch. Deserialization failed."));
			return false;
		}
		
		int32 Offset = 0;
		
		FMemory::Memcpy(&Result.Location, Bytes.GetData(), sizeof(FVector));
		Offset += sizeof(FVector);
		
		FMemory::Memcpy(&Result.Rotation, Bytes.GetData() + Offset, sizeof(FRotator));
		
		return true;
	}
	
	FORCEINLINE static constexpr int32 GetStateSize()
	{
		return sizeof(FVector) + sizeof(FRotator);
	}
};