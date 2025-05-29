#include "Character/GS_Character.h"
#include "Character/Component/GS_StatComp.h"
#include "Character/Component/GS_DebuffComp.h"
#include "Character/Skill/GS_SkillComp.h"
#include "UI/Character/GS_HPTextWidgetComp.h"
#include "UI/Character/GS_HPText.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "UI/Character/GS_HPWidget.h"
#include "Weapon/GS_Weapon.h"

AGS_Character::AGS_Character()
{
	PrimaryActorTick.bCanEverTick = true;

	StatComp = CreateDefaultSubobject<UGS_StatComp>(TEXT("StatComp"));
	SkillComp = CreateDefaultSubobject<UGS_SkillComp>(TEXT("SkillComp"));
	DebuffComp = CreateDefaultSubobject<UGS_DebuffComp>(TEXT("DebuffComp"));
	
	HPTextWidgetComp = CreateDefaultSubobject<UGS_HPTextWidgetComp>(TEXT("TextWidgetComp"));
	HPTextWidgetComp->SetupAttachment(RootComponent);
	HPTextWidgetComp->SetWidgetSpace(EWidgetSpace::World);
	HPTextWidgetComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HPTextWidgetComp->SetVisibility(false);
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
	
	if (HPTextWidgetComp->GetOwner()->ActorHasTag("Monster"))
	{
		HPTextWidgetComp->SetVisibility(true);
	}

	if (HasAuthority())
	{
		SpawnAndAttachWeapons();
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

void AGS_Character::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGS_Character, WeaponSlots);
}

float AGS_Character::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	float CurrentHealth = StatComp->GetCurrentHealth();

	UE_LOG(LogTemp, Warning, TEXT("%s Damaged %f by %s"), *GetName(), ActualDamage, *DamageCauser->GetName());

	StatComp->SetCurrentHealth(CurrentHealth - ActualDamage, false);

	return ActualDamage;
}

void AGS_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AGS_Character::OnDeath()
{
	MulticastRPCCharacterDeath();
}

void AGS_Character::WatchOtherPlayer()
{
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

void AGS_Character::SetHPBarWidget(UGS_HPWidget* InHPBarWidget)
{
	UGS_HPWidget* HPBarWidget = Cast<UGS_HPWidget>(InHPBarWidget);
	if (IsValid(HPBarWidget))
	{
		HPBarWidget->InitializeHPWidget(GetStatComp());
		StatComp->OnCurrentHPChanged.AddUObject(HPBarWidget, &UGS_HPWidget::OnCurrentHPBarChanged);
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

bool AGS_Character::IsEnemy(const AGS_Character* Other) const
{
	return Other && GetGenericTeamId()!= Other->GetGenericTeamId();
}

AGS_Weapon* AGS_Character::GetWeaponByIndex(int32 Index) const
{
	return WeaponSlots.IsValidIndex(Index) ? WeaponSlots[Index].WeaponInstance : nullptr;
}

void AGS_Character::MulticastRPCCharacterDeath_Implementation()
{
	 GetMesh()->SetSimulatePhysics(true);
	 GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
}

void AGS_Character::MulticastRPCPlaySkillMontage_Implementation(UAnimMontage* SkillMontage)
{
	if (!HasAuthority())
	{
		PlayAnimMontage(SkillMontage);
	}
}

void AGS_Character::MulicastRPCStopCurrentSkillMontage_Implementation(UAnimMontage* CurrentSkillMontage)
{
	if (!HasAuthority())
	{
		StopAnimMontage(CurrentSkillMontage);
	}
}

void AGS_Character::SpawnAndAttachWeapons()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	
	for (FWeaponSlot& Slot : WeaponSlots)
	{
		if (!Slot.WeaponClass) continue;

		FActorSpawnParameters Params;
		Params.Owner = this;
		Slot.WeaponInstance = World->SpawnActor<AGS_Weapon>(Slot.WeaponClass, Params);
		if (!Slot.WeaponInstance) continue;

		Slot.WeaponInstance->AttachToComponent(
			GetMesh(),
			FAttachmentTransformRules::SnapToTargetIncludingScale,
			Slot.SocketName);
	}
}