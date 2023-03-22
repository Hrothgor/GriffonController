// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/ObjectMacros.h"

/**
 * 
 */

UENUM(BlueprintType)
enum ECustomMovementMode
{
	CMOVE_Climbing      UMETA(DisplayName = "Climbing"),
	CMOVE_MAX			UMETA(Hidden),
};

UENUM(BlueprintType)
enum EShapeShiftForm
{
	SSForm_Druid			UMETA(DisplayName = "Druid"),
	SSForm_Griffon			UMETA(DisplayName = "Griffon"),
	SSForm_Werewolf			UMETA(DisplayName = "Werewolf"),
	SSForm_SeaCreature		UMETA(DisplayName = "SeaCreature"),
	SSForm_MAX				UMETA(Hidden),
};