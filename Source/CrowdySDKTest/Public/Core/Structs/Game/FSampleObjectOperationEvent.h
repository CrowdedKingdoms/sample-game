#pragma once
#include "CoreMinimal.h"
#include "Core/Enums/ESampleObjectOperationType.h"
#include "FSampleObjectOperationEvent.generated.h"

USTRUCT(BlueprintType)
struct FSampleObjectOperationEvent
{
	GENERATED_BODY()
	
	// To indicate the type of operation
	UPROPERTY(BlueprintReadWrite, Category="CrowdySDKTest|Object Operation")
	ESampleObjectOperationType OperationType = ESampleObjectOperationType::Destroy;
	
	// To give it a unique ID, associating it with ownership
	UPROPERTY(BlueprintReadWrite, Category="CrowdySDKTest|Object Operation")
	FGuid ObjectId;
	
	// To Map these names to object nature such as cube, box, sphere, or any other actor
	UPROPERTY(BlueprintReadWrite, Category="CrowdySDKTest|Object Operation")
	FName ObjectName;
	
	// Additional state to activate/deactivate objects
	UPROPERTY(BlueprintReadWrite, Category="CrowdySDKTest|Object Operation")
	bool bActivateObject = false;
	
	// Rotation and Location will be simply for just acting as states, we can do more complicated states, but this keeps it simple
	// As in our sample, we'll just be spawning, destroying and updating locations which will behave like teleporting
	UPROPERTY(BlueprintReadWrite, Category="CrowdySDKTest|Object Operation")
	FVector Location = FVector::ZeroVector;
	
	UPROPERTY(BlueprintReadWrite, Category="CrowdySDKTest|Object Operation")
	FRotator Rotation = FRotator::ZeroRotator;
	
	
	static TArray<uint8> Serialize(const FSampleObjectOperationEvent& ObjectOperation)
	{
		TArray<uint8> Result;
		
		// Reserve memory
		Result.Reserve(sizeof(FGuid) + sizeof(FVector) + sizeof(FRotator) + sizeof(ESampleObjectOperationType));
		
		// Add Operation Type
		Result.Add(static_cast<uint8>(ObjectOperation.OperationType));
		
		// Append ID and Name
		Result.Append(reinterpret_cast<const uint8*>(&ObjectOperation.ObjectId), sizeof(FGuid));
		Result.Append(reinterpret_cast<const uint8*>(&ObjectOperation.ObjectName), sizeof(FName));
		
		// If Operation is not Destroy, then it must contain Location and Rotation as both are required in Spawn and State Update
		if (ObjectOperation.OperationType != ESampleObjectOperationType::Destroy)
		{
			Result.Append(reinterpret_cast<const uint8*>(&ObjectOperation.bActivateObject), sizeof(bool));
			Result.Append(reinterpret_cast<const uint8*>(&ObjectOperation.Location), sizeof(FVector));
			Result.Append(reinterpret_cast<const uint8*>(&ObjectOperation.Rotation), sizeof(FRotator));
		}
		
		return Result;
	}
	
	static bool Deserialize(const TArray<uint8>& Bytes, FSampleObjectOperationEvent& ObjectOperation)
	{
		// Validation check, if state size is not satisfied, we drop it
		if (Bytes.Num() != GetSizeWithState() && Bytes.Num() != GetSizeWithoutState())
		{
			UE_LOG(LogTemp, Warning, TEXT("[FSampleObjectOperationEvent]: Received Byte Payload has a size mismatch. Deserialization failed."));
			return false;
		}
		
		int32 Offset = 0;
		
		// Extracting Operation Type
		ObjectOperation.OperationType = static_cast<ESampleObjectOperationType>(Bytes[0]);
		Offset += 1;
		
		// Extracting Object ID
		FMemory::Memcpy(&ObjectOperation.ObjectId, Bytes.GetData() + Offset, sizeof(FGuid));
		Offset += sizeof(FGuid);
		
		// Extracting Object Name
		FMemory::Memcpy(&ObjectOperation.ObjectName, Bytes.GetData() + Offset, sizeof(FName));
		Offset += sizeof(FName);
		
		// We return early if payload doesn't contain state, which is in the case if it's a destroy operation
		if (ObjectOperation.OperationType == ESampleObjectOperationType::Destroy)
			return true;
		
		// Object Activation
		FMemory::Memcpy(&ObjectOperation.bActivateObject, Bytes.GetData() + Offset, sizeof(bool));
		Offset += sizeof(bool);
		
		// If operation is not `Destroy,` then we extract state otherwise we don't
		FMemory::Memcpy(&ObjectOperation.Location, Bytes.GetData() + Offset, sizeof(FVector));
		Offset += sizeof(FVector);
		
		FMemory::Memcpy(&ObjectOperation.Rotation, Bytes.GetData() + Offset, sizeof(FRotator));
		
		return true;
	}
	
	static constexpr int32 GetSizeByType(const ESampleObjectOperationType OperationType)
	{
		switch (OperationType)
		{
			case ESampleObjectOperationType::Spawn:
			case ESampleObjectOperationType::UpdateState:
			return GetSizeWithState();
			
			case ESampleObjectOperationType::Destroy:
			return GetSizeWithoutState();
			
			default:
			return 0;
		}
	}
	
private:
	
	static constexpr int32 GetSizeWithState()
	{
		return sizeof(FGuid) + sizeof(FName) + sizeof(FVector) + sizeof(FRotator) + sizeof(ESampleObjectOperationType)
		+ sizeof(bool);
	}
	
	static constexpr int32 GetSizeWithoutState()
	{
		return sizeof(FGuid) + sizeof(ESampleObjectOperationType) + sizeof(FName);
	}
	
};