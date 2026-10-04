// Fill out your copyright notice in the Description page of Project Settings.


#include "Resource_M.h"
#include "TimerManager.h"

// Sets default values
AResource_M::AResource_M()
{
	// Nothing runs per frame on resources; regeneration uses a timer instead.
	PrimaryActorTick.bCanEverTick = false;

	ResourceNameTxt = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Text Render"));
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));

	RootComponent = Mesh;

	ResourceNameTxt->SetupAttachment(Mesh);

}

// Called when the game starts or when spawned
void AResource_M::BeginPlay()
{
	Super::BeginPlay();

	tempText = tempText.FromString(resourceName);

	ResourceNameTxt->SetText(tempText);

	// Start full.
	currentResource = totalResource;
	bIsDepleted = false;

}

// Called every frame
void AResource_M::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

int AResource_M::Gather()
{
	if (bIsDepleted || currentResource <= 0) {
		return 0;
	}

	// Never hand out more than what's left, so the final gather still gives something.
	const int gathered = FMath::Min(resourceAmount, currentResource);
	currentResource -= gathered;

	if (currentResource <= 0) {
		Deplete();
	}

	return gathered;
}

void AResource_M::Deplete()
{
	bIsDepleted = true;
	currentResource = 0;

	SetNodeVisible(false);

	GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AResource_M::Respawn, respawnTime, false);
}

void AResource_M::Respawn()
{
	currentResource = totalResource;
	bIsDepleted = false;

	SetNodeVisible(true);
}

void AResource_M::SetNodeVisible(bool bVisible)
{
	// Hidden + no collision means it can't be seen, walked into, or hit by the interact trace.
	SetActorHiddenInGame(!bVisible);
	SetActorEnableCollision(bVisible);
}

void AResource_M::OnBeforeSave_Implementation()
{
	SavedRespawnRemaining = 0.0f;

	if (bIsDepleted) {
		// Returns -1 if the timer isn't running; treat that as "respawn right away".
		SavedRespawnRemaining = FMath::Max(GetWorldTimerManager().GetTimerRemaining(RespawnTimerHandle), 0.0f);
	}
}

void AResource_M::OnAfterLoad_Implementation()
{
	GetWorldTimerManager().ClearTimer(RespawnTimerHandle);

	if (bIsDepleted) {
		SetNodeVisible(false);
		GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AResource_M::Respawn, FMath::Max(SavedRespawnRemaining, 0.1f), false);
	}
	else {
		SetNodeVisible(true);
	}
}