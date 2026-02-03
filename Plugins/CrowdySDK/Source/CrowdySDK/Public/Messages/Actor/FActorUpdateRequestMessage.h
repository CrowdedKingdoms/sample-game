#pragma once
#include "Core/UDP/Interfaces/ICrowdyMessage.h"
#include "Core/UDP/Enums/ECrowdyMessageType.h"
#include "Shared/Types/Structures/Actors/FActorState.h"
#include "Utils/SerializationFunctionLibrary.h"

/**
 * @class FActorUpdateRequestMessage
 * @brief Represents a message containing a request to update an actor within the system.
 *
 * This class encapsulates the data required to request an update to a specific actor,
 * such as its unique identifier, updated properties, or state information.
 *
 * It is typically used in messaging systems where updates to actors need to be communicated
 * between different parts of the application or across network boundaries.
 *
 * Responsibilities of this class:
 * - Encapsulate information about the actor update request.
 * - Provide a structured way to relay actor update details.
 *
 * Use this class in conjunction with systems that require actor state synchronization
 * or have mechanisms that handle update messages to apply changes to actors.
 */
struct FActorUpdateRequestMessage : ICrowdyMessage
{
	
	/**
	 * A unique identifier representing the current map in the game environment.
	 *
	 * This value is typically used to distinguish between different maps or levels
	 * within a game session. It is set dynamically and used in operations such as
	 * serialization, deserialization, and communication with external systems like
	 * the CrowdySDK.
	 *
	 * MapID is integral in ensuring that updates or requests are correctly associated
	 * with the corresponding map context in distributed or multiplayer scenarios.
	 */
	int64 MapID;
	/**
	 * Represents the X-coordinate of a chunk in a 3D grid-based system.
	 * Used in conjunction with ChunkY and ChunkZ to define the location
	 * of a chunk within the grid. Commonly used in actor update
	 * requests or related serialization and deserialization for network
	 * message handling.
	 */
	int64 ChunkX, ChunkY, ChunkZ;
	
	/**
	 * Holds the unique identifier for an actor.
	 *
	 * This variable represents the UUID (Universally Unique Identifier) assigned to an actor.
	 * It is used to uniquely identify an actor across various processes and systems.
	 * The UUID is serialized and transmitted as part of actor-related network messages,
	 * ensuring consistent identification regardless of location or context.
	 */
	FString ActorUUID;
	
	/**
	 * Represents the state of an actor in the system.
	 *
	 * This variable is an instance of the FActorState structure, which contains
	 * key information about the actor, such as its position, version, and other
	 * properties. The state is used to represent and update the actor's current
	 * attributes within the game or simulation environment.
	 */
	int32 StateSize;
	TArray<uint8> StateBytes;
	
	
	/**
	 * Retrieves the type of the message.
	 *
	 * @return The message type as ECrowdyMessageType::ACTOR_UPDATE_REQUEST.
	 */
	virtual ECrowdyMessageType GetType() const override
	{
		return ECrowdyMessageType::ACTOR_UPDATE_REQUEST;
	}

	/**
	 * Retrieves the name of the message type.
	 *
	 * This method overrides the base class implementation to provide a unique name
	 * for the type of this message. It is used for identifying the message by its type name.
	 *
	 * @return An FName instance representing the name "Actor Update Request Message".
	 */
	virtual FName GetTypeName() const override
	{
		return "Actor Update Request Message";
	}

	/**
	 * Serializes the FActorUpdateRequestMessage into a byte array.
	 *
	 * This method converts the internal state of the FActorUpdateRequestMessage
	 * object into a binary format suitable for transmission or storage. The serialization
	 * process includes the message type, map ID, chunk coordinates, actor UUID, and actor state.
	 *
	 * @return A TArray of uint8 containing the serialized representation of the message.
	 */
	virtual TArray<uint8> Serialize() const override
	{
		TArray<uint8> Data;
		Data.Reserve(sizeof(int64) * 4 + 32 + 88 + 1);
		
		Data.Add(static_cast<uint32>(GetType()) & 0xFF);
		Data.Append(USerializationFunctionLibrary::SerializeValue(MapID));
		Data.Append(USerializationFunctionLibrary::SerializeValue(ChunkX));
		Data.Append(USerializationFunctionLibrary::SerializeValue(ChunkY));
		Data.Append(USerializationFunctionLibrary::SerializeValue(ChunkZ));

		const FTCHARToUTF8 ConvertedUUID(*ActorUUID);
		Data.Append(reinterpret_cast<const uint8*>(ConvertedUUID.Get()), ConvertedUUID.Length());
		Data.Append(USerializationFunctionLibrary::SerializeValue(StateSize));
		Data.Append(StateBytes);

		return Data;
	}

	/**
	 * Deserializes the provided binary data into the object's properties.
	 * This method reads data from the given byte array and updates the internal state
	 * of the object. It is intended for use in editor mode only and logs a warning if
	 * called during runtime.
	 *
	 * @param Data The binary array containing the serialized data to be deserialized.
	 */
	virtual void Deserialize(const TArray<uint8>& Data) override
	{
		UE_LOG(LogTemp, Warning, TEXT("FActorUpdateRequestMessage::Deserialize called, but should not be used."));

#if WITH_EDITOR
		
		int32 Offset = 0;
		int64 MapID_Lcl;
		int64 ChunkX_Lcl, ChunkY_Lcl, ChunkZ_Lcl;

		USerializationFunctionLibrary::DeserializeValue(Data, MapID_Lcl, Offset);
		Offset += sizeof(MapID);
		USerializationFunctionLibrary::DeserializeValue(Data, ChunkX_Lcl, Offset);
		Offset += sizeof(ChunkX);
		USerializationFunctionLibrary::DeserializeValue(Data, ChunkY_Lcl, Offset);
		Offset += sizeof(ChunkY);
		USerializationFunctionLibrary::DeserializeValue(Data, ChunkZ_Lcl, Offset);
		Offset += sizeof(ChunkZ);
		FString ActorUUID_Lcl = USerializationFunctionLibrary::DeserializeString(Data, Offset, 32);
		Offset += 32;
		FActorState State_Lcl = FActorState().DeserializeActorState(Data, Offset);

		UE_LOG(LogTemp, Log, TEXT("MapID: %lld\nChunk %lld %lld %lld\n"), MapID_Lcl, ChunkX_Lcl, ChunkY_Lcl, ChunkZ_Lcl);
#endif
	}

	/**
	 * Calculates and retrieves the total size of the message in bytes.
	 *
	 * The size is computed as the sum of the following components:
	 * - The size of the message type indicator (1 byte).
	 * - The size of four 64-bit integers (MapID and Chunk coordinates).
	 * - The size of the UUID for the actor (32 bytes).
	 * - The size of the actor state (FActorState structure).
	 *
	 * This method is primarily used to determine the exact amount of memory
	 * required for serialization or transmission of the message.
	 *
	 * @return The total size of the message in bytes as a 32-bit unsigned integer.
	 */
	virtual uint32 GetMessageSize() const override
	{
		// Message Type + 4 int64 + UUID + Actor State
		return 1 + sizeof(int64) * 4 + 32 + sizeof(FActorState);
	}

	
};
