#pragma once
#include "Core/UDP/Enums/ECrowdyMessageType.h"
#include "Core/UDP/Interfaces/ICrowdyMessage.h"
#include "Shared/Types/Structures/Events/FBaseEventState.h"
#include "Utils/SerializationFunctionLibrary.h"

/**
 * @class FGameEventNotification
 * @brief A class designed to handle event-based notifications related to ball tracking or game states.
 *
 * This class provides mechanisms to manage and deliver notifications triggered by specific events
 * occurring within the gameplay or tracking systems. It encapsulates the necessary data and functionality
 * to support various event-handling scenarios in sports or simulation environments.
 *
 * The class is intended to be flexible, supporting different notification types, event listeners,
 * and data broadcasting to registered components or systems.
 *
 * Responsibilities of the FBallEventNotification class include:
 * - Storing event-specific details.
 * - Notifying registered observers or systems about the occurrence of events.
 * - Providing a structured way to handle ball state-related activities.
 *
 * Typical use cases:
 * - Updating game states when specific ball movements are detected.
 * - Triggering UI or audio feedback for in-game events.
 * - Integration with analytics or telemetry systems to log events.
 *
 * This class is designed as part of a larger system and may require interaction with other components
 * for full functionality.
 */
struct FGameEventNotification : ICrowdyMessage
{
	
	/**
	 * @brief Represents the identifier for a specific map instance.
	 *
	 * This variable is used to uniquely identify a map in a system
	 * that manages multiple maps. It serves as an essential key
	 * for retrieving, updating, or performing operations on the corresponding map data.
	 *
	 * It is crucial in scenarios where multiple maps coexist and
	 * maintaining distinction between them is necessary.
	 */
	int64 MapID;
	/**
	 * @brief Represents the X-coordinate of a chunk in a grid or partitioned space.
	 *
	 * This variable is typically used to identify or manipulate the horizontal
	 * position of a chunk within a larger system, such as a game world or a
	 * divided data structure. Each chunk represents a discrete segment, and
	 * ChunkX determines its location along the X-axis.
	 *
	 * Usage scenarios include spatial computations, rendering systems, and
	 * data organization where chunks are divided in a grid-like manner.
	 */
	int64 ChunkX, ChunkY, ChunkZ;
	/**
	 * @brief A universally unique identifier (UUID).
	 *
	 * This variable is used to represent a 128-bit unique identifier.
	 * It is commonly used in software systems to uniquely identify
	 * objects, entities, or records across distributed systems or
	 * within a single application.
	 *
	 * The UUID ensures a high probability of being unique, even when
	 * generated simultaneously by different processes or systems.
	 *
	 * Note: Adheres to the specifications defined in RFC 4122.
	 */
	FString UUID;
	/**
	 * @brief Represents the type of an event in the system.
	 *
	 * This variable is used to categorize or distinguish different kinds of events
	 * that occur within the application. The specific values and their meanings
	 * depend on the context in which the variable is used.
	 *
	 * Potential usage scenarios include:
	 * - Identifying user actions or interactions.
	 * - Differentiating between system-generated events.
	 * - Managing custom application-defined events.
	 *
	 * Proper assignment and usage of this variable ensure accurate event handling
	 * and processing within the system.
	 */
	uint16 EventType;
	/**
	 * @brief Represents the state or condition of an entity or process.
	 *
	 * The State variable is used to describe and manage the current mode, status, or condition.
	 * It is typically part of a broader system and can take on different values depending
	 * on the context where it is applied. The exact behavior or usage is determined by its
	 * implementation or the system's requirements.
	 */
	int32 StateSize;
	TArray<uint8> StateBytes;
	
	
	/**
	 * Retrieves the type of the current object or instance.
	 *
	 * This method is typically used to determine the runtime type of an object
	 * and can be useful for type-checking or implementing type-based logic.
	 *
	 * @return The type of the current object or instance.
	 */
	virtual ECrowdyMessageType GetType() const override
	{
		return ECrowdyMessageType::CLIENT_EVENT_NOTIFICATION;
	}

	/**
	 * Retrieves the name of the message type.
	 *
	 * This method returns an FName instance representing the unique identifier
	 * for the type of message. It is used to distinguish and identify the
	 * specific message type in the system.
	 *
	 * @return An FName instance containing the name of the message type as "Ball Event Notification".
	 */
	virtual FName GetTypeName() const override
	{
		return "Ball Event Notification";
	}

	/**
	 * Serializes the given object into a format suitable for storage or transmission.
	 *
	 * This method converts the object into a specific serialized format, such as JSON, XML,
	 * or binary data, depending on the implementation. The resulting serialized data
	 * can then be saved to a file, sent over a network, or used for other purposes.
	 *
	 * @return A string or byte array representing the serialized form of the object.
	 */
	virtual TArray<uint8> Serialize() const override
	{
		return TArray<uint8>();
	}

	/**
	 * Deserializes a given input into an object of the specified type.
	 *
	 * This method converts serialized data, such as JSON or XML, back
	 * into the original object representation. It ensures the integrity
	 * of the deserialized data and throws exceptions if the process fails.
	 *
	 * @param input The serialized data to be deserialized.
	 * @param type The target class type to deserialize the input into.
	 * @throws IOException If there is an issue reading the input.
	 * @throws IllegalArgumentException If the input or type is invalid.
	 * @throws DeserializationException If the deserialization process fails.
	 */
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
		
		USerializationFunctionLibrary::DeserializeValue(Data, EventType, Offset);
		Offset += sizeof(EventType);
		
		USerializationFunctionLibrary::DeserializeValue(Data, StateSize, Offset);
		Offset += sizeof(StateSize);
		
		StateBytes.SetNumUninitialized(StateSize);
		FMemory::Memcpy(StateBytes.GetData(), Data.GetData() + Offset, StateSize);
	}

	/**
	 * Calculates the size of the message in bytes.
	 *
	 * This method overrides the base class implementation to return the size of
	 * the message, calculated as the sum of the sizes of its member variables.
	 * The returned size includes the sizes of the MapID, Chunk coordinates,
	 * UUID (fixed size of 32 bytes), EventType, and State variables.
	 *
	 * @return The total size of the message in bytes as a 32-bit unsigned integer.
	 */
	virtual uint32 GetMessageSize() const override
	{
		return sizeof(MapID) + sizeof(int64)*3 + 32 + sizeof(EventType) + StateSize;
	}

	
};
