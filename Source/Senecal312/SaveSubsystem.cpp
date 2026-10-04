// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveSubsystem.h"
#include "SurvivalSaveGame.h"
#include "SaveableActor.h"
#include "PlayerChar.h"
#include "BuildingPart.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

const FString USaveSubsystem::QuicksaveSlot = TEXT("Quicksave");
const FString USaveSubsystem::AutosaveSlot = TEXT("Autosave");

static void ShowSaveMessage(const FString& Message, const FColor& Color = FColor::Green)
{
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, Color, Message);
	}
}

// - - LIFETIME - -

void USaveSubsystem::Deinitialize()
{
	if (BoundViewport.IsValid() && CloseRequestedHandle.IsValid()) {
		BoundViewport->OnCloseRequested().Remove(CloseRequestedHandle);
	}

	Super::Deinitialize();
}

void USaveSubsystem::HandleWorldBeginPlay(UWorld& World)
{
	// Autosave when the game window is closed. Bound once per session.
	if (!CloseRequestedHandle.IsValid()) {
		if (UGameViewportClient* Viewport = GetGameInstance()->GetGameViewportClient()) {
			BoundViewport = Viewport;
			CloseRequestedHandle = Viewport->OnCloseRequested().AddUObject(this, &USaveSubsystem::HandleCloseRequested);
		}
	}

	// Autosave timer lives in this world, so it resets on every level load and pauses with the game.
	World.GetTimerManager().SetTimer(AutosaveTimerHandle, FTimerDelegate::CreateUObject(this, &USaveSubsystem::AutosaveTick), AutosaveInterval, true);

	// Wait one frame so every actor (and the player pawn) has finished BeginPlay before restoring state.
	if (PendingLoad) {
		World.GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &USaveSubsystem::ApplyPendingLoad));
	}
}

// - - SAVING - -

bool USaveSubsystem::SaveToSlot(const FString& SlotName)
{
	if (!CanSave()) {
		return false;
	}

	UWorld* World = GetGameWorld();
	APlayerChar* Player = GetPlayer();

	USurvivalSaveGame* Data = Cast<USurvivalSaveGame>(UGameplayStatics::CreateSaveGameObject(USurvivalSaveGame::StaticClass()));
	if (!Data) {
		return false;
	}

	Data->SlotName = SlotName;
	Data->LevelName = UGameplayStatics::GetCurrentLevelName(World, true);
	Data->SaveTime = FDateTime::Now();

	Player->WriteSaveData(Data->Player);

	for (TActorIterator<AActor> It(World); It; ++It) {
		AActor* Actor = *It;

		if (!IsValid(Actor) || Actor->IsActorBeingDestroyed() || !Actor->Implements<USaveableActor>()) {
			continue;
		}

		// Skip the preview part that's still following the camera.
		if (const ABuildingPart* Part = Cast<ABuildingPart>(Actor)) {
			if (!Part->bIsPlaced) {
				continue;
			}
		}

		ISaveableActor::Execute_OnBeforeSave(Actor);

		FActorSaveData Record;
		Record.ActorName = Actor->GetFName();
		Record.ActorClass = FSoftClassPath(Actor->GetClass());
		Record.Transform = Actor->GetActorTransform();
		WriteSaveProperties(Actor, Record.ByteData);

		Data->Actors.Add(MoveTemp(Record));
	}

	const bool bSaved = UGameplayStatics::SaveGameToSlot(Data, SlotName, 0);

	if (bSaved) {
		ShowSaveMessage(FString::Printf(TEXT("Game saved (%s)"), *GetSlotDisplayName(SlotName).ToString()));
	}
	else {
		ShowSaveMessage(TEXT("Save failed"), FColor::Red);
	}

	return bSaved;
}

bool USaveSubsystem::QuickSave()
{
	return SaveToSlot(QuicksaveSlot);
}

bool USaveSubsystem::Autosave()
{
	return SaveToSlot(AutosaveSlot);
}

void USaveSubsystem::AutosaveTick()
{
	// CanSave() quietly skips this on the main menu, while dead, or mid-load.
	Autosave();
}

void USaveSubsystem::HandleCloseRequested(FViewport* Viewport)
{
	Autosave();
}

