#include "Sample/SampleChannelSwitch.h"

#include "Engine/GameInstance.h"
#include "Replication/Components/CrowdyEntityComponent.h"
#include "Subsystem/CrowdyChannels.h"

DEFINE_LOG_CATEGORY_STATIC(LogCrowdySampleChannel, Log, All);

// Keep this in lockstep with the CrowdyChannel meta on Announce.
const FString ASampleChannelSwitch::ChannelName = TEXT("SampleWorldChat");

ASampleChannelSwitch::ASampleChannelSwitch()
{
	SwitchLabel = FText::FromString(TEXT("CHANNEL RPC\nReliable multicast event"));
	SwitchColor = FColor(60, 120, 255);

	CrowdyEntity = CreateDefaultSubobject<UCrowdyEntityComponent>(TEXT("CrowdyEntity"));

	// The switch never streams continuous state; it only sends the Announce event.
	CrowdyEntity->Mode = ECrowdyEntityMode::Static;

	// Placed in the level, so a path-hashed NetID lines up across clients without a
	// spawn event.
	CrowdyEntity->IdentityPolicy = ECrowdyIdentityPolicy::Stable;
}

void ASampleChannelSwitch::Interact_Implementation(APawn* Interactor)
{
	// Once the channel is provisioned and registered, announcing is a single call.
	if (bChannelReady)
	{
		SendAnnounce();
		return;
	}

	EnsureChannelThenAnnounce();
}

void ASampleChannelSwitch::EnsureChannelThenAnnounce()
{
	UGameInstance* GameInstance = GetGameInstance();
	UCrowdyChannels* Channels = GameInstance ? GameInstance->GetSubsystem<UCrowdyChannels>() : nullptr;
	if (!Channels)
	{
		return;
	}

	// Step 1: list the app's channels so we can see whether ours already exists.
	FOnChannelsSuccess OnList;
	OnList.BindDynamic(this, &ASampleChannelSwitch::HandleChannelList);

	FOnChannelError OnError;
	OnError.BindDynamic(this, &ASampleChannelSwitch::HandleChannelError);

	Channels->GetChannels(OnList, OnError);
}

void ASampleChannelSwitch::HandleChannelList(TArray<FCrowdyGroup> Channels)
{
	UGameInstance* GameInstance = GetGameInstance();
	UCrowdyChannels* ChannelsSub = GameInstance ? GameInstance->GetSubsystem<UCrowdyChannels>() : nullptr;
	if (!ChannelsSub)
	{
		return;
	}

	int64 FoundId = 0;
	for (const FCrowdyGroup& Group : Channels)
	{
		if (Group.Name == ChannelName)
		{
			FoundId = Group.GroupId;
			break;
		}
	}

	FOnChannelError OnError;
	OnError.BindDynamic(this, &ASampleChannelSwitch::HandleChannelError);

	if (FoundId != 0)
	{
		// Step 2a: it exists. Join it if we are not already a member, otherwise it is
		// ready to use right now.
		if (ChannelsSub->IsPlayerInChannel(FoundId))
		{
			RegisterAndAnnounce(FoundId);
		}
		else
		{
			FOnChannelMemberSuccess OnJoined;
			OnJoined.BindDynamic(this, &ASampleChannelSwitch::HandleChannelJoined);
			ChannelsSub->JoinChannel(FoundId, OnJoined, OnError);
		}
		return;
	}

	// Step 2b: it does not exist. Create it with an Open membership policy so any player
	// can join; bMembersCanSend lets ordinary members publish. The creator is added as a
	// member as part of creation, so no separate join is needed.
	FOnChannelSuccess OnCreated;
	OnCreated.BindDynamic(this, &ASampleChannelSwitch::HandleChannelCreated);

	ChannelsSub->CreateChannel(ChannelName, TEXT("Created from the sample"),
		ECrowdyTeamMembershipPolicy::Open, /*bMembersCanSend*/ true, OnCreated, OnError);
}

void ASampleChannelSwitch::HandleChannelCreated(FCrowdyGroup Channel)
{
	UE_LOG(LogCrowdySampleChannel, Log, TEXT("Created channel \"%s\" (id %lld)."),
		*Channel.Name, Channel.GroupId);
	RegisterAndAnnounce(Channel.GroupId);
}

void ASampleChannelSwitch::HandleChannelJoined(FCrowdyGroupMember Member)
{
	RegisterAndAnnounce(Member.GroupId);
}

void ASampleChannelSwitch::RegisterAndAnnounce(int64 ChannelId)
{
	UGameInstance* GameInstance = GetGameInstance();
	UCrowdyChannels* Channels = GameInstance ? GameInstance->GetSubsystem<UCrowdyChannels>() : nullptr;
	if (!Channels)
	{
		return;
	}

	// Step 3: wire the runtime channel into the reliable RPC transport so the Multicast
	// event routes over it (the connect-time bootstrap only knows channels that already
	// existed). Then send.
	Channels->RegisterReliableRpcChannel(ChannelId, ChannelName);
	bChannelReady = true;
	SendAnnounce();
}

void ASampleChannelSwitch::SendAnnounce()
{
	// Pass the values straight in. No payload struct, no manual serialization.
	const TArray<int32> Scores = {10, 20, 30};
	Announce(Message, static_cast<uint8>(1), Scores, Icon);
}

void ASampleChannelSwitch::HandleChannelError(FCrowdyTeamError Error, FString InMessage)
{
	UE_LOG(LogCrowdySampleChannel, Warning, TEXT("Channel provisioning failed: %s"), *InMessage);
}

void ASampleChannelSwitch::Announce_Implementation(const FString& InMessage, uint8 InMood,
                                                   const TArray<int32>& InScores, TSubclassOf<AActor> InIcon)
{
	UE_LOG(LogCrowdySampleChannel, Log, TEXT("Announce: \"%s\" (mood %d, %d scores)"),
		*InMessage, static_cast<int32>(InMood), InScores.Num());

	OnAnnounceReceived(InMessage, InMood, InScores, InIcon);
}
