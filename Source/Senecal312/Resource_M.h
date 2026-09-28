// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Resource_M.generated.h"

UCLASS()
class SENECAL312_API AResource_M : public AActor
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

	UFUNCTION(BlueprintCallable, Category = "Resource")
		int Gather();

	UFUNCTION(BlueprintCallable, Category = "Resource")
	bool IsDepleted() const { return bIsDepleted; }

	UPROPERTY(EditAnywhere, Category = "Resource")
		FString resourceName = "Wood";

	UPROPERTY(EditAnywhere, Category = "Resource")
		int resourceAmount = 5;

	UPROPERTY(EditAnywhere, Category = "Resource")
		int totalResource = 100;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resource")
		int currentResource = 0;

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

	UPROPERTY(VisibleAnywhere, Category = "Resource")
		bool bIsDepleted = false;

	FTimerHandle RespawnTimerHandle;

};
