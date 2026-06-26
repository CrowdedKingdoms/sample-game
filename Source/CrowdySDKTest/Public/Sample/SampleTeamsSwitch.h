#pragma once

#include "CoreMinimal.h"
#include "Sample/SampleSwitchBase.h"
#include "Queries/Data/Teams/Types/FCrowdyGroup.h"
#include "Queries/Data/Teams/Types/FCrowdyGroupMember.h"
#include "Queries/Data/Teams/Types/FCrowdyGroupMembership.h"
#include "Queries/Data/Teams/Types/FCrowdyTeamError.h"
#include "SampleTeamsSwitch.generated.h"

/**
 * Teams from gameplay code, with no manual setup. Interacting self-provisions a team by
 * name: it looks the team up, creates it (Open policy) if missing, and joins it, so the
 * demo works without anyone authoring a team in CrowdyStudio first. Interacting again
 * leaves the team, so one switch shows the whole join/leave lifecycle. Creating a team at
 * runtime is the same operation CrowdyStudio exposes in the editor.
 *
 * Membership is reported cache-first: OnMembershipChanged is driven from the cached
 * memberships for instant UI, and the OnMyTeamsCacheChanged callback is treated as the
 * eventual truth that corrects it.
 */
UCLASS()
class CROWDYSDKTEST_API ASampleTeamsSwitch : public ASampleSwitchBase
{
	GENERATED_BODY()

public:

	ASampleTeamsSwitch();

protected:

	virtual void BeginPlay() override;
	virtual void Interact_Implementation(APawn* Interactor) override;

	// The team to provision and join. Designers can point several switches at different
	// names; the first interaction creates it if it does not exist yet.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sample")
	FString TeamName = TEXT("Sample Team");

	// Designers show membership here so the same state drives C++ and Blueprint.
	UFUNCTION(BlueprintImplementableEvent, Category="Sample")
	void OnMembershipChanged(bool bInTeam);

private:

	// Look the team up by name, create it (Open) if missing, then join it.
	void EnsureTeamThenJoin();

	UFUNCTION()
	void HandleTeamList(TArray<FCrowdyGroup> Teams);

	UFUNCTION()
	void HandleTeamCreated(FCrowdyGroup Team);

	UFUNCTION()
	void HandleJoined(FCrowdyGroupMember Member);

	UFUNCTION()
	void HandleLeft();

	UFUNCTION()
	void HandleError(FCrowdyTeamError Error, FString InMessage);

	// The param is by value to match the dynamic delegate's declared signature.
	UFUNCTION()
	void HandleCacheChanged(TArray<FCrowdyGroupMembership> Memberships);

	void RefreshMembership();

	// The resolved id of the team we found or created, used for join/leave and the
	// membership readout.
	int64 ActiveTeamId = 0;
};
