#pragma once

#include "CoreMinimal.h"
#include "Replication/RPC/CrowdyEvent.h"
#include "Sample/SampleSwitchBase.h"
#include "Queries/Data/Teams/Types/FCrowdyGroup.h"
#include "Queries/Data/Teams/Types/FCrowdyGroupMember.h"
#include "Queries/Data/Teams/Types/FCrowdyTeamError.h"
#include "SampleChannelSwitch.generated.h"

class UCrowdyEntityComponent;

/**
 * Announces a message to everyone on a named channel, regardless of distance. A
 * Multicast CrowdyEvent travels over the channel transport instead of the spatial
 * path, so it ignores decay and range. The switch is itself a Static entity (it is
 * placed in the level, so it uses Stable identity and resolves to the same NetID on
 * every client).
 *
 * Before the first announce it self-provisions the channel so you do not have to create
 * it by hand in Crowdy Studio: it looks the channel up by name, creates it (Open policy)
 * if missing, joins it, registers it for reliable RPC, and only then sends. After that
 * the channel is ready and each interaction announces straight away.
 *
 * The Announce signature mixes a string, an enum, an array of ints, and a class
 * reference to show that one CrowdyEvent can carry all of them directly.
 */
UCLASS()
class CROWDYSDKTEST_API ASampleChannelSwitch : public ASampleSwitchBase
{
	GENERATED_BODY()

public:

	ASampleChannelSwitch();

	UFUNCTION(meta=(CrowdyEvent, CrowdyRecipient="Multicast", CrowdyChannel="SampleWorldChat"))
	void Announce_Implementation(const FString& InMessage, uint8 InMood,
	                             const TArray<int32>& InScores, TSubclassOf<AActor> InIcon);
	CROWDY_EVENT(Announce)

protected:

	virtual void Interact_Implementation(APawn* Interactor) override;

	// Hook the visible reaction here so the same received event drives a Blueprint.
	UFUNCTION(BlueprintImplementableEvent, Category="Sample")
	void OnAnnounceReceived(const FString& InMessage, uint8 InMood,
	                        const TArray<int32>& InScores, TSubclassOf<AActor> InIcon);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sample")
	FString Message = TEXT("Hello from the channel");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sample")
	TSubclassOf<AActor> Icon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Sample")
	TObjectPtr<UCrowdyEntityComponent> CrowdyEntity;

private:

	// Kicks off the look-up / create / join chain, ending in SendAnnounce.
	void EnsureChannelThenAnnounce();

	// Registers the resolved channel for reliable RPC, marks ready, then announces.
	void RegisterAndAnnounce(int64 ChannelId);

	void SendAnnounce();

	UFUNCTION()
	void HandleChannelList(TArray<FCrowdyGroup> Channels);

	UFUNCTION()
	void HandleChannelCreated(FCrowdyGroup Channel);

	UFUNCTION()
	void HandleChannelJoined(FCrowdyGroupMember Member);

	UFUNCTION()
	void HandleChannelError(FCrowdyTeamError Error, FString InMessage);

	// Must match the CrowdyChannel meta on Announce above.
	static const FString ChannelName;

	bool bChannelReady = false;
};
