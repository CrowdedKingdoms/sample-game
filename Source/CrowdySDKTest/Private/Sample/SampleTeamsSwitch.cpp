#include "Sample/SampleTeamsSwitch.h"

#include "Engine/GameInstance.h"
#include "Subsystem/CrowdyTeams.h"

DEFINE_LOG_CATEGORY_STATIC(LogCrowdySampleTeams, Log, All);

ASampleTeamsSwitch::ASampleTeamsSwitch()
{
	SwitchLabel = FText::FromString(TEXT("TEAMS\nSelf-provision, join / leave"));
	SwitchColor = FColor(220, 60, 200);
}

void ASampleTeamsSwitch::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UCrowdyTeams* Teams = GameInstance->GetSubsystem<UCrowdyTeams>())
		{
			// The cache fires whenever the player's memberships change; keep the display
			// in step with it.
			Teams->OnMyTeamsCacheChanged.AddDynamic(this, &ASampleTeamsSwitch::HandleCacheChanged);
		}
	}

	RefreshMembership();
}

void ASampleTeamsSwitch::Interact_Implementation(APawn* Interactor)
{
	UGameInstance* GameInstance = GetGameInstance();
	UCrowdyTeams* Teams = GameInstance ? GameInstance->GetSubsystem<UCrowdyTeams>() : nullptr;
	if (!Teams)
	{
		return;
	}

	// Already in the team: leave, to show the other half of the lifecycle.
	if (ActiveTeamId != 0 && Teams->IsPlayerInTeam(ActiveTeamId))
	{
		FOnTeamVoidSuccess OnLeft;
		OnLeft.BindDynamic(this, &ASampleTeamsSwitch::HandleLeft);

		FOnTeamError OnError;
		OnError.BindDynamic(this, &ASampleTeamsSwitch::HandleError);

		Teams->LeaveTeam(ActiveTeamId, OnLeft, OnError);
		return;
	}

	EnsureTeamThenJoin();
}

void ASampleTeamsSwitch::EnsureTeamThenJoin()
{
	UGameInstance* GameInstance = GetGameInstance();
	UCrowdyTeams* Teams = GameInstance ? GameInstance->GetSubsystem<UCrowdyTeams>() : nullptr;
	if (!Teams)
	{
		return;
	}

	// Step 1: list the app's teams so we can see whether ours already exists.
	FOnTeamsSuccess OnList;
	OnList.BindDynamic(this, &ASampleTeamsSwitch::HandleTeamList);

	FOnTeamError OnError;
	OnError.BindDynamic(this, &ASampleTeamsSwitch::HandleError);

	Teams->GetTeams(OnList, OnError);
}

void ASampleTeamsSwitch::HandleTeamList(TArray<FCrowdyGroup> Teams)
{
	UGameInstance* GameInstance = GetGameInstance();
	UCrowdyTeams* TeamsSub = GameInstance ? GameInstance->GetSubsystem<UCrowdyTeams>() : nullptr;
	if (!TeamsSub)
	{
		return;
	}

	int64 FoundId = 0;
	for (const FCrowdyGroup& Group : Teams)
	{
		if (Group.Name == TeamName)
		{
			FoundId = Group.GroupId;
			break;
		}
	}

	FOnTeamError OnError;
	OnError.BindDynamic(this, &ASampleTeamsSwitch::HandleError);

	if (FoundId != 0)
	{
		// Step 2a: it exists. Join it unless the cache already shows us as a member.
		ActiveTeamId = FoundId;
		if (TeamsSub->IsPlayerInTeam(FoundId))
		{
			RefreshMembership();
		}
		else
		{
			FOnTeamMemberSuccess OnJoined;
			OnJoined.BindDynamic(this, &ASampleTeamsSwitch::HandleJoined);
			TeamsSub->JoinTeam(FoundId, OnJoined, OnError);
		}
		return;
	}

	// Step 2b: it does not exist. Create it with an Open membership policy so any player
	// can join. The creator is added as a member, so no separate join is needed.
	FOnTeamSuccess OnCreated;
	OnCreated.BindDynamic(this, &ASampleTeamsSwitch::HandleTeamCreated);

	TeamsSub->CreateTeam(TeamName, TEXT("Created from the sample"),
		ECrowdyTeamMembershipPolicy::Open, OnCreated, OnError);
}

void ASampleTeamsSwitch::HandleTeamCreated(FCrowdyGroup Team)
{
	ActiveTeamId = Team.GroupId;
	UE_LOG(LogCrowdySampleTeams, Log, TEXT("Created team \"%s\" (id %lld)"), *Team.Name, Team.GroupId);
	RefreshMembership();
}

void ASampleTeamsSwitch::HandleJoined(FCrowdyGroupMember Member)
{
	ActiveTeamId = Member.GroupId;
	RefreshMembership();
}

void ASampleTeamsSwitch::HandleLeft()
{
	RefreshMembership();
}

void ASampleTeamsSwitch::HandleError(FCrowdyTeamError Error, FString InMessage)
{
	UE_LOG(LogCrowdySampleTeams, Warning, TEXT("Team request failed: %s"), *InMessage);
}

void ASampleTeamsSwitch::HandleCacheChanged(TArray<FCrowdyGroupMembership> Memberships)
{
	RefreshMembership();
}

void ASampleTeamsSwitch::RefreshMembership()
{
	bool bInTeam = false;

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UCrowdyTeams* Teams = GameInstance->GetSubsystem<UCrowdyTeams>())
		{
			// Cache-first: this answers instantly from the last known memberships.
			bInTeam = ActiveTeamId != 0 && Teams->IsPlayerInTeam(ActiveTeamId);
		}
	}

	OnMembershipChanged(bInTeam);
}
