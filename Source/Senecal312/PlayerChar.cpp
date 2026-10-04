// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerChar.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/DamageEvents.h"
#include "SaveSubsystem.h"

// Sets default values
APlayerChar::APlayerChar()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Initial setup of camera component.
	PlayerCamComp = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Cam"));

	// Attaching camera to the character mesh and head bone.
	PlayerCamComp->SetupAttachment(GetMesh(), "head");

	// Share rotation with controller.
	PlayerCamComp->bUsePawnControlRotation = true;

	BuildingArray.SetNum(3);
	ResourcesArray.SetNum(3);
	ResourcesNameArray.Add(TEXT("Wood"));
	ResourcesNameArray.Add(TEXT("Stone"));
	ResourcesNameArray.Add(TEXT("Berry"));

}

// Called when the game starts or when spawned
void APlayerChar::BeginPlay()
{
	Super::BeginPlay();

	if (objWidget) {

		objWidget->UpdatebuildOBJ(0.0f);
		objWidget->UpdatematOBJ(0.0f);
	}

}

// Called every frame
void APlayerChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateStats(DeltaTime);

	if (playerUI) {
		playerUI->UpdateBars(Health, Hunger, Stamina);
	}

	if (isBuilding) {
		if (spawnedPart) {
			FVector StartLocation = PlayerCamComp->GetComponentLocation();
			FVector Direction = PlayerCamComp->GetForwardVector() * 400.0f;
			FVector EndLocation = StartLocation + Direction;
			spawnedPart->SetActorLocation(EndLocation);
		}
	}
}

// Called to bind functionality to input
void APlayerChar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Setup player axis
	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerChar::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerChar::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &APlayerChar::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &APlayerChar::AddControllerYawInput);

	// Setup player actions
	PlayerInputComponent->BindAction("JumpEvent", IE_Pressed, this, &APlayerChar::StartJump);
	PlayerInputComponent->BindAction("JumpEvent", IE_Released, this, &APlayerChar::StopJump);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &APlayerChar::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &APlayerChar::StopSprint);
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &APlayerChar::FindObject);
	PlayerInputComponent->BindAction("RotPart", IE_Pressed, this, &APlayerChar::RotateBuilding);

	// Save + load keys (bound directly, no Input settings needed)
	PlayerInputComponent->BindKey(EKeys::F5, IE_Pressed, this, &APlayerChar::QuickSave);
	PlayerInputComponent->BindKey(EKeys::F9, IE_Pressed, this, &APlayerChar::QuickLoad);

}

float APlayerChar::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (ActualDamage > 0.0f) {
		SetHealth(-ActualDamage);

		// Pauses stamina and health regen for a moment.
		LastDamageTime = GetWorld()->GetTimeSeconds();
	}

	return ActualDamage;
}

void APlayerChar::MoveForward(float axisValue)
{
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::X);
	AddMovementInput(Direction, axisValue);
}

void APlayerChar::MoveRight(float axisValue)
{
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::Y);
	AddMovementInput(Direction, axisValue);
}

void APlayerChar::StartJump()
{
	bPressedJump = true;
}

void APlayerChar::StopJump()
{
	bPressedJump = false;
}

void APlayerChar::StartSprint()
{
	bWantsToSprint = true;
}

void APlayerChar::StopSprint()
{
	bWantsToSprint = false;
}

