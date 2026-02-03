#pragma once
#include "Core/UDP/Enums/ECrowdyMessageType.h"
#include "Core/UDP/Interfaces/ICrowdyMessage.h"
#include "Utils/SerializationFunctionLibrary.h"

/**
 * Represents a request for a specific ball event in a system,
 * which may contain details and parameters related to
 * processing or handling the event.
 */
struct FGameEventRequest : ICrowdyMessage
{
	
	/**
	 * @brief Represents the unique identifier for a map in the messaging system.
	 *
	 * This variable is used to uniquely identify a specific map within the context
	 * of a message or event. The value is typically serialized as part of the message
	 * data to ensure map-specific operations or tracking can be performed.
	 *
	 * Integral to message serialization and deserialization, the `MapID` is a 64-bit signed
	 * integer and plays a critical role in differentiating maps within the application.
	 */
	int64 MapID;
	/**
	 * @brief Represents the X-coordinate of a chunk in a 3D spatial grid.
	 *
	 * This variable is used in conjunction with ChunkY and ChunkZ to define a
	 * specific chunk's position within a three-dimensional grid. Typically, chunks
	 * are part of spatial partitioning used to organize or manage data or objects
	 * within a large virtual space.
	 *
	 * @note This is a 64-bit integer to accommodate large coordinate values,
	 * which may be required in expansive environments or systems handling high-resolution grids.
	 */
	int64 ChunkX, ChunkY, ChunkZ;
	/**
	 * A unique identifier for the activator of the event.
	 *
	 * This string represents the UUID associated with the entity or system
	 * that initiated or triggered the event. It is typically used for
	 * identifying the source of the event in a distributed system or
	 * event-handling framework. The UUID should adhere to standardized
	 * UUID format and be unique across all events.
	 */
	FString ActivatorUUID;
	/**
	 * Represents the category or classification of an event.
	 */
	uint16 EventType = 0;
	/**
	 * Represents the current status or condition of an object, system, or process.
	 */
	int32 StateSize;
	TArray<uint8> StateBytes;
	
	
	
	/**
	 * Retrieves the type of the current object or entity.
	 *
	 * @return The type of the object or entity as a string or equivalent representation.
	 */
	virtual ECrowdyMessageType GetType() const override
	{
		return ECrowdyMessageType::CLIENT_EVENT_NOTIFICATION;
	}

	/**
	 * Retrieves the name of the type.
	 *
	 * @return The name of the type as a string.
	 */
	virtual FName GetTypeName() const override
	{
		return "Ball Event Activation Request";
	}

	/**
	 * Converts an object or data structure into a format suitable for storage or transmission.
	 *
	 * @return The serialized representation of the object or data structure.
	 */
	virtual TArray<uint8> Serialize() const override
	{
		TArray<uint8> Data;
		
		Data.Add(static_cast<uint32>(GetType()) & 0xFF);
		Data.Append(USerializationFunctionLibrary::SerializeValue(MapID));
		
		Data.Append(USerializationFunctionLibrary::SerializeValue(ChunkX));
		Data.Append(USerializationFunctionLibrary::SerializeValue(ChunkY));
		Data.Append(USerializationFunctionLibrary::SerializeValue(ChunkZ));
		
		const FTCHARToUTF8 ConvertedUUID(*ActivatorUUID);
		Data.Append(reinterpret_cast<const uint8*>(ConvertedUUID.Get()), ConvertedUUID.Length());
		Data.Append(USerializationFunctionLibrary::SerializeValue(EventType));
		Data.Append(USerializationFunctionLibrary::SerializeValue(StateSize));
		Data.Append(StateBytes);
		return Data;
	}

	/**
	 * Parses the provided data and reconstructs the original object or data structure.
	 *
	 * @param data The serialized input data to be deserialized.
	 * @param format The format or schema used during the serialization process, if applicable.
	 * @return The reconstructed object or data structure.
	 */
	virtual void Deserialize(const TArray<uint8>& Data) override
	{
		return;
	}

	/**
	 * @brief Retrieves the total size of the message in bytes.
	 *
	 * This method calculates the size of the message by accounting for the sizes
	 * of its individual components. It is primarily used for message serialization
	 * and network communication purposes.
	 *
	 * Components included in the calculation:
	 * - Size of the MapID.
	 * - Size of three 64-bit integer parts: ChunkX, ChunkY, and ChunkZ.
	 * - Size of a fixed-length 32-byte string (ActivatorUUID in UTF-8 format).
	 * - Size of the EventType enum.
	 * - Size of the State object.
	 *
	 * @return The total size of the message in bytes, as a 32-bit unsigned integer.
	 */
	virtual uint32 GetMessageSize() const override
	{
		return sizeof(MapID) + sizeof(int64)*3 + 32 + sizeof(EventType) + StateSize;
	}

	
	
};
