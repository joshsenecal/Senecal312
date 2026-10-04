// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SurvivalSaveGame.generated.h"

// One saved world actor (resource node, building part, enemy, ...).
USTRUCT()
struct FActorSaveData
{
	GENERATED_BODY()

	// Used to match level-placed actors when loading.
	UPROPERTY()
	FName ActorName;

	// Used to respawn actors that were created at runtime (e.g. placed building parts).
	UPROPERTY()
	FSoftClassPath ActorClass;

	UPROPERTY()
	FTransform Transform;

	// The actor's SaveGame-flagged variables.
	UPROPERTY()
	TArray<uint8> ByteData;
};

USTRUCT()
struct FPlayerSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHasData = false;

	UPROPERTY()
	FTransform Transform;

	// Where the camera was looking.
	UPROPERTY()
	FRotator ControlRotation = FRotator::ZeroRotator;

	// The player's SaveGame-flagged variables (stats, resources, building supplies...).
	UPROPERTY()
	TArray<uint8> ByteData;
};

UCLASS()
class SENECAL312_API USurvivalSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// Bump this if the save format changes in a way old saves can't handle.
	UPROPERTY()
	int32 SaveVersion = 1;

	UPROPERTY()
	FString SlotName;

	UPROPERTY()
	FString LevelName;

	UPROPERTY()
	FDateTime SaveTime;

	UPROPERTY()
	FPlayerSaveData Player;

	UPROPERTY()
	TArray<FActorSaveData> Actors;
};