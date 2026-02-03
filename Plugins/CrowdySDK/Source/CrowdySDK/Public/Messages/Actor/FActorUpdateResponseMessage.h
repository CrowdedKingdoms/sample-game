#pragma once
#include "Core/UDP/Interfaces/ICrowdyMessage.h"
#include "Core/UDP/Enums/ECrowdyMessageType.h"
#include "Utils/SerializationFunctionLibrary.h"

/**
 * Encapsulates a response message for updating an actor.
 * This class is used to handle and process the results of an actor update operation.
 */
struct FActorUpdateResponseMessage : ICrowdyMessage
{
	
	/**
	 * @brief Represents the unique identifier for a map in the system.
	 *
	 * The MapID variable is a 64-bit integer used to distinguish between different
	 * maps or terrains in the context of actor updates and message serialization.
	 * It is initialized and deserialized as part of the FActorUpdateResponseMessage
	 * structure, allowing for precise mapping and processing of actor-related data
	 * in different map regions.
	 *
	 * This variable is critical for identifying the map associated with a specific
	 * actor update event, ensuring correct synchronization and data integrity.
	 */
	int64 MapID;
	/**
	 * Represents the X-coordinate of a chunk in a 3D grid or spatial partitioning system.
	 *
	 * This variable is typically used in the context of map or world representation,
	 * where chunks are subdivisions of the larger environment. It holds a 64-bit integer
	 * value that specifies the position of the chunk along the X-axis.
	 */
	int64 ChunkX, ChunkY, ChunkZ;
	/**
	 * Represents the universally unique identifier (UUID) of an actor.
	 *
	 * This variable is used to uniquely identify an actor instance for purposes
	 * such as serialization, deserialization, or tracking during message handling.
	 *
	 * In the context of message communication, `ActorUUID` is deserialized from
	 * incoming data to provide a reference to a specific actor associated with the
	 * given message. It may also be used for debugging purposes to log or trace
	 * actor-related operations.
	 */
	FString ActorUUID;
	/**
	 * Represents the error code associated with the processing or validation of an
	 * actor update response message within the Crowdy framework. The value is of
	 * type ECrowdyErrorCode and indicates the specific error encountered during the
	 * operation.
	 *
	 * Possible values:
	 * - SUCCESS: Indicates that the operation completed successfully.
	 * - UNKNOWN_ERROR: An unspecified error occurred.
	 * - EMAIL_NOT_FOUND: Email address could not be found.
	 * - BAD_PASSWORD: Provided password is invalid or incorrect.
	 * - EMAIL_ALREADY_EXISTS: The email address is already in use.
	 * - INVALID_TOKEN: Authentication or request token is invalid.
	 * - MAP_NOT_FOUND: The specified map could not be found.
	 * - UNAUTHORIZED: The operation is not authorized for the user.
	 * - MAP_NOT_LOADED: The required map is not loaded.
	 * - EMAIL_TOO_SHORT: The email address provided is shorter than allowed.
	 * - EMAIL_TOO_LONG: The email address provided exceeds the maximum length.
	 * - PASSWORD_TOO_SHORT: The password provided is shorter than the minimum length.
	 * - PASSWORD_TOO_LONG: The password provided exceeds the maximum length.
	 * - GAME_TOKEN_WRONG_SIZE: The size of the game token is incorrect.
	 * - NAME_TOO_LONG: The provided name exceeds the allowed length.
	 * - INVALID_REQUEST: The request is malformed or invalid.
	 * - EMAIL_INVALID: The email address has an invalid format.
	 * - INVALID_TOKEN_LENGTH: The length of the token is not valid.
	 * - INVALID_MAP_ID: Specified Map ID is invalid.
	 * - CHUNK_NOT_FOUND: Unable to locate the specified chunk.
	 * - USER_NOT_AUTHENTICATED: The user is not authenticated.
	 */
	ECrowdyErrorCode ErrorCode;
	
	/**
	 * Retrieves the specific type of the message.
	 *
	 * This method overrides the base class implementation to return the specific message
	 * type associated with the FActorUpdateResponseMessage. This type is used to identify
	 * the message as an actor update response in the ECrowdyMessageType enumeration.
	 *
	 * @return ECrowdyMessageType::ACTOR_UPDATE_RESPONSE The type of the message as
	 * an enumerator from ECrowdyMessageType.
	 */
	virtual ECrowdyMessageType GetType() const override
	{
		return ECrowdyMessageType::ACTOR_UPDATE_RESPONSE;
	}

	/**
	 * Retrieves the name of the message type.
	 *
	 * This method overrides the base class implementation to return a unique
	 * identifier for the message type as an FName. The returned name is
	 * specific to the "Actor Update Response Message" type.
	 *
	 * @return An FName instance representing the name of the message type.
	 */
	virtual FName GetTypeName() const override
	{
		return "Actor Update Response Message";
	}

	/**
	 * Deserializes the provided data into its original structure or object format.
	 *
	 * @param data The serialized data to be deserialized.
	 * @param format The format in which the data is serialized, such as JSON or XML.
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
		
		ActorUUID = USerializationFunctionLibrary::DeserializeString(Data, Offset, 32);
		Offset += 32;
		
		ErrorCode = static_cast<ECrowdyErrorCode>(Data[Offset]);
		
		UE_LOG(LogTemp, Error, TEXT("Actor Update Response: UUID %s for Chunk %lld %lld %lld with error %d"),
		       *ActorUUID, ChunkX, ChunkY, ChunkZ, ErrorCode);
	}

	/**
	 * Converts an object or data structure into a format suitable for storage or transmission.
	 * The serialized output can later be deserialized to reconstruct the original object.
	 *
	 * @return A serialized representation of the object or data structure.
	 */
	virtual TArray<uint8> Serialize() const override
	{
		return TArray<uint8>();
	}

	/**
	 * Calculates and returns the size of the message in bytes.
	 *
	 * This method provides the total size of the message, which is determined
	 * by summing the sizes of its individual components. The size includes the
	 * memory required for storing map information, chunk coordinates, actor
	 * identifier, and additional metadata. It can be used to determine the
	 * message's footprint during serialization or transmission processes.
	 *
	 * @return The size of the message in bytes as a 32-bit unsigned integer.
	 */
	virtual uint32 GetMessageSize() const override { return sizeof(int64) * 4 + 32 + 1; }

	
};
