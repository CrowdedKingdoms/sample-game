// Fill out your copyright notice in the Description page of Project Settings.


#include "Replication/Subsystems/CrowdyActorPoolSubsystem.h"

void UCrowdyActorPoolSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UCrowdyActorPoolSubsystem::Deinitialize()
{
	for (auto& [Class, Pool] : Pools)
	{
		for (FSlot& Slot : Pool.Slots)
		{
			if (!Slot.Actor.IsValid())
				continue;

			if (Slot.bActive && Pool.Policy)
				Pool.Policy->OnActorDeactivated(Slot.Actor.Get());

			AActor* Actor = Slot.Actor.Get();
			if (IsValid(Actor))
				Actor->Destroy();

			Slot.Actor = nullptr;
		}

		Pool.Slots.Empty();
		Pool.Policy = nullptr;
	}

	Pools.Empty();
	UUIDToActor.Empty();

	Super::Deinitialize();
}

bool UCrowdyActorPoolSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}

	const UWorld* World = Outer ? Outer->GetWorld() : nullptr;
	if (!World)
	{
		return false;
	}

	// PIE or Game only
	if (World->WorldType != EWorldType::PIE &&
		World->WorldType != EWorldType::Game)
	{
		return false;
	}
	
	return true;
}

void UCrowdyActorPoolSubsystem::RegisterPool(const FCrowdyPoolConfig& Config)
{
	if (!Config.ActorClass || Pools.Contains(Config.ActorClass.Get()))
		return;

	const TSubclassOf<UCrowdyActorPoolPolicy> PolicyClass = Config.PoolPolicyClass
	? Config.PoolPolicyClass
	: TSubclassOf<UCrowdyActorPoolPolicy>(UCrowdyActorPoolPolicy::StaticClass());

	FPool& Pool     = Pools.Add(Config.ActorClass.Get());
	Pool.ActorClass = Config.ActorClass;
	Pool.Policy     = NewObject<UCrowdyActorPoolPolicy>(this, PolicyClass);

	const FVector Origin = FVector::ZeroVector;
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	Pool.Slots.Reserve(Config.PoolSize);
	for (int32 i = 0; i < Config.PoolSize; i++)
	{
		AActor* Actor = GetWorld()->SpawnActor<AActor>(Config.ActorClass, Origin, FRotator::ZeroRotator, Params);
		if (!Actor) continue;

		Pool.Policy->OnActorPooled(Actor);

		FSlot Slot;
		Slot.Actor   = Actor;
		Slot.bActive = false;
		Pool.Slots.Add(Slot);
	}
}

AActor* UCrowdyActorPoolSubsystem::AcquireActor(const TSubclassOf<AActor> ActorClass, const FGuid& UUID,
	const FInstancedStruct& InitialState)
{
	// If UUID is already active, return the existing actor instead of double-acquiring
	if (AActor** Existing = UUIDToActor.Find(UUID))
	{
		if (IsValid(*Existing))
			return *Existing;

		// Stale entry — clean it up before reacquiring
		UUIDToActor.Remove(UUID);
	}

	FPool* Pool = FindPool(ActorClass.Get());
	if (!Pool) return nullptr;

	for (FSlot& Slot : Pool->Slots)
	{
		if (Slot.bActive || !Slot.Actor.IsValid()) continue;

		Slot.bActive = true;
		Slot.UUID    = UUID;

		AActor* Actor = Slot.Actor.Get();
		if (!IsValid(Actor)) continue;  // skip stale weak ptrs

		Pool->Policy->OnActorActivated(Actor, InitialState);
		UUIDToActor.Add(UUID, Actor);
		return Actor;
	}

	UE_LOG(LogTemp, Warning, TEXT("[CrowdyActorPool]: Pool exhausted for class %s"), *ActorClass->GetName());
	return nullptr;
}

void UCrowdyActorPoolSubsystem::ReleaseActor(const FGuid& UUID)
{
	AActor** ActorPtr = UUIDToActor.Find(UUID);
	if (!ActorPtr) return;

	AActor* Actor = *ActorPtr;
	UUIDToActor.Remove(UUID);

	for (auto& [Class, Pool] : Pools)
	{
		for (FSlot& Slot : Pool.Slots)
		{
			if (Slot.Actor.Get() != Actor) continue;
			Slot.bActive = false;
			Slot.UUID    = FGuid{};
			Pool.Policy->OnActorDeactivated(Actor);
			return;
		}
	}
}

AActor* UCrowdyActorPoolSubsystem::FindActor(const FGuid& UUID) const
{
	AActor* const* ActorPtr = UUIDToActor.Find(UUID);
	return ActorPtr ? *ActorPtr : nullptr;
}

UCrowdyActorPoolSubsystem::FPool* UCrowdyActorPoolSubsystem::FindPool(const UClass* ActorClass)
{
	return Pools.Find(ActorClass);
}
