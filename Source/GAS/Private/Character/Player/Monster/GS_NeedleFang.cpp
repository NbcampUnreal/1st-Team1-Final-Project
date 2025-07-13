// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Monster/GS_NeedleFang.h"
#include "AI/GS_AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon/Equipable/GS_WeaponWand.h"
#include "Weapon/Projectile/Guardian/GS_NeedleFangProjectile.h"

AGS_NeedleFang::AGS_NeedleFang()
{
}


void AGS_NeedleFang::BeginPlay()
{
	Super::BeginPlay();
}


void AGS_NeedleFang::Server_SpawnProjectile_Implementation()
{
	FVector SpawnLocation;
	FRotator SpawnRotation;
	
	AGS_WeaponWand* EquippedWeapon = Cast<AGS_WeaponWand>(GetWeaponByIndex(0));
	if (EquippedWeapon && EquippedWeapon->GetWeaponMesh()) 
	{
		SpawnLocation = EquippedWeapon->GetWeaponMesh()->GetSocketLocation(TEXT("ProjectileSpawnSocket")) + FVector(0, 0, 50);
	}

	AGS_AIController* AIController = Cast<AGS_AIController>(GetController());
	AGS_Character* TargetActor = nullptr;
	if (AIController && AIController->GetBlackboardComponent())
	{
		TargetActor = Cast<AGS_Character>(AIController->GetBlackboardComponent()->GetValueAsObject(AGS_AIController::TargetActorKey)); 
	}

	if (TargetActor)
	{
		FVector MonsterForwardVector = GetActorForwardVector();
		FVector DirectionToTarget = (TargetActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		
		float DotProduct = FVector::DotProduct(MonsterForwardVector, DirectionToTarget);
		const float HalfAngleRadians = FMath::DegreesToRadians(20.0f); 
		const float CosineThreshold = FMath::Cos(HalfAngleRadians);
		if (DotProduct >= CosineThreshold)
		{
			SpawnRotation = UKismetMathLibrary::FindLookAtRotation(SpawnLocation, TargetActor->GetActorLocation());
		}
		else
		{
			SpawnRotation = GetActorRotation();
		}
	}
	else
	{
		SpawnRotation = GetActorRotation();
	}
	
	if (ProjectileClass) 
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this; 
		SpawnParams.Instigator = GetInstigator(); 

		AGS_NeedleFangProjectile* SpawnedProjectile = GetWorld()->SpawnActor<AGS_NeedleFangProjectile>(
		   ProjectileClass,
		   SpawnLocation,
		   SpawnRotation,
		   SpawnParams
		);
	}
}