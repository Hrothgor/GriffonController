// Fill out your copyright notice in the Description page of Project Settings.


#include "ShapeShiftMenu.h"

#include "DruidControllerCharacter.h"
#include "Kismet/GameplayStatics.h"

void UShapeShiftMenu::NativeConstruct()
{
	Super::NativeConstruct();

	SetVisibility(ESlateVisibility::Hidden);
}


void UShapeShiftMenu::Show()
{
	SetVisibility(ESlateVisibility::Visible);

	APlayerController *Controller = Cast<APlayerController>(GetOwningPlayer());
	
	if (Controller)
	{
		Controller->SetShowMouseCursor(true);
	}
}

void UShapeShiftMenu::ChooseShapeShiftForm(EShapeShiftForm form)
{
	SetVisibility(ESlateVisibility::Hidden);

	APlayerController *Controller = Cast<APlayerController>(GetOwningPlayer());

	if (Controller)
	{
		Controller->SetShowMouseCursor(false);

		ADruidControllerCharacter *Character = Controller->GetPawn<ADruidControllerCharacter>();
		if (Character)
		{
			Character->ShapeShift(form);
		}
	}
}
