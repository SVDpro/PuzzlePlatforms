// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuWidget.h"

void UMenuWidget::SetMenuInterface(IMenuInterface* Interface)
{
	MenuInterface = Interface;
}

void UMenuWidget::Setup()
{
	// bIsFocusable = true;
	AddToViewport();

	UWorld* World = GetWorld();
	if (World == nullptr) return;
	
	if (APlayerController* PlayerController = World->GetFirstPlayerController())
	{
		FInputModeUIOnly InputModeData;
		InputModeData.SetWidgetToFocus(TakeWidget());
		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

		PlayerController->SetInputMode(InputModeData);
		PlayerController->bShowMouseCursor = true;
	}
}

void UMenuWidget::Teardown()
{
	RemoveFromViewport();

	UWorld* World = GetWorld();
	if (World == nullptr) return;
	
	if (APlayerController* PlayerController = World->GetFirstPlayerController())
	{
		FInputModeGameOnly InputModeData;
		PlayerController->SetInputMode(InputModeData);
		PlayerController->bShowMouseCursor = false;
	}
}
