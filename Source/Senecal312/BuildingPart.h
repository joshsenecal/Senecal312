// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ArrowComponent.h"
#include "SaveableActor.h"
#include "BuildingPart.generated.h"

UCLASS()
class SENECAL312_API ABuildingPart : public AActor, public ISaveableActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABuildingPart();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// ISaveableActor
	virtual void OnBeforeSave_Implementation() override;
	virtual void OnAfterLoad_Implementation() override;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere)
	UArrowComponent* PivotArrow;

	// False while the part is still a preview following the camera. Previews aren't saved.
	UPROPERTY(BlueprintReadWrite, Category = "Building")
	bool bIsPlaced = true;

private:
	// Which mesh this part uses, in case walls/floors/ceilings share a class and only swap meshes.
	UPROPERTY(SaveGame)
	TSoftObjectPtr<UStaticMesh> SavedMesh;
};