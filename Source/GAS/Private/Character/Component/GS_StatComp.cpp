#include "Character/Component/GS_StatComp.h"

#include "Character/GS_Character.h"
#include "Character/Component/GS_StatRow.h"

#include "Net/UnrealNetwork.h"

UGS_StatComp::UGS_StatComp()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);

	static ConstructorHelpers::FObjectFinder<UDataTable> StatDataTableAsset(TEXT("/Game/DataTable/StatDataTable.StatDataTable"));
	if (StatDataTableAsset.Succeeded())
	{
		StatDataTable = StatDataTableAsset.Object;
	}
}

void UGS_StatComp::BeginPlay()
{
	Super::BeginPlay();

}

void UGS_StatComp::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);	

	DOREPLIFETIME(ThisClass, CurrentHealth);
}

void UGS_StatComp::InitStat(FName RowName)
{
	if (!IsValid(StatDataTable))
	{	
		return;
	}

	const FGS_StatRow* FoundRow = StatDataTable->FindRow<FGS_StatRow>(RowName, TEXT("InitStat"));

	if (FoundRow)
	{
		MaxHealth = FoundRow->HP;
		AttackPower = FoundRow->ATK;
		Defense = FoundRow->DEF;
		Agility = FoundRow->AGL;
		AttackSpeed = FoundRow->ATS;

		CurrentHealth = MaxHealth;
	}	
}

void UGS_StatComp::UpdateStat()
{
	//update stats by rune system
}

float UGS_StatComp::CalculateDamage(AGS_Character* InDamageCauser, AGS_Character* InDamagedCharacter, float InSkillCoefficient, float SlopeCoefficient)
{
	float Damage = 0.f;
	float DamagedCharacterDefense = InDamagedCharacter->GetStatComp()->GetDefense();
	float DamageCauserAttack = InDamageCauser->GetStatComp()->GetAttackPower();
	Damage = (DamageCauserAttack * InSkillCoefficient) * (100.f / (100.f + SlopeCoefficient * DamagedCharacterDefense));

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

void UGS_StatComp::OnRep_CurrentHealth()
{
	OnCurrentHPChanged.Broadcast(CurrentHealth);
}