#pragma once
#include "Core/UDP/Interfaces/ICrowdyMessage.h"
#include "Core/UDP/Enums/ECrowdyMessageType.h"
#include "Shared/Types/Structures/Actors/FActorState.h"
#include "Shared/Types/Structures/Actors/FActorUpdateStruct.h"
#include "Utils/SerializationFunctionLibrary.h"

/**
 * Represents a message that notifies subscribers about updates related to an actor.
 *
 * This class is designed to encapsulate all relevant information required to
 * convey the details of an actor's state update. It can include metadata or
 * information such as updated properties, timestamps, or any other data
 * necessary for consumers to process the notification.
 */
struct FActorUpdateNotificationMessage : ICrowdyMessage
{
	

	int64 MapID;
	int64 ChunkX;
	int64 ChunkY;
	int64 ChunkZ;
	FGuid UUID;
	int32 StateSize;
	TArray<uint8> StateBytes;
	
	
	/**
	 * Retrieves the specific type of the message.
	 *
	 * This method overrides the pure virtual function defined in the ICrowdyMessage interface.
	 * It is used to identify the type of the message, which in this case is an actor update notification.
	 *
	 * @return ECrowdyMessageType::ACTOR_UPDATE_NOTIFICATION, indicating that this message type
	 * represents an actor update notification.
	 */
	virtual ECrowdyMessageType GetType() const override
	{
		return ECrowdyMessageType::ACTOR_UPDATE_NOTIFICATION;
	}

	/**
	 * Retrieves the name of the message type.
	 *
	 * This method returns an FName that represents the unique name of the message type
	 * associated with this class. It is used to identify and distinguish the message type
	 * in a human-readable format.
	 *
	 * @return An FName instance containing the string "Actor Update Notification Message".
	 */
	virtual FName GetTypeName() const override
	{
		return "Actor Update Notification Message";
	}

	/**
	 * Deserializes the given data into its corresponding object representation.
	 *
	 * This method takes a serialized data input and reconstructs the original
	 * object by parsing the data and mapping it to the appropriate fields.
	 *
	 * @param Data The serialized data to be deserialized.
	 */
	virtual void Deserialize(const TArray<uint8>& Data) override
	{
		if (Data.Num() < sizeof(int64) * 3 + 32 + sizeof(int32))
			return;
		
		int32 Offset = 0;
		
		USerializationFunctionLibrary::DeserializeValue(Data, MapID, Offset);
		Offset += sizeof(int64);
		USerializationFunctionLibrary::DeserializeValue(Data, ChunkX, Offset);
		Offset+= sizeof(int64);
		USerializationFunctionLibrary::DeserializeValue(Data, ChunkY, Offset);
		Offset+= sizeof(int64);
		USerializationFunctionLibrary::DeserializeValue(Data, ChunkZ, Offset);
		Offset+= sizeof(int64);
		UUID = USerializationFunctionLibrary::ToGuid(
			USerializationFunctionLibrary::DeserializeString(Data, Offset, 32));
		
		Offset += 32;
		USerializationFunctionLibrary::DeserializeValue(Data, StateSize, Offset);
		Offset += sizeof(StateSize);
		
		StateBytes.SetNumUninitialized(StateSize);
		FMemory::Memcpy(StateBytes.GetData(), Data.GetData() + Offset, StateSize);
	}

	/**
	 * Serializes the current object into a data format suitable for storage or transmission.
	 *
	 * This method converts the fields of the object into a serialized representation
	 * that can be deserialized later to reconstruct the original object.
	 *
	 * @return A byte array representing the serialized content of the object.
	 */
	virtual TArray<uint8> Serialize() const override
	{
		return TArray<uint8>();
	}

	/**
	 * Calculates and returns the total size of the message in bytes.
	 *
	 * This method computes the size of the message by accounting for the size
	 * of three 64-bit integers, a fixed additional offset, and the size of the
	 * serialized actor state data. The calculation ensures accurate representation
	 * of the message size for serialization or network transmission.
	 *
	 * @return The total size of the message in bytes as a 32-bit unsigned integer.
	 */
	virtual uint32 GetMessageSize() const override { return sizeof(int64) * 3 + 32;}

	
};
