// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenu.h"


#include "ServerRow.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Components/EditableText.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"

UMainMenu::UMainMenu()
{
	ConstructorHelpers::FClassFinder<UUserWidget> ServerRowBPClass(TEXT("/Game/MenuSystem/WBP_ServerRow"));

	if (ServerRowBPClass.Class != nullptr)
	{
		ServerRowClass = ServerRowBPClass.Class;
	}
}

bool UMainMenu::Initialize()
{
	bool Success = Super::Initialize();

	if (!Success)
		return false;

	if (HostButton == nullptr)
		return false;

	HostButton->OnClicked.AddDynamic(this, &UMainMenu::OpenHostMenu);

    if (CancelHostMenuButton == nullptr)
        return false;

    CancelHostMenuButton->OnClicked.AddDynamic(this, &UMainMenu::OpenMainMenu);

    if (ConfirmHostMenuButton == nullptr)
        return false;

    ConfirmHostMenuButton->OnClicked.AddDynamic(this, &UMainMenu::HostServer);

	if (JoinButton == nullptr)
		return false;

	JoinButton->OnClicked.AddDynamic(this, &UMainMenu::OpenJoinMenu);

	if (CancelButton == nullptr)
		return false;

	CancelButton->OnClicked.AddDynamic(this, &UMainMenu::OpenMainMenu);

	if (JoinServerButton == nullptr)
		return false;

	JoinServerButton->OnClicked.AddDynamic(this, &UMainMenu::JoinServer);

	if (QuitButton == nullptr)
		return false;

	QuitButton->OnClicked.AddDynamic(this, &UMainMenu::QuitPressed);

	return true;
}

void UMainMenu::OpenHostMenu()
{
    if (MenuSwitcher == nullptr)
        return;

    if (HostMenu == nullptr)
        return;

    MenuSwitcher->SetActiveWidget(HostMenu);
}

void UMainMenu::HostServer()
{
	if (MenuInterface != nullptr)
	{
        FString ServerName = ServerHostName->Text.ToString();
		MenuInterface->Host(ServerName);
	}
}

void UMainMenu::SetServerList(TArray<FServerData> ServerNames)
{
	if (ServerList != nullptr)
	{	
		UWorld* World = GetWorld();

		if (World == nullptr)
			return;

		ServerList->ClearChildren();

		uint32 i = 0;
		for (const FServerData& ServerData : ServerNames)
		{
			UServerRow* ServerRow = CreateWidget<UServerRow>(World, ServerRowClass);

			if (ServerRow != nullptr)
			{
				ServerRow->ServerName->SetText(FText::FromString(ServerData.Name));
				ServerRow->HostUser->SetText(FText::FromString(ServerData.HostUsername));

                FString FractionText = FString::Printf(TEXT("%d/%d"), ServerData.CurrentPlayers, ServerData.MaxPlayers);

				ServerRow->ConnectionFraction->SetText(FText::FromString(FractionText));
				ServerRow->Setup(this, i);
				++i;

				ServerList->AddChild(ServerRow);
			}
		}

	}
}

void UMainMenu::SelectIndex(uint32 Index)
{
	SelectedIndex = Index;
    UpdateChildren();
}

void UMainMenu::UpdateChildren()
{
    if (ServerList == nullptr)
        return;

    for (int32 Index = 0; Index < ServerList->GetChildrenCount(); ++Index)
    {
        if (auto* ServerRow = Cast<UServerRow>(ServerList->GetChildAt(Index)))
        {
            ServerRow->Selected = (SelectedIndex.IsSet() && SelectedIndex.GetValue() == Index);
        }
    }
}

void UMainMenu::JoinServer()
{
	if (SelectedIndex.IsSet() && MenuInterface != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Selected index: %d"), SelectedIndex.GetValue());
		MenuInterface->Join(SelectedIndex.GetValue());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Selected index not set"));
	}
}

void UMainMenu::OpenJoinMenu()
{
	if (MenuSwitcher == nullptr)
		return;

	if (JoinMenu == nullptr)
		return;

	MenuSwitcher->SetActiveWidget(JoinMenu);

	if (MenuInterface != nullptr)
	{
		MenuInterface->RefreshServerList();
	}
}

void UMainMenu::OpenMainMenu()
{
	if (MenuSwitcher == nullptr)
		return;

	if (MainMenu == nullptr)
		return;

	MenuSwitcher->SetActiveWidget(MainMenu);
}

void UMainMenu::QuitPressed()
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	if (APlayerController* PlayerController = World->GetFirstPlayerController())
	{
		PlayerController->ConsoleCommand("quit");
	}
}
