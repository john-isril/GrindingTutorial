// Fill out your copyright notice in the Description page of Project Settings.


#include "GrindEffectsComponent.h"
#include "GameFramework/Character.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"

// Sets default values for this component's properties
UGrindEffectsComponent::UGrindEffectsComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	LeftFootGrindSparks = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LeftFootGrindSparks"));
	LeftFootGrindSparks->bAutoActivate = false;

	RightFootGrindSparks = CreateDefaultSubobject<UNiagaraComponent>(TEXT("RightFootGrindSparks"));
	RightFootGrindSparks->bAutoActivate = false;

	GrindSFX = CreateDefaultSubobject<UAudioComponent>(TEXT("GrindSFX"));
	GrindSFX->bAutoActivate = false;

	LeftFootGrindSocketName = "foot_l_GrindSocket";
	RightFootGrindSocketName = "foot_r_GrindSocket";
	// ...
}

void UGrindEffectsComponent::ActivateGrindEffects()
{
	if (LeftFootGrindSparks->GetAsset())
	{
		LeftFootGrindSparks->ActivateSystem();
	}

	if (RightFootGrindSparks->GetAsset())
	{
		RightFootGrindSparks->ActivateSystem();
	}

	if (GrindSFX->GetSound())
	{
		GrindSFX->Play();
	}
}

void UGrindEffectsComponent::DeactivateGrindEffects()
{
	if (LeftFootGrindSparks->IsActive())
	{
		LeftFootGrindSparks->Deactivate();
	}

	if (RightFootGrindSparks->IsActive())
	{
		RightFootGrindSparks->Deactivate();
	}

	if (GrindSFX->GetSound())
	{
		GrindSFX->Stop();
	}
}


// Called when the game starts
void UGrindEffectsComponent::BeginPlay()
{
	Super::BeginPlay();

	CharacterOwner = CastChecked<ACharacter>(GetOwner());

	AttachToMesh();

	// ...
	
}


// Called every frame
void UGrindEffectsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UGrindEffectsComponent::AttachToMesh()
{
	check(CharacterOwner && CharacterOwner->GetMesh());

	if (LeftFootGrindSparks->GetAsset())
	{
		LeftFootGrindSparks->AttachToComponent(CharacterOwner->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, LeftFootGrindSocketName);
	}

	if (RightFootGrindSparks->GetAsset())
	{
		RightFootGrindSparks->AttachToComponent(CharacterOwner->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, RightFootGrindSocketName);
	}

	if (GrindSFX->GetSound())
	{
		GrindSFX->AttachToComponent(CharacterOwner->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
}

