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

	// - - PLAYER STAT SETTERS - -

	UFUNCTION(BlueprintCallable)
		void SetHealth(float amount);

	UFUNCTION(BlueprintCallable)
		void SetHunger(float amount);

	UFUNCTION(BlueprintCallable)
		void SetStamina(float amount);

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

	// - - CURRENT PLAYER STATS - -

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
		float Health = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
		float Hunger = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
		float Stamina = 100.0f;

	// - - MAX PLAYER STATS - -

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats|Max")
		float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats|Max")
		float MaxHunger = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats|Max")
		float MaxStamina = 100.0f;

	// - - RATES PER SECOND - -

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerStats|Rates")
		float HungerDrainPerSecond = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerStats|Rates")
		float StarvationDamagePerSecond = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerStats|Rates")
		float StaminaRegenPerSecond = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerStats|Rates")
		float HealthRegenPerSecond = 1.0f;

	// - - HUNGER EFFECTS - -

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerStats|Hunger Effects")
		float LowHungerThreshold = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerStats|Hunger Effects", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float LowHungerMinStaminaFraction = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerStats|Hunger Effects", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float LowHungerMinStaminaRegenMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerStats|Hunger Effects")
		float HealthRegenHungerThreshold = 60.0f;

	// - - DAMAGE EFFECTS - -

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerStats|Damage Effects")
		float HealthRegenDelayAfterDamage = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerStats|Damage Effects")
		float StaminaRegenDelayAfterDamage = 2.0f;

	// - - ACTION COSTS - -

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerStats|Actions")
		float GatherStaminaCost = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerStats|Actions")
		float SprintStaminaPerSecond = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerStats|Actions")
		float SprintSpeedMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerStats|Actions")
		float SprintRecoverThreshold = 20.0f;

	// - - RESOURCES + BUILDING - -
	UPROPERTY(EditAnywhere, Category = "Resources")
		int Wood;

	UPROPERTY(EditAnywhere, Category = "Resources")
		int Stone;

	UPROPERTY(EditAnywhere, Category = "Resources")
		int Berry;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
		TArray<int> ResourcesArray;

	UPROPERTY(EditAnywhere, Category = "Resources")
		TArray<FString> ResourcesNameArray;

	UPROPERTY(EditAnywhere, Category = "HitMarker")
		UMaterialInterface* hitDecal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Supplies")
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

	UPROPERTY()
		float objectsBuilt;

	UPROPERTY()
		float matsCollected;

private:
	// Runs every frame: hunger drain, starvation, regen, sprint drain.
	void UpdateStats(float DeltaTime);

	bool bWantsToSprint = false;
	bool bIsSprinting = false;
	bool bIsExhausted = false;

	// Normal walk speed, captured from CharacterMovementComponent at BeginPlay to allow for sprinting speed changes
	float WalkSpeed = 600.0f;

	// Time since last damage taken, starts far in the past so regen is not prevented at spawn
	float LastDamageTime = -1000.0f;
};
