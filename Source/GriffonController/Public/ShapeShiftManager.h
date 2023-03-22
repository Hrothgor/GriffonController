// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnumFile.h"
#include "GameFramework/Actor.h"
#include "ShapeShiftManager.generated.h"

class AShapeShiftForm;

UCLASS()
class GRIFFONCONTROLLER_API AShapeShiftManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AShapeShiftManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetActiveCharacter(ACharacter *Character, bool Active);
	
	void ShapeShiftBackToDruid();
	void ShapeShiftToForm(EShapeShiftForm form);

	UPROPERTY(EditAnywhere)
	TSubclassOf<AShapeShiftForm> ShapeShiftFormDruidClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AShapeShiftForm> ShapeShiftFormGriffonClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AShapeShiftForm> ShapeShiftFormWerewolfClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AShapeShiftForm> ShapeShiftFormSeaCreatureClass;

	UPROPERTY()
	TArray<AShapeShiftForm *> CharacterRefs;
	EShapeShiftForm ActualForm;
};
