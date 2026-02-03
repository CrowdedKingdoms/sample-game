#pragma once
#include "CoreMinimal.h"
#include "FBaseEventState.generated.h"


USTRUCT(BlueprintType, Blueprintable)
struct FBaseEventState
{
	GENERATED_BODY()
	
	// Common properties all events might have
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game Event")
	FVector Location;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game Event")
	FRotator Rotation;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game Event")
	FVector Scale = FVector(1.0f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game Event")
	int64 Timestamp;
	
	// Virtual destructor for proper cleanup
	virtual ~FBaseEventState() {}

};

USTRUCT(BlueprintType, Blueprintable)
struct FBallState : public FBaseEventState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game Event")
	uint8 Version = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game Event")
	FVector Velocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game Event")
	float InitialSpeed;
};