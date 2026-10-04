// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Resource_M.h"
#include "Kismet/GameplayStatics.h"
#include "BuildingPart.h"
#include "PlayerWidget.h"
#include "ObjectiveWidget.h"
#include "SurvivalSaveGame.h"
#include "PlayerChar.generated.h"

UCLASS()
class SENECAL312_API APlayerChar : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerChar();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION()
	void MoveForward(float axisValue);

	UFUNCTION()
	void MoveRight(float axisValue);

	UFUNCTION()
	void StartJump();

	UFUNCTION()
	void StopJump();

	UFUNCTION()
	void StartSprint();

	UFUNCTION()
	void StopSprint();

	UFUNCTION()
	void FindObject();

	// F5 / F9
	UFUNCTION()
	void QuickSave();

	UFUNCTION()
	void QuickLoad();

	// Used by the save system.
	void WriteSaveData(FPlayerSaveData& OutData);
	void ApplySaveData(const FPlayerSaveData& InData);

	// Stat setters add 'amount' to the stat and clamp the result to its valid range.
	UFUNCTION(BlueprintCallable)
	void SetHealth(float amount);

	UFUNCTION(BlueprintCallable)
	void SetHunger(float amount);

	UFUNCTION(BlueprintCallable)
	void SetStamina(float amount);

	// Current stamina ceiling. Drops below MaxStamina when hunger is low.
	UFUNCTION(BlueprintPure, Category = "Player Stats")
	float GetMaxStaminaCap() const;

	UFUNCTION(BlueprintPure, Category = "Player Stats")
	bool IsSprinting() const { return bIsSprinting; }

	UFUNCTION()
	void GiveResource(float amount, FString resourceType);

	UFUNCTION(BlueprintCallable)
	void UpdateResources(float woodAmount, float stoneAmount, FString buildingObject);

	UFUNCTION(BlueprintCallable)
	void SpawnBuilding(int buildingID, bool& isSuccess);

	UFUNCTION()
	void RotateBuilding();

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* PlayerCamComp;

	// - - CURRENT STATS - -

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Player Stats")
	float Health = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Player Stats")
	float Hunger = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Player Stats")
	float Stamina = 100.0f;

	// - - MAXIMUMS - -

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats|Max")
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats|Max")
	float MaxHunger = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats|Max")
	float MaxStamina = 100.0f;

	// - - RATES (PER SECOND) - -

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats|Rates")
	float HungerDrainPerSecond = 0.5f;

	// Health lost per second while hunger is at 0.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats|Rates")
	float StarvationDamagePerSecond = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats|Rates")
	float StaminaRegenPerSecond = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats|Rates")
	float HealthRegenPerSecond = 1.0f;

	// - - HUNGER EFFECTS - -

	// Below this hunger, max stamina shrinks and stamina regen slows.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats|Hunger Effects")
	float LowHungerThreshold = 30.0f;

	// Fraction of MaxStamina left when hunger reaches 0 (scales linearly from LowHungerThreshold).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats|Hunger Effects", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LowHungerMinStaminaFraction = 0.5f;

	// Stamina regen is multiplied by this while hunger is below LowHungerThreshold.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats|Hunger Effects", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LowHungerStaminaRegenMultiplier = 0.5f;

	// Health only regenerates while hunger is at or above this.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats|Hunger Effects")
	float HealthRegenHungerThreshold = 60.0f;

	// - - DAMAGE EFFECTS - -

	// Seconds after taking damage before stamina starts regenerating again.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats|Damage Effects")
	float StaminaRegenDelayAfterDamage = 2.0f;

	// Seconds after taking damage before health starts regenerating again.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats|Damage Effects")
	float HealthRegenDelayAfterDamage = 5.0f;

	// - - ACTION COSTS - -

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats|Actions")
	float GatherStaminaCost = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats|Actions")
	float SprintStaminaPerSecond = 15.0f;

	// Sprint speed = normal walk speed * this.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats|Actions")
	float SprintSpeedMultiplier = 1.5f;

	// After running out of stamina, sprinting stays locked until stamina recovers to this.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats|Actions")
	float SprintRecoverThreshold = 20.0f;

	// - - RESOURCES / BUILDING - -

	UPROPERTY(EditAnywhere, Category = "Resources")
	int Wood;

	UPROPERTY(EditAnywhere, Category = "Resources")
	int Stone;

	UPROPERTY(EditAnywhere, Category = "Resources")
	int Berry;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Resources")
	TArray<int> ResourcesArray;

	UPROPERTY(EditAnywhere, Category = "Resources")
	TArray<FString> ResourcesNameArray;

	UPROPERTY(EditAnywhere, Category = "HitMarker")
	UMaterialInterface* hitDecal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Building Supplies")
	TArray<int> BuildingArray;

	UPROPERTY()
	bool isBuilding;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<ABuildingPart> BuildPartClass;

	UPROPERTY()
	ABuildingPart* spawnedPart;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPlayerWidget* playerUI;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UObjectiveWidget* objWidget;

	UPROPERTY(SaveGame)
	float objectsBuilt;

	UPROPERTY(SaveGame)
	float matsCollected;

private:
	// Runs every frame: hunger drain, starvation, regen, sprint drain.
	void UpdateStats(float DeltaTime);

	bool bWantsToSprint = false;
	bool bIsSprinting = false;
	bool bIsExhausted = false;

	// BuildingArray index of the part currently being placed (-1 if none).
	// If you save mid-placement, that part is refunded in the save instead of being saved half-placed.
	int32 CurrentBuildID = -1;

	// Normal movement speed, captured from the movement component when sprinting starts.
	float WalkSpeed = 600.0f;

	// World time of the last hit taken. Starts far in the past so regen isn't blocked at spawn.
	float LastDamageTime = -1000.0f;
};