// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SaveableActor.generated.h"

/*
 * Add this interface to any actor that should be saved (C++ or Blueprint).
 * The save system stores the actor's class, name, and transform, plus every
 * variable marked "SaveGame" (UPROPERTY(SaveGame) in C++, or the SaveGame
 * checkbox under Advanced in a Blueprint variable's details).
 */
UINTERFACE(MinimalAPI, Blueprintable)
class USaveableActor : public UInterface
{
	GENERATED_BODY()
};

class SENECAL312_API ISaveableActor
{
	GENERATED_BODY()

public:
	// Called right before this actor's data is written. Use it to copy runtime-only state
	// (like timer progress) into SaveGame variables.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save")
	void OnBeforeSave();

	// Called right after this actor's SaveGame variables are restored. Use it to re-apply
	// visuals, timers, etc. based on the loaded values.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save")
	void OnAfterLoad();
};