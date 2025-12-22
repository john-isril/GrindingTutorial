// Fill out your copyright notice in the Description page of Project Settings.


#include "GrindingPlatform.h"
#include <Components/SplineComponent.h>

AGrindingPlatform::AGrindingPlatform()
{
	if (UStaticMeshComponent* StaticMeshComp = GetStaticMeshComponent())
	{
		StaticMeshComp->SetCollisionProfileName(FName{ "GrindingPlatform" });
	}
}

void AGrindingPlatform::OnConstruction(const FTransform& Transform)
{
	// Pass off null spline responsibility to PostEditChangeProperty.
	if (GrindSplines.Contains(nullptr)) return;

	// If a new Spline Component was added to the components array, add it to the GrindSplines array if it's not already in there.
	for (UActorComponent* ActorComp : GetComponents())
	{
		USplineComponent* SplineComp = Cast<USplineComponent>(ActorComp);

		if (SplineComp && !GrindSplines.Contains(SplineComp))
		{
			GrindSplines.Add(SplineComp);
		}
	}

	// If 1 or more spline comps were deleted from the hierarchy, remove them from GrindSplines.
	GrindSplines.RemoveAll([this](TObjectPtr<USplineComponent> SplineComp) {
		return SplineComp && !OwnsComponent(SplineComp);
		});
}

#ifdef WITH_EDITOR

void AGrindingPlatform::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangeEvent)
{
	Super::PostEditChangeProperty(PropertyChangeEvent);

	// Handles the case when a new element is added to the GrindSplines array via the editor.
	if (PropertyChangeEvent.ChangeType == EPropertyChangeType::ArrayAdd)
	{
		const int32 NullElementIdx = GrindSplines.Find(nullptr);

		if (NullElementIdx != INDEX_NONE)
		{
			TObjectPtr<USplineComponent> NewSplineComp = Cast<USplineComponent>(AddComponentByClass(USplineComponent::StaticClass(), false, FTransform{}, false));
			if (!NewSplineComp) return;
			AddInstanceComponent(NewSplineComp);
			NewSplineComp->SetWorldScale3D(FVector{ 1.0 });
			NewSplineComp->SetAbsolute(false, false, true);
			GrindSplines[NullElementIdx] = NewSplineComp;
		}
	}

	// Handles the case when an element in the GrindSplines array is reset via the editor.
	{
		TArray<USplineComponent*> SplineCompsInOwnedComps{};
		GetComponents(SplineCompsInOwnedComps);

		for (USplineComponent* SplineComp : SplineCompsInOwnedComps)
		{
			if (!GrindSplines.Contains(SplineComp))
			{
				SplineComp->DestroyComponent();
			}
		}
		
		GrindSplines.RemoveAll([](TObjectPtr<USplineComponent> SplineComp) {
			return SplineComp == nullptr;
			});
	}
}

#endif // WITH_EDITOR