void USaveSubsystem::SaveAndQuit()
{
	Autosave();

	UWorld* World = GetGameWorld();
	APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	UKismetSystemLibrary::QuitGame(World, PC, EQuitPreference::Quit, false);
}

void USaveSubsystem::SaveAndOpenLevel(FName LevelName)
{
	Autosave();

	PendingLoad = nullptr;
	UGameplayStatics::OpenLevel(GetGameWorld(), LevelName);
}

bool USaveSubsystem::CanSave() const
{
	if (bIsApplyingLoad || PendingLoad) {
		return false;
	}

	const APlayerChar* Player = GetPlayer();

	// No player means we're on a menu level. Don't save over a run once the player is dead.
	return Player != nullptr && Player->Health > 0.0f;
}

// - - LOADING - -

bool USaveSubsystem::LoadFromSlot(const FString& SlotName)
{
	USurvivalSaveGame* Data = LoadSaveObject(SlotName);

	if (!Data || Data->LevelName.IsEmpty()) {
		ShowSaveMessage(TEXT("No save in that slot"), FColor::Red);
		return false;
	}

	// Reopen the level from scratch; ApplyPendingLoad restores the state once it's running.
	PendingLoad = Data;
	UGameplayStatics::OpenLevel(GetGameWorld(), FName(*Data->LevelName));

	return true;
}

bool USaveSubsystem::LoadMostRecent()
{
	const FSaveSlotInfo* Newest = nullptr;
	const TArray<FSaveSlotInfo> Slots = GetAllSlotInfo();

	for (const FSaveSlotInfo& Slot : Slots) {
		if (Slot.bExists && (!Newest || Slot.SaveTime > Newest->SaveTime)) {
			Newest = &Slot;
		}
	}

	if (!Newest) {
		ShowSaveMessage(TEXT("No saves found"), FColor::Red);
		return false;
	}

	return LoadFromSlot(Newest->SlotName);
}

void USaveSubsystem::ApplyPendingLoad()
{
	USurvivalSaveGame* Data = PendingLoad;
	PendingLoad = nullptr;

	UWorld* World = GetGameWorld();
	if (!Data || !World) {
		return;
	}

	bIsApplyingLoad = true;

	// Every saveable actor currently in the fresh level, by name.
	TMap<FName, AActor*> ExistingActors;
	for (TActorIterator<AActor> It(World); It; ++It) {
		AActor* Actor = *It;
		if (IsValid(Actor) && Actor->Implements<USaveableActor>()) {
			ExistingActors.Add(Actor->GetFName(), Actor);
		}
	}

	TSet<AActor*> RestoredActors;

	for (const FActorSaveData& Record : Data->Actors) {
		UClass* ActorClass = Record.ActorClass.TryLoadClass<AActor>();
		if (!ActorClass) {
			continue;
		}

		AActor* Target = nullptr;

		// Level-placed actors (resource nodes, placed enemies) are matched by name.
		if (AActor** Found = ExistingActors.Find(Record.ActorName)) {
			if (*Found && (*Found)->GetClass() == ActorClass && !RestoredActors.Contains(*Found)) {
				Target = *Found;
			}
		}

		if (Target) {
			Target->SetActorTransform(Record.Transform, false, nullptr, ETeleportType::TeleportPhysics);
		}
		else {
			// Runtime-spawned actors (building parts, spawned enemies) are recreated.
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			Target = World->SpawnActor<AActor>(ActorClass, Record.Transform, SpawnParams);
			if (!Target) {
				continue;
			}

			// Spawned AI needs a controller to think.
			if (APawn* Pawn = Cast<APawn>(Target)) {
				if (!Pawn->GetController()) {
					Pawn->SpawnDefaultController();
				}
			}
		}

		RestoredActors.Add(Target);

		ReadSaveProperties(Target, Record.ByteData);
		ISaveableActor::Execute_OnAfterLoad(Target);
	}

	// Saveable actors that weren't in the save were gone at save time (e.g. killed enemies).
	for (const TPair<FName, AActor*>& Pair : ExistingActors) {
		if (IsValid(Pair.Value) && !RestoredActors.Contains(Pair.Value)) {
			Pair.Value->Destroy();
		}
	}

	if (Data->Player.bHasData) {
		if (APlayerChar* Player = GetPlayer()) {
			Player->ApplySaveData(Data->Player);
		}
	}

	bIsApplyingLoad = false;

	ShowSaveMessage(FString::Printf(TEXT("Game loaded (%s)"), *GetSlotDisplayName(Data->SlotName).ToString()));
}

