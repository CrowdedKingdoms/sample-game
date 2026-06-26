#include "Sample/SamplePersistenceSwitch.h"

#include "Engine/GameInstance.h"
#include "GameFramework/Pawn.h"
#include "Sample/SampleTypes.h"
#include "Subsystem/CrowdyPersistenceSubsystem.h"

ASamplePersistenceSwitch::ASamplePersistenceSwitch()
{
	SwitchLabel = FText::FromString(TEXT("PERSISTENCE\nSave then load your spot"));
	SwitchColor = FColor(235, 205, 40);
}

void ASamplePersistenceSwitch::Interact_Implementation(APawn* Interactor)
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance || !Interactor)
	{
		return;
	}

	UCrowdyPersistenceSubsystem* Persistence = GameInstance->GetSubsystem<UCrowdyPersistenceSubsystem>();
	if (!Persistence)
	{
		return;
	}

	if (Step % 2 == 0)
	{
		// Save. FSampleProgress is tagged CrowdyPersistent, so the subsystem already
		// knows how to store it. Subject = the player, so each player gets its own slot.
		FSampleProgress Progress;
		Progress.Level = ++SavedLevel;
		Progress.LastLocation = Interactor->GetActorLocation();
		Persistence->PushState(Progress, Interactor);
	}
	else
	{
		// Load. The pull is asynchronous; apply the result on the game thread when it
		// arrives. Capture a weak pointer so a destroyed pawn does not crash the callback.
		TWeakObjectPtr<APawn> WeakPawn = Interactor;
		Persistence->PullState<FSampleProgress>(Interactor,
			[WeakPawn](bool bSuccess, const FSampleProgress& Loaded)
			{
				if (bSuccess && WeakPawn.IsValid())
				{
					WeakPawn->SetActorLocation(Loaded.LastLocation);
				}
			});
	}

	++Step;
}
