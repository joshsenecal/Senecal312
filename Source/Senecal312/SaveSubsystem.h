// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "SaveSubsystem.generated.h"

class USurvivalSaveGame;
class APlayerChar;
class FViewport;

// Summary of one save slot, for showing in menus.
USTRUCT(BlueprintType)
struct FSaveSlotInfo
{
	GENERATED_BODY()

	// Internal slot name: pass this to LoadFromSlot / SaveToSlot / DeleteSlot.
	UPROPERTY(BlueprintReadOnly, Category = "Save")
	FString SlotName;

	// Friendly name for the UI ("Autosave", "Quicksave", "Slot 1"...).
	UPROPERTY(BlueprintReadOnly, Category = "Save")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "Save")
	bool bExists = false;

	UPROPERTY(BlueprintReadOnly, Category = "Save")
	FDateTime SaveTime;

	// SaveTime formatted for display, or "Empty".
	UPROPERTY(BlueprintReadOnly, Category = "Save")
	FText SaveTimeText;

	UPROPERTY(BlueprintReadOnly, Category = "Save")
	FString LevelName;
};

/*
 * Owns saving and loading for the whole game session (survives level changes).
 * Get it anywhere in Blueprints with the "Save Subsystem" getter node.
 */
UCLASS()
class SENECAL312_API USaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	static const FString QuicksaveSlot;
	static const FString AutosaveSlot;
	static constexpr int32 NumManualSlots = 3;

	// Saves the current game to the given slot. Returns false if saving isn't possible right now.
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool SaveToSlot(const FString& SlotName);

	// Reopens the saved level and restores the saved state once it loads.
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool LoadFromSlot(const FString& SlotName);

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool QuickSave();

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool Autosave();

	// Loads whichever existing save is newest (used by F9 and "Continue").
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool LoadMostRecent();

	UFUNCTION(BlueprintCallable, Category = "Save")
	void DeleteSlot(const FString& SlotName);

	// Opens a level fresh, with no save applied.
	UFUNCTION(BlueprintCallable, Category = "Save")
	void StartNewGame(FName LevelName);

	// Autosaves, then quits the game.
	UFUNCTION(BlueprintCallable, Category = "Save")
	void SaveAndQuit();

	// Autosaves, then opens the given level (e.g. the main menu).
	UFUNCTION(BlueprintCallable, Category = "Save")
	void SaveAndOpenLevel(FName LevelName);

	// Slot name for manual slot 1..NumManualSlots ("Slot1", "Slot2", ...).
	UFUNCTION(BlueprintPure, Category = "Save")
	FString GetManualSlotName(int32 SlotNumber) const;

	UFUNCTION(BlueprintCallable, Category = "Save")
	FSaveSlotInfo GetSlotInfo(const FString& SlotName) const;

	// Autosave, Quicksave, then the manual slots, in that order.
	UFUNCTION(BlueprintCallable, Category = "Save")
	TArray<FSaveSlotInfo> GetAllSlotInfo() const;

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool HasAnySave() const;

	// Writes / reads every SaveGame-flagged variable on an object.
	static void WriteSaveProperties(UObject* Object, TArray<uint8>& OutBytes);
	static void ReadSaveProperties(UObject* Object, const TArray<uint8>& InBytes);

	// Called by USaveWorldHelper whenever a gameplay world begins play.
	void HandleWorldBeginPlay(UWorld& World);

	// Seconds between autosaves.
	float AutosaveInterval = 300.0f;

private:
	bool CanSave() const;
	UWorld* GetGameWorld() const;
	APlayerChar* GetPlayer() const;
	USurvivalSaveGame* LoadSaveObject(const FString& SlotName) const;
	FText GetSlotDisplayName(const FString& SlotName) const;

	void ApplyPendingLoad();
	void AutosaveTick();
	void HandleCloseRequested(FViewport* Viewport);

	// Save waiting to be applied after its level finishes loading.
	UPROPERTY()
	TObjectPtr<USurvivalSaveGame> PendingLoad;

	FTimerHandle AutosaveTimerHandle;

	TWeakObjectPtr<class UGameViewportClient> BoundViewport;
	FDelegateHandle CloseRequestedHandle;

	bool bIsApplyingLoad = false;
};

/*
 * Tiny helper that exists in every gameplay world and tells the save subsystem when
 * the world starts, so it can apply a pending load and start the autosave timer.
 * Nothing to set up; Unreal creates it automatically.
 */
UCLASS()
class SENECAL312_API USaveWorldHelper : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
};