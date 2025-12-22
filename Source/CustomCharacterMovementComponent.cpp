// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "GrindingPlatform.h"
#include "Components/SplineComponent.h"

void UCustomCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	GrindState = {};
	GrindDetectionRadiusSquared = GrindDetectionRadius * GrindDetectionRadius;
}

bool UCustomCharacterMovementComponent::CanAttemptJump() const
{
	return Super::CanAttemptJump() || IsGrinding();
}

bool UCustomCharacterMovementComponent::IsGrinding() const
{
	return (MovementMode == MOVE_Custom && CustomMovementMode == CMOVE_Grinding);
}

void UCustomCharacterMovementComponent::AddInputVector(FVector WorldVector, bool bForce)
{
	if (!IsGrinding())
	{
		Super::AddInputVector(WorldVector, bForce);
	}
}

void UCustomCharacterMovementComponent::PhysFalling(float deltaTime, int32 Iterations)
{
	const bool bEnteredGrind = TryEnterGrind();

	if (bEnteredGrind)
	{
		StartNewPhysics(deltaTime, Iterations);
	}
	else
	{
		Super::PhysFalling(deltaTime, Iterations);
	}
}

bool UCustomCharacterMovementComponent::TryEnterGrind()
{
	if ((MovementMode != EMovementMode::MOVE_Falling) || (Velocity.Z >= 0.0)) return false;

	FHitResult HitResult{};
	const FVector TraceStart = GetActorFeetLocation();
	const FVector TraceEnd = TraceStart + FVector{ 0.0, 0.0, -1.0 };
	constexpr ECollisionChannel GrindCollisionChannel = ECC_GameTraceChannel2;
	GetWorld()->SweepSingleByChannel(HitResult, TraceStart, TraceEnd, FQuat::Identity, GrindCollisionChannel, FCollisionShape::MakeSphere(GrindDetectionRadius));
	
	DrawDebugSphere(GetWorld(), TraceStart, GrindDetectionRadius, 32, FColor::Red);
	
	if (!HitResult.bBlockingHit) return false;

	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Black, TEXT("Hit grinding platform!"));

	AGrindingPlatform* HitGrindingPlatform = CastChecked<AGrindingPlatform>(HitResult.GetActor());
	const FVector CharacterLocation = GetActorLocation();
	const float CharacterHalfHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector CharacterForward = CharacterOwner->GetActorForwardVector();
	USplineComponent* BestGrindSpline = nullptr;
	FTransform BestGrindTransform{};
	double BestGrindSplineScore{};
	FVector BestGrindFwd{};

	for (const TObjectPtr<USplineComponent> GrindSpline : HitGrindingPlatform->GetGrindSplines())
	{
		checkf(GrindSpline, TEXT("GrindSpline is null!"));

		FTransform GrindTransform = GrindSpline->FindTransformClosestToWorldLocation(CharacterLocation, ESplineCoordinateSpace::World);

		const FVector CharacterToGrindLocation = GrindTransform.GetLocation() - CharacterLocation;

		if (FVector::DotProduct(CharacterToGrindLocation, Velocity) < 0.0)
		{
			continue;
		}

		const FVector CharacterHeightOffset = GrindTransform.GetUnitAxis(EAxis::Z) * CharacterHalfHeight;
		GrindTransform.AddToTranslation(CharacterHeightOffset);

		if (FVector::DistSquared(CharacterLocation, GrindTransform.GetLocation()) > GrindDetectionRadiusSquared)
		{
			continue;
		}

		if (BestGrindSpline)
		{
			const FVector GrindFwd = GrindTransform.GetUnitAxis(EAxis::X);

			const double GrindSplineScore = FMath::Abs(FVector::DotProduct(CharacterForward, GrindFwd));

			if (GrindSplineScore > BestGrindSplineScore)
			{
				BestGrindSpline = GrindSpline;
				BestGrindTransform = GrindTransform;
				BestGrindFwd = GrindFwd;
				BestGrindSplineScore = GrindSplineScore;
			}
		}
		else
		{
			BestGrindSpline = GrindSpline;
			BestGrindTransform = GrindTransform;
			BestGrindFwd = BestGrindTransform.GetUnitAxis(EAxis::X);
			BestGrindSplineScore = FMath::Abs(FVector::DotProduct(CharacterForward, BestGrindFwd));
		}
	}

	if (!BestGrindSpline) return false;

	for (float Distance = 0.0f; Distance <= BestGrindSpline->GetSplineLength(); Distance += 30.0f)
	{
		const FVector SphereLocation = BestGrindSpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
		DrawDebugSphere(GetWorld(), SphereLocation, 15.0f, 16, FColor::Red);
	}

	GrindState.GrindingPlatform = HitGrindingPlatform;
	GrindState.GrindSpline = BestGrindSpline;
	GrindState.CharacterHalfHeight = CharacterHalfHeight;
	GrindState.bGrindingForward = FVector::DotProduct(CharacterForward, BestGrindFwd) > 0.0;
	GrindState.DistanceAlongGrind = GrindState.GrindSpline->GetDistanceAlongSplineAtLocation(CharacterLocation, ESplineCoordinateSpace::World);

	if (GrindState.bGrindingForward)
	{
		GrindState.DistanceAlongGrind += GrindSpeed * GrindState.MoveToGrindEntryPointDuration;
	}
	else
	{
		GrindState.DistanceAlongGrind -= GrindSpeed * GrindState.MoveToGrindEntryPointDuration;
	}

	if (GrindState.GrindSpline->IsClosedLoop())
	{
		GrindState.DistanceAlongGrind = FMath::Wrap(GrindState.DistanceAlongGrind, 0.0f, GrindState.GrindSpline->GetSplineLength());
	}

	GrindState.GrindDetectionLocation = CharacterLocation;
	GrindState.GrindDetectionRotation = CharacterOwner->GetActorQuat();

	GrindState.GrindEntryRotation = GrindState.GrindSpline->GetQuaternionAtDistanceAlongSpline(GrindState.DistanceAlongGrind, ESplineCoordinateSpace::World);

	if (!GrindState.bGrindingForward)
	{
		GrindState.GrindEntryRotation *= FQuat(FVector::UpVector, UE_PI);
	}

	GrindState.GrindEntryLocation = GrindState.GrindSpline->GetLocationAtDistanceAlongSpline(GrindState.DistanceAlongGrind, ESplineCoordinateSpace::World);
	GrindState.GrindEntryLocation += GrindState.GrindEntryRotation.GetUpVector() * GrindState.CharacterHalfHeight;
	GrindState.MoveToGrindEntryPointTimeElapsed = 0.0f;

	CharacterOwner->MoveIgnoreActorAdd(GrindState.GrindingPlatform.Get());
	GrindState.bMovingToGrindEntryPoint = true;
	SetMovementMode(EMovementMode::MOVE_Custom, ECustomMovementMode::CMOVE_Grinding);

	return true;
}

void UCustomCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);

	switch (CustomMovementMode)
	{
	case ECustomMovementMode::CMOVE_Grinding:
		PhysGrinding(deltaTime, Iterations);

		break;

	default:
		break;
	}

}

void UCustomCharacterMovementComponent::PhysGrinding(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME) return;

	const FVector LastLocation = UpdatedComponent->GetComponentLocation();
	FVector NewLocation{};
	FQuat NewRotation{};
	bool bShouldContinueGrinding = true;

	if (GrindState.bMovingToGrindEntryPoint)
	{
		GrindState.MoveToGrindEntryPointTimeElapsed += deltaTime;
		checkf(GrindState.MoveToGrindEntryPointDuration >= UE_SMALL_NUMBER, TEXT("MoveToGrindEntryPointDuration must be greater than 0.0!"));
		float Alpha = GrindState.MoveToGrindEntryPointTimeElapsed / GrindState.MoveToGrindEntryPointDuration;
		Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);

		NewLocation = FMath::Lerp(GrindState.GrindDetectionLocation, GrindState.GrindEntryLocation, Alpha);
		NewRotation = FQuat::Slerp(GrindState.GrindDetectionRotation, GrindState.GrindEntryRotation, Alpha);
	}
	else
	{
		if (GrindState.bGrindingForward)
		{
			GrindState.DistanceAlongGrind += GrindSpeed * deltaTime;
		}
		else
		{
			GrindState.DistanceAlongGrind -= GrindSpeed * deltaTime;
		}

		const float SplineLength = GrindState.GrindSpline->GetSplineLength();
		const bool bPassedEndpoint = (GrindState.DistanceAlongGrind >= SplineLength) || (GrindState.DistanceAlongGrind <= 0.0f);

		if (bPassedEndpoint && !(GrindState.GrindSpline->IsClosedLoop()))
		{
			bShouldContinueGrinding = false;
			NewRotation = UpdatedComponent->GetComponentQuat();
			NewLocation = LastLocation + UpdatedComponent->GetForwardVector() * GrindSpeed * deltaTime;
		}
		else
		{
			if (GrindState.GrindSpline->IsClosedLoop())
			{
				GrindState.DistanceAlongGrind = FMath::Wrap(GrindState.DistanceAlongGrind, 0.0f, SplineLength);
			}

			NewRotation = GrindState.GrindSpline->GetQuaternionAtDistanceAlongSpline(GrindState.DistanceAlongGrind, ESplineCoordinateSpace::World);

			if (!GrindState.bGrindingForward)
			{
				NewRotation *= FQuat(FVector::UpVector, UE_PI);
			}

			NewLocation = GrindState.GrindSpline->GetLocationAtDistanceAlongSpline(GrindState.DistanceAlongGrind, ESplineCoordinateSpace::World);
			NewLocation += NewRotation.GetUpVector() * GrindState.CharacterHalfHeight;
		}
	}

	Iterations++;
	bJustTeleported = false;

	const FVector DeltaLocation = NewLocation - LastLocation;
	FHitResult HitResult{};
	SafeMoveUpdatedComponent(DeltaLocation, NewRotation, true, HitResult);

	Velocity = (UpdatedComponent->GetComponentLocation() - LastLocation) / deltaTime;

	if (HitResult.bBlockingHit)
	{
		SetMovementMode(MOVE_Walking);
	}
	else if (!bShouldContinueGrinding)
	{
		SetMovementMode(EMovementMode::MOVE_Falling);
	}

	const FString DebugString = FString::Printf(TEXT("Distance Along Grind: %f"), GrindState.DistanceAlongGrind);

	DrawDebugString(GetWorld(), GetActorLocation(), DebugString, nullptr, FColor::White, 0.0f, true, 2.0f);
}

void UCustomCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

	if (GrindState.bMovingToGrindEntryPoint && (GrindState.MoveToGrindEntryPointTimeElapsed >= GrindState.MoveToGrindEntryPointDuration))
	{
		GrindState.bMovingToGrindEntryPoint = false;
		OnGrindBegin.ExecuteIfBound();
	}
}

void UCustomCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == CMOVE_Grinding)
	{
		CharacterOwner->MoveIgnoreActorRemove(GrindState.GrindingPlatform.Get());
		OnGrindEnd.ExecuteIfBound();
	}
}
