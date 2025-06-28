#include "Character/Component/GS_StatComp.h"

#include "AkGameplayStatics.h"
#include "Character/GS_Character.h"
#include "Character/Component/GS_StatRow.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "RuneSystem/GS_EnumUtils.h"
#include "RuneSystem/GS_ArcaneBoardLPS.h"
#include "RuneSystem/GS_ArcaneBoardManager.h"
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

	AGS_Character* OwnerCharacter = Cast<AGS_Character>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
	if (PC && PC->IsLocalController())
	{
		if (UGS_ArcaneBoardLPS* LPS = PC->GetLocalPlayer()->GetSubsystem<UGS_ArcaneBoardLPS>())
		{
			if (UGS_ArcaneBoardManager* Manager = LPS->GetOrCreateBoardManager())
			{
				FArcaneBoardStats AppliedStats = Manager->AppliedBoardStats;
				FGS_StatRow RuneStats = AppliedStats.RuneStats+ AppliedStats.BonusStats;
				UpdateStat(RuneStats);

				UE_LOG(LogTemp, Warning, TEXT("StatComp BeginPlay: 룬 스탯 적용 완료"));
			}
		}
	}
}

void UGS_StatComp::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);	

	DOREPLIFETIME(ThisClass, CurrentHealth);
	DOREPLIFETIME(ThisClass, bIsInvincible);
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

void UGS_StatComp::ChangeStat(const FGS_StatRow& InChangeStat)
{
	MaxHealth += InChangeStat.HP;
	AttackPower += InChangeStat.ATK;
	Defense += InChangeStat.DEF;
	Agility += InChangeStat.AGL;
	AttackSpeed += InChangeStat.ATS;
}

void UGS_StatComp::ResetStat(const FGS_StatRow& InChangeStat)
{
	MaxHealth -= InChangeStat.HP;
	AttackPower -= InChangeStat.ATK;
	Defense -= InChangeStat.DEF;
	Agility -= InChangeStat.AGL;
	AttackSpeed -= InChangeStat.ATS;
}

void UGS_StatComp::UpdateStat_Implementation(const FGS_StatRow& RuneStats)
{
	//update stats by rune system
	AGS_Character* OwnerCharacter = Cast<AGS_Character>(GetOwner());
	FString CurrClass = UGS_EnumUtils::GetEnumAsString<ECharacterType>(OwnerCharacter->GetCharacterType());
	FName RowName = FName(CurrClass);
	const FGS_StatRow* FoundRow = StatDataTable->FindRow<FGS_StatRow>(RowName, TEXT("InitStat"));

	if (FoundRow)
	{
		MaxHealth = FoundRow->HP + RuneStats.HP;
		AttackPower = FoundRow->ATK + RuneStats.ATK;
		Defense = FoundRow->DEF + RuneStats.DEF;
		Agility = FoundRow->AGL + RuneStats.AGL;
		AttackSpeed = FoundRow->ATS + RuneStats.ATS;

		UE_LOG(LogTemp, Log, TEXT("캐릭터 스탯 업데이트 - HP: %.1f, ATK: %.1f, DEF: %.1f, AGL: %.1f, ATS: %.1f"),
			MaxHealth, AttackPower, Defense, Agility, AttackSpeed);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("스탯 컴프 로우 네임 못찾음"));
	}
}

float UGS_StatComp::CalculateDamage(AGS_Character* InDamageCauser, AGS_Character* InDamagedCharacter, float InSkillCoefficient, float SlopeCoefficient)
{
	if (bIsInvincible)
	{
		return 0.f;
	}

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
	
	float PreviousHealth = CurrentHealth;
	
	//update health
	CurrentHealth = InHealth;
	OnCurrentHPChanged.Broadcast(this);
	
	//healing
	if (bIsHealing)
	{
		if (CurrentHealth > MaxHealth)
		{
			CurrentHealth = MaxHealth;
		}
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
		if (CurrentHealth <= KINDA_SMALL_NUMBER && PreviousHealth > KINDA_SMALL_NUMBER)
		{
			UE_LOG(LogTemp, Warning, TEXT("death"));
			CurrentHealth = 0.f;
	
			AGS_Character* OwnerCharacter = Cast<AGS_Character>(GetOwner());
			if (IsValid(OwnerCharacter))
			{
				OwnerCharacter->OnDeath();
			}
		}
		else if (CurrentHealth <= KINDA_SMALL_NUMBER)
		{
			// 이미 죽은 상태에서 추가 데미지를 받은 경우 HP를 0으로 고정만 하고 OnDeath()는 호출하지 않음
			CurrentHealth = 0.f;
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

	// 죽은 상태가 아닐 때만 히트 사운드 재생
	if (HitSoundEvent && CanPlayHitSound() && CurrentHealth > KINDA_SMALL_NUMBER)
	{
		UAkGameplayStatics::PostEvent(HitSoundEvent, OwnerCharacter, 0, FOnAkPostEventCallback());
		LastHitSoundTime = GetWorld()->GetTimeSeconds();
	}
	
	int32 idx = FMath::RandRange(0, TakeDamageMontages.Num() - 1);
	UAnimMontage* AnimMontage = TakeDamageMontages[idx];

	if (IsValid(OwnerCharacter))
	{
		if (AGS_Monster* Monster = Cast<AGS_Monster>(OwnerCharacter))
		{
			Monster->PlayAnimMontage(AnimMontage);

			if (Monster->HasAuthority())
			{
				//stop character during damage animation
				CharacterWalkSpeed = Monster->GetCharacterMovement()->MaxWalkSpeed;
				Monster->GetCharacterMovement()->MaxWalkSpeed = 0.f;
			}

			if (UAnimInstance* AnimInstance = Monster->GetMesh()->GetAnimInstance())
			{
				FOnMontageBlendingOutStarted BlendOut;
				BlendOut.BindUObject(this, &UGS_StatComp::OnDamageMontageEnded);
				AnimInstance->Montage_SetBlendingOutDelegate(BlendOut, AnimMontage);
			}
		}
	}
}

void UGS_StatComp::OnRep_CurrentHealth()
{
	OnCurrentHPChanged.Broadcast(this);
}

void UGS_StatComp::SetInvincible(bool bEnable)
{
	bIsInvincible = bEnable;
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
		OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = CharacterWalkSpeed;
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

bool UGS_StatComp::CanPlayHitSound() const
{
	if (!GetWorld())
	{
		return false;
	}
	
	float CurrentTime = GetWorld()->GetTimeSeconds();
	return (CurrentTime - LastHitSoundTime) >= HitSoundCooldownTime;
}

ECharacterClass UGS_StatComp::MapCharacterTypeToCharacterClass(ECharacterType CharacterType)
{
	switch (CharacterType)
	{
	case ECharacterType::Ares:
		return ECharacterClass::Ares;
	case ECharacterType::Chan:
		return ECharacterClass::Chan;
	case ECharacterType::Merci:
		return ECharacterClass::Merci;
	default:
		UE_LOG(LogTemp, Warning, TEXT("MapCharacterTypeToCharacterClass: 알 수 없는 캐릭터 타입, 기본값 Ares 반환"));
		return ECharacterClass::Ares;
	}
}