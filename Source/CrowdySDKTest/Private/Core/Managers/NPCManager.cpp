// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Managers/NPCManager.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Core/Structs/FActorData.h"
#include "Core/UDP/Interfaces/ICrowdyMessage.h"
#include "Messages/Actor/FActorUpdateNotificationMessage.h"
#include "Messages/Actor/FActorUpdateRequestMessage.h"
#include "Messages/Actor/FActorUpdateResponseMessage.h"
#include "Subsystem/CrowdyGameSession.h"
#include "Subsystem/CrowdySDKSubsystem.h"
#include "Utils/HelperFunctions.h"


// Sets default values
ANPCManager::ANPCManager()
{
	PrimaryActorTick.bCanEverTick = true;
	
	PrimaryActorTick.TickInterval = 1.0f/30.0f;
	
	ISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("NpcInstanceMesh"));
	SetRootComponent(ISM);
	ISM->SetMobility(EComponentMobility::Movable);
	ISM->bDisableCollision = true;
	ISM->NumCustomDataFloats = 0;
}

// Called when the game starts or when spawned
void ANPCManager::BeginPlay()
{
	Super::BeginPlay();
	
	SDK = GetWorld()->GetGameInstance()->GetSubsystem<UCrowdySDKSubsystem>();
	SDK->RegisterReceptionLayer(this);
	ISM->SetStaticMesh(MeshToSpawn);
}

void ANPCManager::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!ISM) return;

	const float Dt = FMath::Min(DeltaTime, 0.05f);

	// ---- tuned for 10Hz, 600cm/s ----
	const float PosSmoothTime = 0.08f;
	const float PredictTime   = 0.12f;
	const float TeleportDist  = 220.0f;
	const float RotInterpSpeed = 24.0f;
	const float MaxSmoothSpeed = 1000.0f;

	FActorData ActorData;

	// 1) Dequeue network updates: set targets only
	while (ActorUpdateQueue.Dequeue(ActorData))
	{
		const float Now = GetWorld()->GetTimeSeconds();
		LastUpdateTimeById.Add(ActorData.ActorID, Now);
		
		const FVector Location = ActorData.Location - FVector(0.0f, 0.0f, 89.0f);
		const FTransform Target(ActorData.Rotation, Location);
		
		if (!SpawnedActors.Contains(ActorData.ActorID))
		{
			const int32 Index = ISM->AddInstance(Target);
			SpawnedActors.Add(ActorData.ActorID, Index);

			TargetById.Add(ActorData.ActorID, Target);
			CurrentById.Add(ActorData.ActorID, Target);

			VelocityById.Add(ActorData.ActorID, ActorData.Velocity);
			CurrentVelById.Add(ActorData.ActorID, FVector::ZeroVector);
			
			IdByInstanceIndex.Add(Index, ActorData.ActorID);
			continue;
		}

		TargetById.FindOrAdd(ActorData.ActorID) = Target;
		VelocityById.FindOrAdd(ActorData.ActorID) = ActorData.Velocity;

		CurrentById.FindOrAdd(ActorData.ActorID);
		CurrentVelById.FindOrAdd(ActorData.ActorID);
		LastUpdateTimeById.FindOrAdd(ActorData.ActorID) = Now;
	}

	// 2) Smooth all instances
	for (const auto& Pair : SpawnedActors)
	{
		const FGuid& Id = Pair.Key;
		const int32 Index = Pair.Value;

		FTransform* TargetPtr = TargetById.Find(Id);
		FTransform* CurrentPtr = CurrentById.Find(Id);
		FVector* SmoothVelPtr = CurrentVelById.Find(Id);

		if (!TargetPtr || !CurrentPtr || !SmoothVelPtr)
			continue;

		const FVector TargetPos = TargetPtr->GetLocation();
		const FQuat   TargetQ   = TargetPtr->GetRotation();

		// Predict ~1 tick ahead (hides 10Hz stepping)
		FVector PredPos = TargetPos;
		if (const FVector* Vel = VelocityById.Find(Id))
		{
			PredPos = TargetPos + (*Vel * PredictTime);
		}

		const FVector CurPos = CurrentPtr->GetLocation();

		// Snap on large correction to avoid "drag trails"
		if (FVector::DistSquared(CurPos, PredPos) > FMath::Square(TeleportDist))
		{
			CurrentPtr->SetLocation(PredPos);
			CurrentPtr->SetRotation(TargetQ);
			*SmoothVelPtr = FVector::ZeroVector;
		}
		else
		{
			// Position: critically damped (smooth + low-latency)
			const FVector NewPos = SmoothDampVector(
				CurPos, PredPos, *SmoothVelPtr, PosSmoothTime, Dt, MaxSmoothSpeed
			);

			// Rotation: keep responsive
			const FQuat CurQ = CurrentPtr->GetRotation();
			const FQuat NewQ = FMath::QInterpTo(CurQ, TargetQ, Dt, RotInterpSpeed).GetNormalized();

			CurrentPtr->SetLocation(NewPos);
			CurrentPtr->SetRotation(NewQ);
		}

		// WorldSpace=true, defer dirty update for performance
		ISM->UpdateInstanceTransform(Index, *CurrentPtr, true, false, true);
	}

	ISM->MarkRenderStateDirty();
	
	const float Now = GetWorld()->GetTimeSeconds();
	TArray<FGuid> ToRemove;
	ToRemove.Reserve(16);
	
	for (const auto& Pair : LastUpdateTimeById)
	{
		const FGuid& ActorId = Pair.Key;
		const float LastSeen = Pair.Value;

		if ((Now - LastSeen) > ActorTimeoutThreshold)
		{
			ToRemove.Add(ActorId);
		}
	}

	for (const FGuid& ActorId : ToRemove)
	{
		RemoveActorInstance(ActorId);
	}

	// If you removed anything, render needs update
	if (ToRemove.Num() > 0)
	{
		ISM->MarkRenderStateDirty();
	}
}

