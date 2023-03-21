// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnumFile.h"
#include "Blueprint/UserWidget.h"
#include "ShapeShiftMenu.generated.h"

/**
 * 
 */
UCLASS()
class GRIFFONCONTROLLER_API UShapeShiftMenu : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;

public:
	void Show();
	UFUNCTION(BlueprintCallable)
	void ChooseShapeShiftForm(EShapeShiftForm form);
};
