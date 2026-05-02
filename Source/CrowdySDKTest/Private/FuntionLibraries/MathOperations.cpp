// Fill out your copyright notice in the Description page of Project Settings.


#include "FuntionLibraries/MathOperations.h"

void UMathOperations::GetMimicTransform(const FVector& SourceLocation, const FRotator& SourceRotation,
	const FVector& CenterLocation, const FRotator& CenterRotation, const float ForwardOffset, FVector& OutLocation,
	FRotator& OutRotation)
{
	const FTransform CenterTM(CenterRotation, CenterLocation);

	const FTransform SourceTM(SourceRotation, SourceLocation);
	const FTransform LocalTM = SourceTM.GetRelativeTransform(CenterTM);

	// Mirror position across Center's forward plane
	FVector MirroredLocal = LocalTM.GetLocation();
	MirroredLocal.X = -MirroredLocal.X + ForwardOffset;

	// Negate Yaw then add 180 — mirrors left/right AND faces you
	const FRotator LocalRot = LocalTM.GetRotation().Rotator();
	const FRotator MirroredRot(LocalRot.Pitch, 180.f - LocalRot.Yaw, LocalRot.Roll);

	const FTransform MirroredLocalTM(MirroredRot, MirroredLocal);
	const FTransform FinalTM = MirroredLocalTM * CenterTM;

	OutLocation = FinalTM.GetLocation();
	OutRotation = FinalTM.GetRotation().Rotator();
	
}