void APlayerChar::FindObject()
{
	FHitResult HitResult;
	FVector StartLocation = PlayerCamComp->GetComponentLocation();
	FVector Direction = PlayerCamComp->GetForwardVector() * 800.0f;
	FVector EndLocation = StartLocation + Direction;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnFaceIndex = true;

	if (!isBuilding) {
		if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams)) {

			AResource_M* HitResource = Cast<AResource_M>(HitResult.GetActor());

			// Gathering needs enough stamina to pay for it.
			if (HitResource && Stamina >= GatherStaminaCost) {

				int gathered = HitResource->Gather();

				if (gathered > 0) {
					GiveResource(gathered, HitResource->resourceName);

					matsCollected = matsCollected + gathered;

					if (objWidget) {
						objWidget->UpdatematOBJ(matsCollected);
					}

					check(GEngine != nullptr);
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Resource Collected"));

					UGameplayStatics::SpawnDecalAtLocation(GetWorld(), hitDecal, FVector(10.0f, 10.0f, 10.0f), HitResult.Location, FRotator(-90, 0, 0), 2.0f);

					SetStamina(-GatherStaminaCost);
				}

				if (HitResource->IsDepleted()) {
					check(GEngine != nullptr);
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Resource Depleted"));
				}
			}
		}
	}
	else {
		isBuilding = false;
		objectsBuilt = objectsBuilt + 1.0f;

		// The preview is now a real placed part, so it gets saved from here on.
		if (spawnedPart) {
			spawnedPart->bIsPlaced = true;
		}
		CurrentBuildID = -1;

		if (objWidget) {
			objWidget->UpdatebuildOBJ(objectsBuilt);
		}
	}
}

void APlayerChar::SetHealth(float amount)
{
	Health = FMath::Clamp(Health + amount, 0.0f, MaxHealth);
}

void APlayerChar::SetHunger(float amount)
{
	Hunger = FMath::Clamp(Hunger + amount, 0.0f, MaxHunger);
}

void APlayerChar::SetStamina(float amount)
{
	Stamina = FMath::Clamp(Stamina + amount, 0.0f, GetMaxStaminaCap());
}

float APlayerChar::GetMaxStaminaCap() const
{
	if (LowHungerThreshold <= 0.0f || Hunger >= LowHungerThreshold) {
		return MaxStamina;
	}

	// Scales from full MaxStamina at the threshold down to the minimum fraction at 0 hunger.
	const float Alpha = FMath::Clamp(Hunger / LowHungerThreshold, 0.0f, 1.0f);
	return FMath::Lerp(MaxStamina * LowHungerMinStaminaFraction, MaxStamina, Alpha);
}

void APlayerChar::UpdateStats(float DeltaTime)
{
	const float TimeSinceDamage = GetWorld()->GetTimeSeconds() - LastDamageTime;

	// Hunger drains over time
	SetHunger(-HungerDrainPerSecond * DeltaTime);

	// Health: starve at 0 hunger, regenerate when well fed
	if (Hunger <= 0.0f) {
		SetHealth(-StarvationDamagePerSecond * DeltaTime);
	}
	else if (Hunger >= HealthRegenHungerThreshold
		&& Health > 0.0f
		&& Health < MaxHealth
		&& TimeSinceDamage >= HealthRegenDelayAfterDamage) {
		SetHealth(HealthRegenPerSecond * DeltaTime);
	}

	// Sprinting
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	const bool bMoving = GetVelocity().SizeSquared2D() > 1.0f;
	const bool bOnGround = !MoveComp->IsFalling();

	// Running out of stamina locks sprint until it recovers a bit, so it doesn't stutter on/off at 0.
	if (Stamina <= 0.0f) {
		bIsExhausted = true;
	}
	else if (bIsExhausted && Stamina >= SprintRecoverThreshold) {
		bIsExhausted = false;
	}

	const bool bShouldSprint = bWantsToSprint && bMoving && !bIsExhausted;

	if (bShouldSprint != bIsSprinting) {
		// Remember the normal speed right before sprinting starts, so it's always the real current value.
		if (bShouldSprint) {
			WalkSpeed = MoveComp->MaxWalkSpeed;
		}

		bIsSprinting = bShouldSprint;
		MoveComp->MaxWalkSpeed = bIsSprinting ? WalkSpeed * SprintSpeedMultiplier : WalkSpeed;
	}

	// Stamina: drain while sprinting, otherwise regen (paused briefly after damage)
	if (bIsSprinting) {
		if (bOnGround) {
			SetStamina(-SprintStaminaPerSecond * DeltaTime);
		}
	}
	else if (TimeSinceDamage >= StaminaRegenDelayAfterDamage) {
		float Regen = StaminaRegenPerSecond;

		if (Hunger < LowHungerThreshold) {
			Regen *= LowHungerStaminaRegenMultiplier;
		}

		SetStamina(Regen * DeltaTime);
	}

	// If low hunger just lowered the cap, pull stamina down to it.
	Stamina = FMath::Min(Stamina, GetMaxStaminaCap());
}