void USaveSubsystem::StartNewGame(FName LevelName)
{
	PendingLoad = nullptr;
	UGameplayStatics::OpenLevel(GetGameWorld(), LevelName);
}

// - - SLOTS - -

FString USaveSubsystem::GetManualSlotName(int32 SlotNumber) const
{
	return FString::Printf(TEXT("Slot%d"), FMath::Clamp(SlotNumber, 1, NumManualSlots));
}

FText USaveSubsystem::GetSlotDisplayName(const FString& SlotName) const
{
	if (SlotName.StartsWith(TEXT("Slot"))) {
		return FText::FromString(FString::Printf(TEXT("Slot %s"), *SlotName.RightChop(4)));
	}

	return FText::FromString(SlotName);
}

FSaveSlotInfo USaveSubsystem::GetSlotInfo(const FString& SlotName) const
{
	FSaveSlotInfo Info;
	Info.SlotName = SlotName;
	Info.DisplayName = GetSlotDisplayName(SlotName);
	Info.SaveTimeText = FText::FromString(TEXT("Empty"));

	if (const USurvivalSaveGame* Data = LoadSaveObject(SlotName)) {
		Info.bExists = true;
		Info.SaveTime = Data->SaveTime;
		Info.SaveTimeText = FText::FromString(Data->SaveTime.ToString(TEXT("%Y-%m-%d  %H:%M")));
		Info.LevelName = Data->LevelName;
	}

	return Info;
}

TArray<FSaveSlotInfo> USaveSubsystem::GetAllSlotInfo() const
{
	TArray<FSaveSlotInfo> Slots;
	Slots.Add(GetSlotInfo(AutosaveSlot));
	Slots.Add(GetSlotInfo(QuicksaveSlot));

	for (int32 i = 1; i <= NumManualSlots; ++i) {
		Slots.Add(GetSlotInfo(GetManualSlotName(i)));
	}

	return Slots;
}

bool USaveSubsystem::HasAnySave() const
{
	if (UGameplayStatics::DoesSaveGameExist(AutosaveSlot, 0) || UGameplayStatics::DoesSaveGameExist(QuicksaveSlot, 0)) {
		return true;
	}

	for (int32 i = 1; i <= NumManualSlots; ++i) {
		if (UGameplayStatics::DoesSaveGameExist(GetManualSlotName(i), 0)) {
			return true;
		}
	}

	return false;
}

void USaveSubsystem::DeleteSlot(const FString& SlotName)
{
	UGameplayStatics::DeleteGameInSlot(SlotName, 0);
}

USurvivalSaveGame* USaveSubsystem::LoadSaveObject(const FString& SlotName) const
{
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0)) {
		return nullptr;
	}

	return Cast<USurvivalSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
}

// - - HELPERS - -

UWorld* USaveSubsystem::GetGameWorld() const
{
	return GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
}

APlayerChar* USaveSubsystem::GetPlayer() const
{
	UWorld* World = GetGameWorld();
	return World ? Cast<APlayerChar>(UGameplayStatics::GetPlayerPawn(World, 0)) : nullptr;
}

void USaveSubsystem::WriteSaveProperties(UObject* Object, TArray<uint8>& OutBytes)
{
	FMemoryWriter MemWriter(OutBytes, true);
	FObjectAndNameAsStringProxyArchive Archive(MemWriter, false);
	Archive.ArIsSaveGame = true; // only variables flagged SaveGame
	Object->Serialize(Archive);
}

void USaveSubsystem::ReadSaveProperties(UObject* Object, const TArray<uint8>& InBytes)
{
	FMemoryReader MemReader(InBytes, true);
	FObjectAndNameAsStringProxyArchive Archive(MemReader, true);
	Archive.ArIsSaveGame = true;
	Object->Serialize(Archive);
}

// - - WORLD HELPER - -

void USaveWorldHelper::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	if (UGameInstance* GameInstance = InWorld.GetGameInstance()) {
		if (USaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<USaveSubsystem>()) {
			SaveSubsystem->HandleWorldBeginPlay(InWorld);
		}
	}
}

bool USaveWorldHelper::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}