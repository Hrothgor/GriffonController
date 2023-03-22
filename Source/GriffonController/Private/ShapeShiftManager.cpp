// Fill out your copyright notice in the Description page of Project Settings.


#include "ShapeShiftManager.h"
#include "ShapeShiftForm.h"
#include "GameFramework/Character.h"
#include "GameFramework/PawnMovementComponent.h"

// Sets default values
AShapeShiftManager::AShapeShiftManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CharacterRefs.Init(nullptr, 4);
}

// Called when the game starts or when spawned
void AShapeShiftManager::BeginPlay()
{
	Super::BeginPlay();
	
	// DRUID
	if (ShapeShiftFormDruidClass)
	{
		CharacterRefs[SSForm_Druid] = GetWorld()->SpawnActor<AShapeShiftForm>(ShapeShiftFormDruidClass, GetActorLocation(), GetActorRotation());
		if (CharacterRefs[SSForm_Druid])
		{
			CharacterRefs[SSForm_Druid]->SetShapeShiftManager(this);
			SetActiveCharacter(CharacterRefs[SSForm_Druid], false);
		}
	}
	
	// GRIFFON
	if (ShapeShiftFormGriffonClass)
	{
		CharacterRefs[SSForm_Griffon] = GetWorld()->SpawnActor<AShapeShiftForm>(ShapeShiftFormGriffonClass, GetActorLocation(), GetActorRotation());
		if (CharacterRefs[SSForm_Griffon])
		{
			CharacterRefs[SSForm_Griffon]->SetShapeShiftManager(this);
			SetActiveCharacter(CharacterRefs[SSForm_Griffon], false);
		}
	}
	
	// Werewolf
	if (ShapeShiftFormWerewolfClass)
	{
		CharacterRefs[SSForm_Werewolf] = GetWorld()->SpawnActor<AShapeShiftForm>(ShapeShiftFormWerewolfClass, GetActorLocation(), GetActorRotation());
		if (CharacterRefs[SSForm_Werewolf])
		{
			CharacterRefs[SSForm_Werewolf]->SetShapeShiftManager(this);
			SetActiveCharacter(CharacterRefs[SSForm_Werewolf], false);
		}
	}
	
	// SeaCreature
	if (ShapeShiftFormSeaCreatureClass)
	{
		CharacterRefs[SSForm_SeaCreature] = GetWorld()->SpawnActor<AShapeShiftForm>(ShapeShiftFormSeaCreatureClass, GetActorLocation(), GetActorRotation());
		if (CharacterRefs[SSForm_SeaCreature])
		{
			CharacterRefs[SSForm_SeaCreature]->SetShapeShiftManager(this);
			SetActiveCharacter(CharacterRefs[SSForm_SeaCreature], false);
		}
	}

	ShapeShiftToForm(SSForm_Druid);
}

// Called every frame
void AShapeShiftManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AShapeShiftManager::SetActiveCharacter(ACharacter *Character, bool Active)
{
	if (Active == true)
	{
		Character->SetActorHiddenInGame(false);
		Character->SetActorEnableCollision(true);
		Character->SetActorTickEnabled(true);
		Character->GetMovementComponent()->Activate();
	} else
	{
		Character->SetActorHiddenInGame(true);
		Character->SetActorEnableCollision(false);
		Character->SetActorTickEnabled(false);
		Character->GetMovementComponent()->Deactivate();
	}
}

void AShapeShiftManager::ShapeShiftBackToDruid()
{
	if (ActualForm == SSForm_Druid)
		return;
	
	ActualForm = SSForm_Druid;
}

void AShapeShiftManager::ShapeShiftToForm(EShapeShiftForm form)
{
	if (CharacterRefs[ActualForm] && CharacterRefs[form])
	{
		SetActiveCharacter(CharacterRefs[ActualForm], false);
		SetActiveCharacter(CharacterRefs[form], true);

		CharacterRefs[form]->SetActorLocation(CharacterRefs[ActualForm]->GetActorLocation());
		CharacterRefs[form]->SetActorRotation(FRotator(0, CharacterRefs[ActualForm]->GetActorRotation().Yaw, 0));

		APlayerController *Controller = GetWorld()->GetFirstPlayerController();

		FRotator RotationController = Controller->GetControlRotation();
		Controller->Possess(CharacterRefs[form]);
		Controller->SetControlRotation(RotationController);
		
		ActualForm = form;
	}
}

