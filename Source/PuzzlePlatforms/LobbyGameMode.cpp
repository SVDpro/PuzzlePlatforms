// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameMapsSettings.h"
#include "TimerManager.h"
#include "PuzzlePlatformsGameInstance.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    ++NumberOfPlayers;

    if (NumberOfPlayers >= 2)
    {
        GetWorldTimerManager().SetTimer(GameStartTimer, this, &ALobbyGameMode::StartGame, 10.0f);
    }
}

void ALobbyGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);

    --NumberOfPlayers;
}

void ALobbyGameMode::StartGame()
{
    if (auto* GameInstance = Cast<UPuzzlePlatformsGameInstance>(GetGameInstance()))
    {
        GameInstance->StartSession();

        if (UWorld* World = GetWorld())
        {
            bUseSeamlessTravel = true;
            World->ServerTravel("/Game/ThirdPersonCPP/Maps/GameMap?listen");
        }
    }
}