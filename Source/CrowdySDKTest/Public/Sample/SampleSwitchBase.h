#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sample/SampleInteractable.h"
#include "SampleSwitchBase.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UTextRenderComponent;
class UMaterialInstanceDynamic;

/**
 * Base for the sample's interact switches. It carries a trigger box and fires
 * Interact when a pawn overlaps it, so each example needs to override one function
 * and nothing else. The original sample drove these from a Blueprint switch with an
 * interact interface; this is the same idea in C++ so both languages share a shape.
 *
 * It also draws itself: a colored pedestal plus a floating label so every example is
 * visible and self-describing in the level. Each subclass sets SwitchLabel and
 * SwitchColor in its constructor.
 */
UCLASS(Abstract)
class CROWDYSDKTEST_API ASampleSwitchBase : public AActor, public ISampleInteractable
{
	GENERATED_BODY()

public:

	ASampleSwitchBase();

	virtual void OnConstruction(const FTransform& Transform) override;

protected:

	virtual void BeginPlay() override;

	// Default does nothing; each example switch overrides this.
	virtual void Interact_Implementation(APawn* Interactor) override {}

	// Pushes SwitchLabel/SwitchColor onto the visual components. Safe to call in the
	// editor (OnConstruction) and at runtime.
	void ApplyVisuals();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Sample")
	TObjectPtr<UBoxComponent> Trigger;

	// The pedestal you see in the level; tinted by SwitchColor.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Sample|Visual")
	TObjectPtr<UStaticMeshComponent> Body;

	// Floating caption naming what this switch demonstrates.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Sample|Visual")
	TObjectPtr<UTextRenderComponent> Label;

	// What the floating caption reads. Set per subclass.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sample|Visual")
	FText SwitchLabel;

	// Pedestal and caption color. Set per subclass.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sample|Visual")
	FColor SwitchColor = FColor::White;

	// Ignore repeat overlaps until the pawn leaves and returns, so one walk-through
	// is one interaction rather than a stream of them.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sample")
	float ReTriggerDelay = 1.0f;

private:

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep);

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> BodyMID;

	double LastTriggerTime = 0.0;
};
