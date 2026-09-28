// Fill out your copyright notice in the Description page of Project Settings.


#include "Resource_M.h"
#include "TimerManager.h"

// Sets default values
AResource_M::AResource_M()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
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
	if (bIsDepleted || currentResource <= 0)
	{
		return 0;
	}

	const int gathered = FMath::Min(resourceAmount, currentResource);
	currentResource -= gathered;

	if (currentResource <= 0)
	{
		Deplete();
	}

	return gathered;
}

void AResource_M::Deplete()
{
	bIsDepleted = true;
	currentResource = 0;

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);

	GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AResource_M::Respawn, respawnTime, false);
}

void AResource_M::Respawn()
{
	currentResource = totalResource;
	bIsDepleted = false;

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
}

