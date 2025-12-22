// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GrindEffectsComponent.generated.h"

class UNiagaraComponent;
class UAudioComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GRINDINGTUTORIAL_API UGrindEffectsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGrindEffectsComponent();

	void ActivateGrindEffects();

	void DeactivateGrindEffects();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	TObjectPtr<ACharacter> CharacterOwner;

	UPROPERTY(EditAnywhere, Category = "GrindEffects", meta = (DisplayName = "Left Foot Grind Sparks"))
	TObjectPtr<UNiagaraComponent> LeftFootGrindSparks;

	UPROPERTY(EditAnywhere, Category = "GrindEffects", meta = (DisplayName = "Right Foot Grind Sparks"))
	TObjectPtr<UNiagaraComponent> RightFootGrindSparks;

	UPROPERTY(EditAnywhere, Category = "GrindEffects", meta = (DisplayName = "Grind SFX"))
	TObjectPtr<UAudioComponent> GrindSFX;

	UPROPERTY(VisibleAnywhere, Category = "GrindEffects", meta = (DisplayName = "Left Foot Grind Socket Name"))
	FName LeftFootGrindSocketName;

	UPROPERTY(VisibleAnywhere, Category = "GrindEffects", meta = (DisplayName = "Right Foot Grind Socket Name"))
	FName RightFootGrindSocketName;

private:

	void AttachToMesh();
		
};
