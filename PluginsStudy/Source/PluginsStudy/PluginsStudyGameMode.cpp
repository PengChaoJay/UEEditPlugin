// Copyright Epic Games, Inc. All Rights Reserved.

#include "PluginsStudyGameMode.h"
#include "PluginsStudyCharacter.h"
#include "UObject/ConstructorHelpers.h"

APluginsStudyGameMode::APluginsStudyGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
