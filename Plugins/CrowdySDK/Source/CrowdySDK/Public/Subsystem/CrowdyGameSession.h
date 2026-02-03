#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Containers/Queue.h"
#include "HAL/CriticalSection.h"
#include "Math/MathFwd.h"
#include "CrowdyGameSession.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FGameSessionInfo
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, Category="CrowdySDK|Game Session")
	int64 MapID = 1;
	
	UPROPERTY(BlueprintReadWrite, Category="CrowdySDK|Game Session")
	FString GameToken = "";
	
	UPROPERTY(BlueprintReadWrite, Category="CrowdySDK|Game Session")
	int64 GameTokenID = 0;
	
	UPROPERTY(BlueprintReadWrite, Category="CrowdySDK|Game Session")
	int64 UserID = 0;
	
	UPROPERTY(BlueprintReadWrite, Category="CrowdySDK|Game Session")
	FString UUID = "";
	
	UPROPERTY()
	FInt64Vector CurrentPlayerChunkCoordinates = {0, 0, 0};
	
	UPROPERTY()
	FInt64Vector LastMinigameChunkCoordinates = {0, 0, 0};
	
	UPROPERTY()
	FInt32Vector LastMinigameVoxelCoordinates = {0, 0, 0};
	
	UPROPERTY(BlueprintReadWrite, Category="CrowdySDK|Game Session")
	bool bWasInMinigame = false;
	
	void Reset()
	{
		MapID = 0;
		GameToken = "";
		GameTokenID = 0;
		UserID = 0;
		UUID = "";
		CurrentPlayerChunkCoordinates = {0, 0, 0};
		LastMinigameChunkCoordinates = {0, 0, 0};
		LastMinigameVoxelCoordinates = {0, 0, 0};
		bWasInMinigame = false;
	}
};

UCLASS(BlueprintType)
class CROWDYSDK_API UCrowdyGameSession : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	[[nodiscard]] bool EnqueueMessageToSend(TArray<uint8>&& Message);

	bool DequeueMessageToSend(TArray<uint8>& OutMessage);

	void EnqueueMessageToReceive(const TArray<uint8>& Message);
	
	[[nodiscard]] bool DequeueMessageToReceive(TArray<uint8>& OutMessage);
	
	UFUNCTION(BlueprintCallable, Category = "CrowdySDK|Game Session")
	void SetMapID(const int64 InMapID) {GameSessionInfo.MapID = InMapID;}
	
	UFUNCTION(BlueprintCallable, Category = "CrowdySDK|Game Session")
	void SetUserID(const int64 InUserID){GameSessionInfo.UserID = InUserID;}
	
	UFUNCTION(BlueprintCallable, Category = "CrowdySDK|Game Session")
	void SetGameToken(const FString InGameToken){GameSessionInfo.GameToken = InGameToken;}
	
	UFUNCTION(BlueprintCallable, Category = "CrowdySDK|Game Session")
	void SetUUID(FString InUUID){GameSessionInfo.UUID = InUUID;}
	
	UFUNCTION(BlueprintCallable, Category = "CrowdySDK|Game Session")
	void SetGameTokenID(const int64 InGameTokenID) {GameSessionInfo.GameTokenID = InGameTokenID;}
	
	UFUNCTION(BlueprintCallable, Category = "CrowdySDK|Game Session")
	void SetPlayerCurrentChunkCoordinates(const int64 X, const int64 Y, const int64 Z) {GameSessionInfo.CurrentPlayerChunkCoordinates = FInt64Vector(X, Y, Z);}
	
	UFUNCTION()
	FInt64Vector GetPlayerCurrentChunkCoordinates() const {return GameSessionInfo.CurrentPlayerChunkCoordinates;}
	
	UFUNCTION(BlueprintPure, Category = "CrowdySDK|Game Session")
	int64 GetMapID() const { return GameSessionInfo.MapID;}
	
	UFUNCTION(BlueprintPure, Category = "CrowdySDK|Game Session")
	FString GetGameToken() const {return GameSessionInfo.GameToken;}
	
	UFUNCTION(BlueprintPure, Category = "CrowdySDK|Game Session")
	FString GetUUID() const {return GameSessionInfo.UUID;}
	
	UFUNCTION(BlueprintPure, Category = "CrowdySDK|Game Session")
	int64 GetUserID() const {return GameSessionInfo.UserID;}
	
	UFUNCTION(BlueprintPure, Category = "CrowdySDK|Game Session")
	int64 GetGameTokenID() const {return GameSessionInfo.GameTokenID;}
	
	UFUNCTION(BlueprintCallable, Category = "CrowdySDK|Game Session")
	void ClearCurrentSessionData() {GameSessionInfo.Reset();}
	
	UFUNCTION(BlueprintPure, Category = "CrowdySDK|Game Session")
	FGameSessionInfo GetCurrentGameSessionInfo() const {return GameSessionInfo;}
	
	UFUNCTION(BlueprintCallable, Category = "CrowdySDK|Game Session")
	void SetLastMinigameCoordinates(int64 ChunkX, int64 ChunkY, int64 ChunkZ, int32 VoxelX, int32 VoxelY, int32 VoxelZ)
	{
		GameSessionInfo.LastMinigameChunkCoordinates = {ChunkX, ChunkY, ChunkZ};
		GameSessionInfo.LastMinigameVoxelCoordinates = {VoxelX, VoxelY, VoxelZ};
	};
	
	UFUNCTION(BlueprintPure, Category = "CrowdySDK|Game Session")
	void GetLastMinigameCoordinates(int64& ChunkX, int64& ChunkY, int64& ChunkZ, int32& VoxelX, int32& VoxelY, int32& VoxelZ) const
	{
		ChunkX = GameSessionInfo.LastMinigameChunkCoordinates.X;
		ChunkY = GameSessionInfo.LastMinigameChunkCoordinates.Y;
		ChunkZ = GameSessionInfo.LastMinigameChunkCoordinates.Z;
		VoxelX = GameSessionInfo.LastMinigameVoxelCoordinates.X;
		VoxelY = GameSessionInfo.LastMinigameVoxelCoordinates.Y;
		VoxelZ = GameSessionInfo.LastMinigameVoxelCoordinates.Z;
	}
	
	UFUNCTION(BlueprintCallable, Category = "CrowdySDK|Game Session")
	void SetWasInMinigame(const bool bWasInMinigame) {GameSessionInfo.bWasInMinigame = bWasInMinigame;}
	
	[[nodiscard]] bool HasPendingIncomingMessages() const; 
	
	[[nodiscard]] bool HasPendingOutgoingMessages() const;
private:
	
	UPROPERTY()
	FGameSessionInfo GameSessionInfo;
	
	TQueue<TArray<uint8>, EQueueMode::Mpsc> SendQueue;
	TQueue<TArray<uint8>, EQueueMode::Spsc> ReceiveQueue;
	
	std::atomic<int32> SendCounter = 0;
	std::atomic<int32> ReceiveCounter = 0;
	
	FCriticalSection SendQueueMutex;
	FCriticalSection ReceiveQueueMutex;
};
