// Fill out your copyright notice in the Description page of Project Settings.


#include "PuzzlePlatformsGameInstance.h"

#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

#include "MenuSystem/MainMenu.h"
#include "MenuSystem/MenuWidget.h"

const static FName SERVER_NAME_SETTINGS_KEY = TEXT("ServerName");

UPuzzlePlatformsGameInstance::UPuzzlePlatformsGameInstance(const FObjectInitializer & ObjectInitializer)
{
	ConstructorHelpers::FClassFinder<UUserWidget> MenuBPClass(TEXT("/Game/MenuSystem/WBP_MainMenu"));

	if (MenuBPClass.Class != nullptr)
	{
		MenuClass = MenuBPClass.Class;
	}

	ConstructorHelpers::FClassFinder<UUserWidget> InGameMenuBPClass(TEXT("/Game/MenuSystem/WBP_InGameMenu"));

	if (InGameMenuBPClass.Class != nullptr)
	{
		InGameMenuClass = InGameMenuBPClass.Class;
	}
}

void UPuzzlePlatformsGameInstance::Init()
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();

	if (OnlineSubsystem != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Found subsystem %s"), *OnlineSubsystem->GetSubsystemName().ToString());
		SessionInterface = OnlineSubsystem->GetSessionInterface();

		if (SessionInterface.IsValid())
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UPuzzlePlatformsGameInstance::OnCreateSessionComplete);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UPuzzlePlatformsGameInstance::OnDestroySessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UPuzzlePlatformsGameInstance::OnFindSessionsComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UPuzzlePlatformsGameInstance::OnJoinSessionComplete);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Found no subsystem"));
	}

    if (GEngine != nullptr)
    {
        GEngine->OnNetworkFailure().AddUObject(this, &UPuzzlePlatformsGameInstance::OnNetworkFailure);
    }
}

void UPuzzlePlatformsGameInstance::LoadMenu()
{
	Menu = CreateWidget<UMainMenu>(this, MenuClass);

	if (Menu != nullptr)
	{
		Menu->Setup();
		Menu->SetMenuInterface(this);
	}
}

void UPuzzlePlatformsGameInstance::InGameLoadMenu()
{
	UMenuWidget* InGameMenu = CreateWidget<UMenuWidget>(this, InGameMenuClass);

	if (InGameMenu != nullptr)
	{
		InGameMenu->Setup();
		InGameMenu->SetMenuInterface(this);
	}
}

void UPuzzlePlatformsGameInstance::Host(const FString& ServerName)
{
    DesiredServerName = ServerName;

	if (SessionInterface.IsValid())
	{
		auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
		
		if (ExistingSession != nullptr)
		{
			SessionInterface->DestroySession(NAME_GameSession);
		}
		else
		{
			CreateSession();
		}
	}
}

void UPuzzlePlatformsGameInstance::Join(uint32 Index)
{
	if (! SessionInterface.IsValid())
		return;

	if (! SessionSearch.IsValid())
		return;
	
	if (Menu != nullptr)
	{
		Menu->Teardown();
	}

	SessionInterface->JoinSession(0, NAME_GameSession, SessionSearch->SearchResults[Index]);
}

void UPuzzlePlatformsGameInstance::LoadMainMenu()
{
	if (APlayerController* PlayerController = GetFirstLocalPlayerController())
		PlayerController->ClientTravel("/Game/ThirdPersonCPP/Maps/MainMenu", ETravelType::TRAVEL_Absolute);
}

void UPuzzlePlatformsGameInstance::RefreshServerList()
{
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
			
	if (SessionSearch.IsValid())
	{
        SessionSearch->MaxSearchResults = 100;
        SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

		UE_LOG(LogTemp, Warning, TEXT("Starting Find Session"));
		SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
	}
}

void UPuzzlePlatformsGameInstance::StartSession()
{
    if (SessionInterface.IsValid())
    {
        SessionInterface->StartSession(NAME_GameSession);
    }
}

void UPuzzlePlatformsGameInstance::OnCreateSessionComplete(FName SessionName, bool Success)
{
	if (! Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not create session"));
		return;
	}
	
	if (Menu != nullptr)
	{
		Menu->Teardown();
	}

	if (UEngine* Engine = GetEngine())
		Engine->AddOnScreenDebugMessage(0, 2.0f, FColor::Green, TEXT("Hosting"));

	if (UWorld* World = GetWorld())
		World->ServerTravel("/Game/ThirdPersonCPP/Maps/LobbyMap?listen"); // ThirdPersonExampleMap
}

void UPuzzlePlatformsGameInstance::OnDestroySessionComplete(FName SessionName, bool Success)
{
	if (Success)
	{
		CreateSession();
	}
}

void UPuzzlePlatformsGameInstance::OnFindSessionsComplete(bool Success)
{
	if (Success && SessionSearch.IsValid() && Menu != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Finished Find Sessions"));

		TArray<FServerData> ServerNames;

		for (const auto& Result : SessionSearch->SearchResults)
		{
			UE_LOG(LogTemp, Warning, TEXT("Found session: %s"), *Result.GetSessionIdStr());

            FServerData Data;
            //Data.Name = Result.GetSessionIdStr();
            Data.HostUsername = Result.Session.OwningUserName;
            Data.MaxPlayers = Result.Session.SessionSettings.NumPublicConnections;
            Data.CurrentPlayers = Data.MaxPlayers - Result.Session.NumOpenPublicConnections;

            FString ServerName;
            if (Result.Session.SessionSettings.Get(SERVER_NAME_SETTINGS_KEY, ServerName))
            {
                Data.Name = ServerName;
            }
            else
            {
                Data.Name = "Could not find name";
            }

			ServerNames.Add(Data);
		}

		Menu->SetServerList(ServerNames);
	}
}

void UPuzzlePlatformsGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (SessionInterface.IsValid())
	{
		if (Result == EOnJoinSessionCompleteResult::Type::Success)
		{
			FString Address;
			
			if (SessionInterface->GetResolvedConnectString(SessionName, Address))
			{
				if (UEngine* Engine = GetEngine())
					Engine->AddOnScreenDebugMessage(0, 2.0f, FColor::Green, FString::Printf(TEXT("Joining %s"), *Address));
				
				if (APlayerController* PlayerController = GetFirstLocalPlayerController())
					PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Could not get connect string"));
			}
		}
	}
}

void UPuzzlePlatformsGameInstance::OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& Error)
{
    LoadMainMenu();
}

void UPuzzlePlatformsGameInstance::CreateSession()
{
	if (SessionInterface.IsValid())
	{
		FOnlineSessionSettings SessionSettings;

        SessionSettings.bIsLANMatch = (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL");
		SessionSettings.NumPublicConnections = 5;
		SessionSettings.bShouldAdvertise = true;
        SessionSettings.bUsesPresence = true;
        SessionSettings.Set(SERVER_NAME_SETTINGS_KEY, DesiredServerName, EOnlineDataAdvertisementType::Type::ViaOnlineServiceAndPing);

		SessionInterface->CreateSession(0, NAME_GameSession, SessionSettings);
	}
}