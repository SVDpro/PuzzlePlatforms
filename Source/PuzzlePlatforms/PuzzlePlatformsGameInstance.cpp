// Fill out your copyright notice in the Description page of Project Settings.


#include "PuzzlePlatformsGameInstance.h"

#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"

#include "MenuSystem/MainMenu.h"
#include "MenuSystem/MenuWidget.h"

#include "PlatformTrigger.h"

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
	UE_LOG(LogTemp, Warning, TEXT("Found class %s"), *MenuClass->GetName());
}

void UPuzzlePlatformsGameInstance::LoadMenu()
{
	Menu = CreateWidget<UMenuWidget>(this, MenuClass);

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

void UPuzzlePlatformsGameInstance::Host()
{
	if (Menu != nullptr)
	{
		Menu->Teardown();
	}

	if (UEngine* Engine = GetEngine())
		Engine->AddOnScreenDebugMessage(0, 2.0f, FColor::Green, TEXT("Hosting"));

	if (UWorld* World = GetWorld())
		World->ServerTravel("/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap?listen");
}

void UPuzzlePlatformsGameInstance::Join(const FString& Address)
{
	if (Menu != nullptr)
	{
		Menu->Teardown();
	}

	if (UEngine* Engine = GetEngine())
		Engine->AddOnScreenDebugMessage(0, 2.0f, FColor::Green, FString::Printf(TEXT("Joining %s"), *Address));

	if (APlayerController* PlayerController = GetFirstLocalPlayerController())
		PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
}

void UPuzzlePlatformsGameInstance::LoadMainMenu()
{
	if (APlayerController* PlayerController = GetFirstLocalPlayerController())
		PlayerController->ClientTravel("/Game/ThirdPersonCPP/Maps/MainMenu", ETravelType::TRAVEL_Absolute);
}
