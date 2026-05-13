// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/SamplePoolPolicy.h"

void USamplePoolPolicy::OnActorActivated_Implementation(AActor* Actor, const FInstancedStruct& InitialState)
{
	Super::OnActorActivated_Implementation(Actor, InitialState);
}

void USamplePoolPolicy::OnActorDeactivated_Implementation(AActor* Actor)
{
	Super::OnActorDeactivated_Implementation(Actor);
}

void USamplePoolPolicy::OnActorPooled_Implementation(AActor* Actor)
{
	Super::OnActorPooled_Implementation(Actor);
}
