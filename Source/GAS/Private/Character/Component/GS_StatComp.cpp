#include "Character/Component/GS_StatComp.h"

#include "AIController.h"
#include "AkGameplayStatics.h"
#include "Character/GS_Character.h"
#include "Character/Component/GS_StatRow.h"

#include "GameFramework/CharacterMovementComponent.h"
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

	//set move speed
	AGS_Character* OwnerCharacter = Cast<AGS_Character>(GetOwner());
	OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed *= Agility;
}

void UGS_StatComp::UpdateStat(const FGS_StatRow& RuneStats)
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

void UGS_StatComp::SetCurrentHealth(float InHealth, bool bIsHealing)
{
	if (!IsValid(GetOwner()) || !GetOwner()->HasAuthority())
	{
		return;
	}
	//update health
	CurrentHealth = InHealth;
	OnCurrentHPChanged.Broadcast(this);
	
	//healing
	if (bIsHealing)
	{
		
	}
	//damaged
	else
	{
		//[TODO] play take damage montage
		if (!TakeDamageMontages.IsEmpty())
		{
			MulticastRPCPlayTakeDamageMontage();
		}

		//dead
		if (CurrentHealth <= KINDA_SMALL_NUMBER)
		{
			UE_LOG(LogTemp, Warning, TEXT("death"));
			CurrentHealth = 0.f;
	
			AGS_Character* OwnerCharacter = Cast<AGS_Character>(GetOwner());
			if (IsValid(OwnerCharacter))
			{
				OwnerCharacter->OnDeath();
			}
		}
	}
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

void UGS_StatComp::MulticastRPCPlayTakeDamageMontage_Implementation()
{
	AGS_Character* OwnerCharacter = Cast<AGS_Character>(GetOwner());

	if (HitSoundEvent)
	{
		UAkGameplayStatics::PostEvent(HitSoundEvent, OwnerCharacter, 0, FOnAkPostEventCallback());
	}
	
	int32 idx = FMath::RandRange(0, TakeDamageMontages.Num() - 1);
	UAnimMontage* AnimMontage = TakeDamageMontages[idx];

	if (IsValid(OwnerCharacter))
	{	
		OwnerCharacter->PlayAnimMontage(AnimMontage);
		if(OwnerCharacter->HasAuthority())
		{
			//stop character during damage animation
			CharacterWalkSpeed = OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed;
			//OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = 0.f;
		}
		
		if (UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance())
		{
			FOnMontageBlendingOutStarted BlendOut;
			BlendOut.BindUObject(this, &UGS_StatComp::OnDamageMontageEnded);
			AnimInstance->Montage_SetBlendingOutDelegate(BlendOut, AnimMontage);
		}
	}
}

void UGS_StatComp::OnRep_CurrentHealth()
{
	OnCurrentHPChanged.Broadcast(this);
}

void UGS_StatComp::OnDamageMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	AGS_Character* OwnerCharacter = Cast<AGS_Character>(GetOwner());
	if (!IsValid(OwnerCharacter))
	{
		return;
	}
	if (OwnerCharacter->HasAuthority())
	{
		//can move
		//OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = CharacterWalkSpeed;
	}
}

// heal system
void UGS_StatComp::ServerRPCHeal_Implementation(float InHealAmount)
{
    if (!IsValid(GetOwner()) || !GetOwner()->HasAuthority())
    {
        return;
    }

    float NewHealth = FMath::Min(CurrentHealth + InHealAmount, MaxHealth);
    SetCurrentHealth(NewHealth, true);
}
