// Fill out your copyright notice in the Description page of Project Settings.


#include "MovingPlatform.h"

AMovingPlatform::AMovingPlatform()
{
	PrimaryActorTick.bCanEverTick = true;

	SetMobility(EComponentMobility::Movable);
}

void AMovingPlatform::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SetReplicates(true);
		SetReplicateMovement(true);
	}

	GlobalStartLocation = GetActorLocation();
	GlobalTargetLocation = GetTransform().TransformPosition(TargetLocation);
}

void AMovingPlatform::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

    if (ActiveTriggers > 0)
    {
	    if (HasAuthority())
	    {
		    FVector Location = GetActorLocation();

		    if (FVector::Distance(Location, GlobalStartLocation) >= FVector::Distance(GlobalTargetLocation, GlobalStartLocation))
		    {
			    Swap(GlobalStartLocation, GlobalTargetLocation);
		    }
		
		    const FVector Direction = (GlobalTargetLocation - GlobalStartLocation).GetSafeNormal();
		    Location += Speed * DeltaSeconds * Direction;
		    SetActorLocation(Location);
	    }
    }
}

void AMovingPlatform::AddActiveTrigger()
{
    ActiveTriggers++;
}

void AMovingPlatform::RemoveActiveTrigger()
{
    if (ActiveTriggers > 0)
    {
        ActiveTriggers--;
    }
}
