// Copyright Epic Games, Inc. All Rights Reserved.

#include "GriffonControllerGameMode.h"
#include "GriffonControllerCharacter.h"
#include "UObject/ConstructorHelpers.h"

AGriffonControllerGameMode::AGriffonControllerGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
