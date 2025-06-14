// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Debuff/GS_DebuffDotBase.h"
#include "Kismet/GameplayStatics.h"
#include "Character/GS_Character.h"
#include "Character/Component/GS_StatComp.h"

void UGS_DebuffDotBase::OnApply()
{
    if (!IsValid(TargetCharacter))
    {
        UE_LOG(LogTemp, Error, TEXT("DOT OnApply: TargetCharacter is nullptr"));
        return;
    }
	TargetCharacter->GetWorld()->GetTimerManager().SetTimer(DamageTimerHandle, this, &UGS_DebuffDotBase::ApplyDotDamage, DamageInterval, true);

	//DamageToApply = TargetCharacter->GetStatComp()->CalculateDamage(OwnerCharacter, TargetCharacter);
}

void UGS_DebuffDotBase::OnExpire()
{
    if (IsValid(TargetCharacter))
    {
        TargetCharacter->GetWorld()->GetTimerManager().ClearTimer(DamageTimerHandle);
    }
}

void UGS_DebuffDotBase::ApplyDotDamage()
{
	if (!IsValid(TargetCharacter))
	{
        UE_LOG(LogTemp, Error, TEXT("DOT: TargetCharacter is nullptr"));
        return;
	}

    AController* InstigatorController = nullptr;
    AActor* DamageCauser = nullptr;

    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("DOT: OwnerCharacter is nullptr"));
    }

    if (IsValid(OwnerCharacter))
    {
        if (AGS_Character* Cuaser = Cast<AGS_Character>(OwnerCharacter))
        {
            InstigatorController = Cuaser->GetController();
        }
        DamageCauser = OwnerCharacter;
       
    }

    UE_LOG(LogTemp, Log, TEXT("DOT: Apply %f damage to %s"), Damage, *TargetCharacter->GetName());

    UGameplayStatics::ApplyDamage(
        TargetCharacter,
        Damage,
        InstigatorController,
        DamageCauser,
        UDamageType::StaticClass()
    );
}