void APlayerChar::GiveResource(float amount, FString resourceType)
{
	if (resourceType == "Wood") {

		ResourcesArray[0] = ResourcesArray[0] + amount;

	}

	if (resourceType == "Stone") {

		ResourcesArray[1] = ResourcesArray[1] + amount;

	}

	if (resourceType == "Berry") {

		ResourcesArray[2] = ResourcesArray[2] + amount;

	}
}

void APlayerChar::UpdateResources(float woodAmount, float stoneAmount, FString buildingObject)
{
	if (woodAmount <= ResourcesArray[0]) {
		if (stoneAmount <= ResourcesArray[1]) {
			ResourcesArray[0] = ResourcesArray[0] - woodAmount;
			ResourcesArray[1] = ResourcesArray[1] - stoneAmount;

			if (buildingObject == "Wall") {
				BuildingArray[0] = BuildingArray[0] + 1;
			}

			if (buildingObject == "Floor") {
				BuildingArray[1] = BuildingArray[1] + 1;
			}

			if (buildingObject == "Ceiling") {
				BuildingArray[2] = BuildingArray[2] + 1;
			}
		}
	}
}

void APlayerChar::SpawnBuilding(int buildingID, bool& isSuccess)
{
	// Default to failure; only set true once a part actually spawns.
	isSuccess = false;

	if (!isBuilding) {
		if (BuildingArray.IsValidIndex(buildingID) && BuildingArray[buildingID] >= 1) {
			isBuilding = true;
			FActorSpawnParameters SpawnParams;
			FVector StartLocation = PlayerCamComp->GetComponentLocation();
			FVector Direction = PlayerCamComp->GetForwardVector() * 400.0f;
			FVector EndLocation = StartLocation + Direction;
			FRotator myRot(0, 0, 0);

			BuildingArray[buildingID] = BuildingArray[buildingID] - 1;

			spawnedPart = GetWorld()->SpawnActor<ABuildingPart>(BuildPartClass, EndLocation, myRot, SpawnParams);

			// Still a preview until the player places it.
			if (spawnedPart) {
				spawnedPart->bIsPlaced = false;
			}
			CurrentBuildID = buildingID;

			isSuccess = true;
		}
	}
}

void APlayerChar::RotateBuilding()
{
	if (isBuilding && spawnedPart) {
		spawnedPart->AddActorWorldRotation(FRotator(0, 90, 0));
	}
}

void APlayerChar::QuickSave()
{
	if (USaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<USaveSubsystem>()) {
		SaveSubsystem->QuickSave();
	}
}

void APlayerChar::QuickLoad()
{
	if (USaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<USaveSubsystem>()) {
		SaveSubsystem->LoadMostRecent();
	}
}

void APlayerChar::WriteSaveData(FPlayerSaveData& OutData)
{
	OutData.bHasData = true;
	OutData.Transform = GetActorTransform();
	OutData.ControlRotation = Controller ? Controller->GetControlRotation() : GetActorRotation();

	// Saving mid-placement: count the preview part as still in the inventory.
	const bool bRefundPreview = isBuilding && BuildingArray.IsValidIndex(CurrentBuildID);

	if (bRefundPreview) {
		BuildingArray[CurrentBuildID] += 1;
	}

	USaveSubsystem::WriteSaveProperties(this, OutData.ByteData);

	if (bRefundPreview) {
		BuildingArray[CurrentBuildID] -= 1;
	}
}

void APlayerChar::ApplySaveData(const FPlayerSaveData& InData)
{
	SetActorTransform(InData.Transform, false, nullptr, ETeleportType::TeleportPhysics);

	if (Controller) {
		Controller->SetControlRotation(InData.ControlRotation);
	}

	USaveSubsystem::ReadSaveProperties(this, InData.ByteData);

	// Fresh level, so nothing is being placed.
	isBuilding = false;
	spawnedPart = nullptr;
	CurrentBuildID = -1;

	if (objWidget) {
		objWidget->UpdatebuildOBJ(objectsBuilt);
		objWidget->UpdatematOBJ(matsCollected);
	}
}