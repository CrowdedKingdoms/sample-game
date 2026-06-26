#include "Sample/SampleObjectEntity.h"

#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Replication/Components/CrowdyEntityComponent.h"
#include "UObject/ConstructorHelpers.h"

ASampleObjectEntity::ASampleObjectEntity()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	// Ship with a visible default so the spawn/move/rotate/destroy demo shows something
	// out of the box; override the mesh on a Blueprint subclass for your own art.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMesh.Object);
	}
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> CubeMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (CubeMaterial.Succeeded())
	{
		Mesh->SetMaterial(0, CubeMaterial.Object);
	}

	CrowdyEntity = CreateDefaultSubobject<UCrowdyEntityComponent>(TEXT("CrowdyEntity"));

	// Static mode: the object has no per-frame state to stream. It only changes when
	// one of its CrowdyEvents arrives, which is exactly what the switch sends.
	CrowdyEntity->Mode = ECrowdyEntityMode::Static;
	CrowdyEntity->IdentityPolicy = ECrowdyIdentityPolicy::Random;
}

void ASampleObjectEntity::SetObjectLocation_Implementation(FVector NewLocation)
{
	SetActorLocation(NewLocation);
}

void ASampleObjectEntity::SetObjectRotation_Implementation(FRotator NewRotation)
{
	SetActorRotation(NewRotation);
}
