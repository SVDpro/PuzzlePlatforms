// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MenuSystem/MenuInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "PuzzlePlatformsGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class PUZZLEPLATFORMS_API UPuzzlePlatformsGameInstance : public UGameInstance, public IMenuInterface
{
	GENERATED_BODY()
	
public:
	UPuzzlePlatformsGameInstance(const FObjectInitializer & ObjectInitializer);

	virtual void Init() override;

	UFUNCTION(BlueprintCallable)
	void LoadMenu();

	UFUNCTION(BlueprintCallable)
	void InGameLoadMenu();

	UFUNCTION(Exec)
	virtual void Host(const FString& ServerName) override;

	UFUNCTION(Exec)
	virtual void Join(uint32 Index) override;

	virtual void LoadMainMenu() override;

	virtual void RefreshServerList() override;

    void StartSession();

private:
	TSubclassOf<UUserWidget> MenuClass;
	TSubclassOf<UUserWidget> InGameMenuClass;

	UPROPERTY()
	class UMainMenu* Menu;

	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	
	void OnCreateSessionComplete(FName SessionName, bool Success);
	void OnDestroySessionComplete(FName SessionName, bool Success);
	void OnFindSessionsComplete(bool Success);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
    void OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& Error);

    FString DesiredServerName;

	void CreateSession();
};