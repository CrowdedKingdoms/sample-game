#pragma once
#include "CoreMinimal.h"
/**
 * @enum ECrowdyMessageType
 * @brief An enumeration defining various types of messages used within the Crowdy SDK.
 *
 * This enumeration is used to differentiate between different kinds of messages passed between
 * systems or components. Each message type represents a distinct purpose or semantic meaning.
 *
 * The underlying type of the enumeration is `uint8`.
 *
 * Enumerators include (but are not limited to):
 * - BAD_MESSAGE: Represents an invalid or unrecognized message.
 * - ACTOR_UPDATE_REQUEST: A request to update an actor's state.
 * - ACTOR_UPDATE_RESPONSE: A response to an actor update request.
 * - ACTOR_UPDATE_NOTIFICATION: A notification about an actor state update.
 * - VOXEL_UPDATE_REQUEST: A request to update voxel data.
 * - VOXEL_UPDATE_RESPONSE: A response to a voxel update request.
 * - VOXEL_UPDATE_NOTIFICATION: A notification about voxel data updates.
 * - CLIENT_AUDIO_PACKET: A packet representing client audio data.
 * - CLIENT_AUDIO_NOTIFICATION: A notification regarding client audio.
 * - CLIENT_TEXT_PACKET: A packet representing client text data.
 * - CLIENT_TEXT_NOTIFICATION: A notification regarding client text.
 * - CLIENT_EVENT_NOTIFICATION: A notification about a client-generated event.
 * - SERVER_EVENT_NOTIFICATION: A notification about a server-generated event.
 */
#include "Core/UDP/Enums/ECrowdyMessageType.h"

/**
 * Interface representing a generic message in the Crowdy system.
 */
class CROWDYSDK_API ICrowdyMessage
{
public:
	/**
	 * Virtual destructor for the ICrowdyMessage interface.
	 *
	 * Ensures proper cleanup of resources in derived classes that implement
	 * the ICrowdyMessage interface. As a virtual destructor, it guarantees that
	 * the destructor of the derived class is called when an instance is deleted
	 * through a pointer to the base class.
	 */
	virtual ~ICrowdyMessage() = default;
	/**
	 * Retrieves the specific type of the message.
	 *
	 * This pure virtual function must be overridden by derived classes.
	 * It is used to determine the type of the message, which is represented
	 * as an ECrowdyMessageType enumerator. The returned type can be used
	 * for message handling and processing, allowing different components or
	 * layers to identify and handle messages appropriately based on their type.
	 *
	 * @return The type of the message as an ECrowdyMessageType enumerator.
	 */
	virtual ECrowdyMessageType GetType() const = 0;
	/**
	 * Retrieves the name of the message type.
	 *
	 * This function must be implemented by all derived classes of ICrowdyMessage.
	 * It is used to return an FName identifier for the specific type of message.
	 *
	 * @return An FName instance that represents the unique name of the message type.
	 */
	virtual FName GetTypeName() const = 0;

	/**
	 * Serializes the message into a byte array.
	 *
	 * @return A TArray of uint8 containing the serialized data representation
	 * of the message.
	 *
	 * @note This method must be implemented by derived classes to define
	 * the serialization logic for specific message types.
	 */
	virtual TArray<uint8> Serialize() const = 0;
	/**
	 * Parses and initializes the object's state using the provided binary data.
	 *
	 * @param Data A reference to an array of bytes representing serialized data
	 *             that should be deserialized to reconstruct the object's state.
	 */
	virtual void Deserialize(const TArray<uint8>& Data) = 0;
	/**
	 * @brief Retrieves the size of the message in bytes.
	 *
	 * This method is a pure virtual function that must be implemented by derived classes.
	 * It provides the total size of the message, typically used for serialization or
	 * network transmission purposes.
	 *
	 * @return The size of the message in bytes as a 32-bit unsigned integer.
	 */
	virtual uint32 GetMessageSize() const = 0;
};
