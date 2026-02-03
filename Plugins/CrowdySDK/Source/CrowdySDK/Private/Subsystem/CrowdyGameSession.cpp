// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/CrowdyGameSession.h"

void UCrowdyGameSession::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("[CrowdySDK]: Game Session Initialized."))
}

void UCrowdyGameSession::Deinitialize()
{
	Super::Deinitialize();
}

bool UCrowdyGameSession::EnqueueMessageToSend(TArray<uint8>&& Message)
{
	++SendCounter;
	return SendQueue.Enqueue(MoveTemp(Message));
}

bool UCrowdyGameSession::DequeueMessageToSend(TArray<uint8>& OutMessage)
{
	TArray<uint8> Tmp;
	if (!SendQueue.Dequeue(Tmp))
		return false;

	OutMessage = MoveTemp(Tmp);
	return true;
}

void UCrowdyGameSession::EnqueueMessageToReceive(const TArray<uint8>& Message)
{
	FScopeLock Lock(&ReceiveQueueMutex);
	ReceiveQueue.Enqueue(Message);
	++ReceiveCounter;
}

bool UCrowdyGameSession::DequeueMessageToReceive(TArray<uint8>& OutMessage)
{
	FScopeLock Lock(&ReceiveQueueMutex);
	
	if (ReceiveQueue.IsEmpty()) return false;
	
	ReceiveQueue.Dequeue(OutMessage);
	--ReceiveCounter;
	return true;
}

bool UCrowdyGameSession::HasPendingIncomingMessages() const
{
	return !ReceiveQueue.IsEmpty();
}

bool UCrowdyGameSession::HasPendingOutgoingMessages() const
{
	return !SendQueue.IsEmpty();
}
