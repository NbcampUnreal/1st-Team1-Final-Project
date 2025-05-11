#include "Character/Component/GS_StatComp.h"

UGS_StatComp::UGS_StatComp()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UGS_StatComp::InitStat()
{
	//init default stats from data table
}

void UGS_StatComp::UpdateStat()
{
	//update stats by rune system
}

float UGS_StatComp::CalculateDamage(float InSkillCoefficient, float SlopeCoefficient)
{
	float Damage = 0.f;
	Damage = (AttackPower * InSkillCoefficient) * (100.f / 100.f + SlopeCoefficient * Defense);

	return Damage;
}

void UGS_StatComp::PerformHit(AActor* DamagedActor, AActor* DamageCauser)
{
	//UGameplayStatics::ApplyDamage(DamagedActor, Damage, EventInstigator, DamageCauser, UDamageType::StaticClass());

	/*float Damage = CalculateDamage();
	if (IsValid(DamagedActor))
	{
		AGS_Character* DamagedCharacter = Cast<AGS_Character>(DamagedActor);
		if (IsValid(DamagedCharacter))
		{
			UGS_StatComp* DamagedActorStatComp = DamagedCharacter->GetStatComp();
			if (IsValid(DamagedActorStatComp))
			{
				float DamagedActorCurrentHealth = DamagedActorStatComp->GetCurrentHealth();
				DamagedActorStatComp->SetCurrentHealth(DamagedActorCurrentHealth - Damage);
			}
		}
	}*/
}


void UGS_StatComp::SetCurrentHealth(float InHealth)
{
	if (!IsValid(GetOwner()) || !GetOwner()->HasAuthority())
	{
		return;
	}

	CurrentHealth = InHealth;

	//dead
	if (CurrentHealth <= KINDA_SMALL_NUMBER)
	{
		CurrentHealth = 0.f;

	}
	OnCurrentHPChanged.Broadcast(CurrentHealth);
}

void UGS_StatComp::SetMaxHealth(float InMaxHealth)
{
	MaxHealth = InMaxHealth;
}

void UGS_StatComp::SetAttackPower(float InAttackPower)
{
	AttackPower = InAttackPower;
}

void UGS_StatComp::SetDefense(float InDefense)
{
	Defense = InDefense;
}

void UGS_StatComp::SetAgility(float InAgility)
{
	Agility = InAgility;
}

void UGS_StatComp::SetAttackSpeed(float InAttackSpeed)
{
	AttackSpeed = InAttackSpeed;
}

void UGS_StatComp::BeginPlay()
{
	Super::BeginPlay();

}