void ANPCManager::OnMessageReceived(TSharedRef<ICrowdyMessage> Message)
{
	switch (Message->GetType())
	{
		case ECrowdyMessageType::ACTOR_UPDATE_NOTIFICATION:
			{
				//UE_LOG(LogTemp, Log, TEXT("Received Actor Update Notification"));
				FActorUpdateNotificationMessage ActorUpdateNotify = static_cast<FActorUpdateNotificationMessage&>(*Message);
				EnqueueActorUpdate(ActorUpdateNotify);
				break;
			}
		case ECrowdyMessageType::ACTOR_UPDATE_RESPONSE:
			{
				const FActorUpdateResponseMessage ActorUpdateResponse = static_cast<FActorUpdateResponseMessage&>(*Message);
				UE_LOG(LogTemp, Warning, TEXT("\nReceived actor update response for UUID %s in chunk(%lld %lld %lld) with error code %d"), 
					*ActorUpdateResponse.ActorUUID, 
					ActorUpdateResponse.ChunkX, ActorUpdateResponse.ChunkY, ActorUpdateResponse.ChunkZ, 
					ActorUpdateResponse.ErrorCode);
				break;
			}
		default:
		UE_LOG(LogTemp, Warning, TEXT("Received unknown message type %s"), *Message->GetTypeName().ToString());
		break;
	}
}

TArray<ECrowdyMessageType> ANPCManager::GetSupportedResponseTypes() const
{
	return {
		ECrowdyMessageType::ACTOR_UPDATE_NOTIFICATION,
		ECrowdyMessageType::ACTOR_UPDATE_RESPONSE
	};
}

