// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "GrindingPlatform.generated.h"

class USplineComponent;

/**
 * 
 */
UCLASS()
class GRINDINGTUTORIAL_API AGrindingPlatform : public AStaticMeshActor
{
	GENERATED_BODY()
	
public:
	AGrindingPlatform();

	virtual void OnConstruction(const FTransform& Transform) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangeEvent) override;
#endif

	const TArray<TObjectPtr<USplineComponent>>& GetGrindSplines() const { return GrindSplines; }

private:
	UPROPERTY(EditInstanceOnly, meta = (DisplayName = "Grind Splines"))
	TArray<TObjectPtr<USplineComponent>> GrindSplines{};
};
