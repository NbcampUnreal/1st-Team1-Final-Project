// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Props/Item/GS_Item.h"
#include "GS_HP_Potion.generated.h"

UCLASS()
class GAS_API AGS_HP_Potion : public AGS_Item
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGS_HP_Potion();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly, Category = "Item")
	FTimerHandle DestroyTimerHandle;

	UFUNCTION()
	void DropFromSocket();
};
