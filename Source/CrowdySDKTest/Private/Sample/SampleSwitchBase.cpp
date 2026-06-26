#include "Sample/SampleSwitchBase.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/Pawn.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ASampleSwitchBase::ASampleSwitchBase()
{
	PrimaryActorTick.bCanEverTick = false;

	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	Trigger->SetBoxExtent(FVector(75.f));
	Trigger->SetCollisionProfileName(TEXT("Trigger"));
	Trigger->SetGenerateOverlapEvents(true);
	RootComponent = Trigger;

	// A flat cylinder pad sitting at the bottom of the trigger, at the player's feet.
	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Body->SetupAttachment(RootComponent);
	Body->SetRelativeLocation(FVector(0.f, 0.f, -75.f));
	Body->SetRelativeScale3D(FVector(1.5f, 1.5f, 0.15f));
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		Body->SetStaticMesh(CylinderMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ShapeMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (ShapeMaterial.Succeeded())
	{
		Body->SetMaterial(0, ShapeMaterial.Object);
	}

	// Floating caption above the pad.
	Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label"));
	Label->SetupAttachment(RootComponent);
	Label->SetRelativeLocation(FVector(0.f, 0.f, 160.f));
	Label->SetHorizontalAlignment(EHTA_Center);
	Label->SetVerticalAlignment(EVRTA_TextBottom);
	Label->SetWorldSize(48.f);
	Label->SetText(SwitchLabel);
}

void ASampleSwitchBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyVisuals();
}

void ASampleSwitchBase::ApplyVisuals()
{
	if (Label)
	{
		Label->SetText(SwitchLabel);
		Label->SetTextRenderColor(SwitchColor);
	}

	if (Body)
	{
		if (!BodyMID)
		{
			BodyMID = Body->CreateDynamicMaterialInstance(0);
		}
		if (BodyMID)
		{
			// BasicShapeMaterial exposes a "Color" parameter; the call is harmless if a
			// swapped-in material does not have it.
			BodyMID->SetVectorParameterValue(TEXT("Color"), FLinearColor(SwitchColor));
		}
	}
}

void ASampleSwitchBase::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn)
	{
		return;
	}

	// Debounce so a single walk-through is a single interaction. GetTimeSeconds is
	// fine here because the switch only ever runs on the interacting client.
	const double Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
	if (Now - LastTriggerTime < ReTriggerDelay)
	{
		return;
	}
	LastTriggerTime = Now;

	// Route through the interface so Blueprint subclasses receive the same call.
	ISampleInteractable::Execute_Interact(this, Pawn);
}

void ASampleSwitchBase::BeginPlay()
{
	Super::BeginPlay();
	ApplyVisuals();
	Trigger->OnComponentBeginOverlap.AddDynamic(this, &ASampleSwitchBase::OnTriggerBeginOverlap);
}