void ANPCManager::SendActorUpdate(const int64 ChunkX, const int64 ChunkY, const int64 ChunkZ, const FString UUID, const FActorData& ActorData) const
{
	// Create an Actor Update Request Message and set params
	FActorUpdateRequestMessage ActorUpdateRequest;
	ActorUpdateRequest.MapID = GetWorld()->GetGameInstance()->GetSubsystem<UCrowdyGameSession>()->GetMapID();
	ActorUpdateRequest.ActorUUID = UUID;
	ActorUpdateRequest.ChunkX = ChunkX;
	ActorUpdateRequest.ChunkY = ChunkY;
	ActorUpdateRequest.ChunkZ = ChunkZ;
	
	// Assign State Size 
	ActorUpdateRequest.StateSize = sizeof(ActorData);
	
	// Create State Bytes
	TArray<uint8> StateBytes;
	StateBytes.SetNumUninitialized(sizeof(ActorData));
	
	// Copy Actor Data into State Bytes
	FMemory::Memcpy(StateBytes.GetData(), &ActorData, sizeof(ActorData));
	
	// Assign State Bytes in the Message
	ActorUpdateRequest.StateBytes = StateBytes;
	
	// Dispatch Message using SDK
	SDK->SendMessage(ActorUpdateRequest);
}

void ANPCManager::EnqueueActorUpdate(FActorUpdateNotificationMessage& ActorUpdateNotify)
{
	// Ignore Owner Updates 
	if (OwnerUUID == ActorUpdateNotify.UUID)
	{
		SDK->TriggerUdpHeartbeat();
		if (!bProcessOwnerUpdates)
			return;
	}
		
	
	// Copy Actor Data into State Bytes
	FActorData ActorData;
	FMemory::Memcpy(&ActorData, ActorUpdateNotify.StateBytes.GetData(), ActorUpdateNotify.StateSize);
	
	// Enqueue for processing in Tick
	ActorUpdateQueue.Enqueue(ActorData);
}

FVector ANPCManager::SmoothDampVector(const FVector& Current, const FVector& Target, FVector& CurrentVelocity,
	float SmoothTime, float DeltaTime, float MaxSpeed)
{
	SmoothTime = FMath::Max(0.0001f, SmoothTime);

	const float Omega = 2.0f / SmoothTime;
	const float X = Omega * DeltaTime;
	const float Exp = 1.0f / (1.0f + X + 0.48f * X * X + 0.235f * X * X * X);

	FVector Change = Current - Target;
	const FVector OriginalTarget = Target;

	// Clamp max speed
	const float MaxChange = MaxSpeed * SmoothTime;
	const float ChangeLen = Change.Size();
	if (ChangeLen > MaxChange)
	{
		Change = Change / ChangeLen * MaxChange;
	}

	const FVector Temp = (CurrentVelocity + Omega * Change) * DeltaTime;
	CurrentVelocity = (CurrentVelocity - Omega * Temp) * Exp;

	FVector Output = OriginalTarget + (Change + Temp) * Exp;

	// Prevent overshoot
	const FVector OrigToCurrent = OriginalTarget - Current;
	const FVector OutToOrig = Output - OriginalTarget;
	if (FVector::DotProduct(OrigToCurrent, OutToOrig) > 0.0f)
	{
		Output = OriginalTarget;
		CurrentVelocity = FVector::ZeroVector;
	}

	return Output;
}

void ANPCManager::RemoveActorInstance(const FGuid& ActorId)
{
	if (!ISM) return;
	
	int32* IndexPtr = SpawnedActors.Find(ActorId);
	if (!IndexPtr) return;
	
	const int32 RemoveIndex = *IndexPtr;
	
	const int32 LastIndex = ISM->GetInstanceCount() - 1;
	
	const bool bRemoved = ISM->RemoveInstance(RemoveIndex);
	
	if (!bRemoved) return;
	
	SpawnedActors.Remove(ActorId);
	TargetById.Remove(ActorId);
	CurrentById.Remove(ActorId);
	VelocityById.Remove(ActorId);
	CurrentVelById.Remove(ActorId);
	LastUpdateTimeById.Remove(ActorId);
	
	if (RemoveIndex != LastIndex)
	{
		if (const FGuid* SwappedActorId = IdByInstanceIndex.Find(LastIndex))
		{
			const FGuid MovedId = *SwappedActorId;
			
			SpawnedActors.FindOrAdd(MovedId) = RemoveIndex;
			
			IdByInstanceIndex.Add(RemoveIndex, MovedId);
			IdByInstanceIndex.Remove(LastIndex);
		}
		else
		{
			IdByInstanceIndex.Remove(LastIndex);
		}
	}
	
	IdByInstanceIndex.Remove(RemoveIndex);
}



