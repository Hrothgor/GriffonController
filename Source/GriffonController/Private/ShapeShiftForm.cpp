// Fill out your copyright notice in the Description page of Project Settings.


#include "ShapeShiftForm.h"
#include "ShapeShiftManager.h"

// Sets default values
AShapeShiftForm::AShapeShiftForm(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AShapeShiftForm::SetShapeShiftManager(AShapeShiftManager *ShapeShiftManager)
{
	ShapeShiftManagerRef = ShapeShiftManager;
}

AShapeShiftManager* AShapeShiftForm::GetShapeShiftManager() const
{
	return ShapeShiftManagerRef;
}

void AShapeShiftForm::StartShapeShifting()
{
	
}

