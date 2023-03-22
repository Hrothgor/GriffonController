// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShapeShiftForm.generated.h"

class UInputAction;
class UInputMappingContext;
class AShapeShiftManager;
class UInputComponent;

UCLASS()
class GRIFFONCONTROLLER_API AShapeShiftForm : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShapeShiftForm(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** ShapeShift Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ShapeShiftAction;
	
public:
	void SetShapeShiftManager(AShapeShiftManager *ShapeShiftManager);
	AShapeShiftManager *GetShapeShiftManager() const;

	virtual void StartShapeShifting();

private:
	UPROPERTY()
	AShapeShiftManager *ShapeShiftManagerRef = nullptr;
};
