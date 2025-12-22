// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CustomCharacterMovementComponent.generated.h"

class AGrindingPlatform;
class USplineComponent;

UENUM(BlueprintType)
enum ECustomMovementMode : int
{
	CMOVE_Grinding		UMETA(DisplayName = "Grinding"),

	CMOVE_Max			UMETA(Hidden)
};

USTRUCT()
struct FGrindState
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<AGrindingPlatform> GrindingPlatform = nullptr;

	UPROPERTY()
	TWeakObjectPtr<USplineComponent> GrindSpline = nullptr;

	FQuat GrindDetectionRotation{};

	FQuat GrindEntryRotation{};

	FVector GrindDetectionLocation{};

	FVector GrindEntryLocation{};

	float CharacterHalfHeight = 0.0f;

	float MoveToGrindEntryPointDuration = 0.2f;

	float MoveToGrindEntryPointTimeElapsed = 0.0f;

	float DistanceAlongGrind = 0.0f;

	bool bGrindingForward = true;

	bool bMovingToGrindEntryPoint = true;
};

DECLARE_DELEGATE(FOnGrindBeginSignature);
DECLARE_DELEGATE(FOnGrindEndSignature);

/**
 * 
 */
UCLASS()
class GRINDINGTUTORIAL_API UCustomCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:

	FOnGrindBeginSignature OnGrindBegin;

	FOnGrindEndSignature OnGrindEnd;

public:

	virtual void BeginPlay() override;

	virtual bool CanAttemptJump() const override;

	UFUNCTION(BlueprintCallable)
	bool IsGrinding() const;

	virtual void AddInputVector(FVector WorldVector, bool bForce = false) override;

private:

	UPROPERTY()
	FGrindState GrindState{};

	UPROPERTY(EditAnywhere, Category = Grinding, meta = (DisplayName = "Grind Detection Radius"))
	float GrindDetectionRadius = 50.0;

	UPROPERTY(EditAnywhere, Category = Grinding, meta = (DisplayName = "Grind Speed"))
	float GrindSpeed = 600.0f;

	float GrindDetectionRadiusSquared;

protected:
	virtual void PhysFalling(float deltaTime, int32 Iterations) override;

	virtual bool TryEnterGrind();

	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

	virtual void PhysGrinding(float deltaTime, int32 Iterations);

	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

};
