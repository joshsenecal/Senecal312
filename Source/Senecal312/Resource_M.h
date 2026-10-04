// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "SaveableActor.h"
#include "Resource_M.generated.h"

UCLASS()
class SENECAL312_API AResource_M : public AActor, public ISaveableActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AResource_M();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// ISaveableActor
	virtual void OnBeforeSave_Implementation() override;
	virtual void OnAfterLoad_Implementation() override;

	// Removes up to resourceAmount from this node and returns how much was actually gathered.
	// Returns 0 if the node is depleted. Depletes the node when it hits 0.
	UFUNCTION(BlueprintCallable, Category = "Resource")
	int Gather();

	UFUNCTION(BlueprintPure, Category = "Resource")
	bool IsDepleted() const { return bIsDepleted; }

	UPROPERTY(EditAnywhere, Category = "Resource")
	FString resourceName = "Wood";

	// Amount given to the player per gather.
	UPROPERTY(EditAnywhere, Category = "Resource")
	int resourceAmount = 5;

	// Maximum amount this node holds. It refills to this when it respawns.
	UPROPERTY(EditAnywhere, Category = "Resource")
	int totalResource = 100;

	// Amount currently left in the node.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category = "Resource")
	int currentResource = 0;

	// Seconds after depletion before the node comes back.
	UPROPERTY(EditAnywhere, Category = "Resource", meta = (ClampMin = "0.1"))
	float respawnTime = 30.0f;

	UPROPERTY()
	FText tempText;

	UPROPERTY(EditAnywhere)
	UTextRenderComponent* ResourceNameTxt;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* Mesh;

private:
	void Deplete();
	void Respawn();

	// Shows/hides the node and turns its collision on/off.
	void SetNodeVisible(bool bVisible);

	UPROPERTY(VisibleAnywhere, SaveGame, Category = "Resource")
	bool bIsDepleted = false;

	// Respawn time left when the game was saved, so the countdown resumes after loading.
	UPROPERTY(SaveGame)
	float SavedRespawnRemaining = 0.0f;

	FTimerHandle RespawnTimerHandle;
};