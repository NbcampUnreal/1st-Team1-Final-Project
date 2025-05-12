#include "Character/GS_Character.h"

#include "Character/Component/GS_StatComp.h"
#include "Character/Component/GS_DebuffComp.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Engine/DamageEvents.h"

AGS_Character::AGS_Character()
{
	PrimaryActorTick.bCanEverTick = false;

	StatComp = CreateDefaultSubobject<UGS_StatComp>(TEXT("StatComp"));
	SkillComp = CreateDefaultSubobject<UGS_SkillComp>(TEXT("SkillComp"));
	DebuffComp = CreateDefaultSubobject<UGS_DebuffComp>(TEXT("DebuffComp"));
}

void AGS_Character::BeginPlay()
{
	Super::BeginPlay();

}

void AGS_Character::ServerRPCMeleeAttack_Implementation(AGS_Character* InDamagedCharacter)
{
	if (IsValid(InDamagedCharacter))
	{
		UGS_StatComp* DamagedCharacterStat = InDamagedCharacter->GetStatComp();
		if (IsValid(DamagedCharacterStat))
		{
			float Damage = DamagedCharacterStat->CalculateDamage(InDamagedCharacter);
			FDamageEvent DamageEvent;
			InDamagedCharacter->TakeDamage(Damage, DamageEvent, GetController(), this);
		}
	}
}

float AGS_Character::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	float CurrentHealth = StatComp->GetCurrentHealth();

	UE_LOG(LogTemp, Warning, TEXT("Damaged"));

	StatComp->SetCurrentHealth(CurrentHealth - ActualDamage);

	return ActualDamage;
}

void AGS_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
