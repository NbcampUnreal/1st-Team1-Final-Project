// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GS_Seeker.h"
#include "Character/Interface/GS_AttackInterface.h"
#include "GS_Merci.generated.h"

UCLASS()
class GAS_API AGS_Merci : public AGS_Seeker, public IGS_AttackInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGS_Merci();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Attack Interface
	virtual void LeftClickPressed_Implementation() override;
	virtual void LeftClickRelease_Implementation() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim")
	UUserWidget* WidgetCrosshair;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon", meta=(AllowPrivateAccess="true"))
	USkeletalMeshComponent* Quiver;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
