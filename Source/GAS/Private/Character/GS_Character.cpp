#include "Character/GS_Character.h"

#include "Character/Component/GS_StatComp.h"
#include "Character/Component/GS_DebuffComp.h"
#include "Character/Skill/GS_SkillComp.h"
#include "UI/Character/GS_HPTextWidgetComp.h"
#include "UI/Character/GS_HPText.h"

#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

AGS_Character::AGS_Character()
{
	PrimaryActorTick.bCanEverTick = false;

	StatComp = CreateDefaultSubobject<UGS_StatComp>(TEXT("StatComp"));
	SkillComp = CreateDefaultSubobject<UGS_SkillComp>(TEXT("SkillComp"));
	DebuffComp = CreateDefaultSubobject<UGS_DebuffComp>(TEXT("DebuffComp"));
	
	HPTextWidgetComp = CreateDefaultSubobject<UGS_HPTextWidgetComp>(TEXT("TextWidgetComp"));
	HPTextWidgetComp->SetupAttachment(RootComponent);
	HPTextWidgetComp->SetWidgetSpace(EWidgetSpace::World);
	HPTextWidgetComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AGS_Character::BeginPlay()
{
	Super::BeginPlay();

	//Set Default Stats to Character
	const UEnum* CharacterEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT("ECharacterType"), true);

	if (CharacterEnum)
	{
		FString EnumToName = CharacterEnum->GetNameStringByValue((int64)CharacterType);
		StatComp->InitStat(FName(EnumToName));
	}
}

void AGS_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsValid(HPTextWidgetComp) && !HasAuthority())
	{
		FVector WidgetComponentLocation = HPTextWidgetComp->GetComponentLocation();
		FVector LocalPlayerCameraLocation = UGameplayStatics::GetPlayerCameraManager(this, 0)->GetCameraLocation();
		HPTextWidgetComp->SetWorldRotation(UKismetMathLibrary::FindLookAtRotation(WidgetComponentLocation, LocalPlayerCameraLocation));
	}

}

float AGS_Character::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	float CurrentHealth = StatComp->GetCurrentHealth();

	UE_LOG(LogTemp, Warning, TEXT("%s Damaged %f"), *GetName(), ActualDamage);

	StatComp->SetCurrentHealth(CurrentHealth - ActualDamage);

	return ActualDamage;
}

void AGS_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AGS_Character::SetHPTextWidget(UGS_HPText* InHPTextWidget)
{
	UGS_HPText* HPTextWidget = Cast<UGS_HPText>(InHPTextWidget);
	if (IsValid(HPTextWidget))
	{
		HPTextWidget->InitializeHPTextWidget(GetStatComp());
		StatComp->OnCurrentHPChanged.AddUObject(HPTextWidget, &UGS_HPText::OnCurrentHPChanged);
	}
}

void AGS_Character::ServerRPCMeleeAttack_Implementation(AGS_Character* InDamagedCharacter)
{
	if (IsValid(InDamagedCharacter))
	{
		UGS_StatComp* DamagedCharacterStat = InDamagedCharacter->GetStatComp();
		if (IsValid(DamagedCharacterStat))
		{
			float Damage = DamagedCharacterStat->CalculateDamage(this, InDamagedCharacter);
			FDamageEvent DamageEvent;
			InDamagedCharacter->TakeDamage(Damage, DamageEvent, GetController(), this);
		}
	}
}

FGenericTeamId AGS_Character::GetGenericTeamId() const
{
	return TeamId;
}
