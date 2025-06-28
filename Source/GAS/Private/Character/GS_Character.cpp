#include "Character/GS_Character.h"
#include "Character/Component/GS_StatComp.h"
#include "Character/Component/GS_DebuffComp.h"
#include "UI/Character/GS_HPTextWidgetComp.h"
#include "UI/Character/GS_HPText.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "UI/Character/GS_HPWidget.h"
#include "System/GS_PlayerState.h"
#include "Weapon/GS_Weapon.h"
#include "AkGameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/Component/GS_HitReactComp.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Character/Player/GS_Player.h"
#include "UI/Character/GS_PlayerInfoWidget.h"

AGS_Character::AGS_Character()
{
	PrimaryActorTick.bCanEverTick = true;

	StatComp = CreateDefaultSubobject<UGS_StatComp>(TEXT("StatComp"));
	DebuffComp = CreateDefaultSubobject<UGS_DebuffComp>(TEXT("DebuffComp"));
	HitReactComp = CreateDefaultSubobject<UGS_HitReactComp>(TEXT("HitReactComp"));
	
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
	bool bStatInitialized = false;

	if (CharacterEnum)
	{
		FString EnumToName = CharacterEnum->GetNameStringByValue((int64)CharacterType);
		StatComp->InitStat(FName(EnumToName));
		bStatInitialized = true;
	}
	if (bStatInitialized)
	{
		AGS_PlayerState* PS = GetPlayerState<AGS_PlayerState>();
		if (PS)
		{
			if (PS->CurrentPlayerRole == EPlayerRole::PR_Seeker)
			{
				PS->OnPawnStatInitialized();
			}
		}
	}

	//Set 3D widget
	if (IsValid(HPTextWidgetComp) && !HPTextWidgetComp->GetOwner()->ActorHasTag("Guardian"))
	{
		HPTextWidgetComp->SetVisibility(true);
	}
	
	DefaultCharacterSpeed = this->GetCharacterMovement()->MaxWalkSpeed;
	//CharacterSpeed = DefaultCharacterSpeed;

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
		APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
		
		FVector CameraForward = CameraManager->GetCameraRotation().Vector();
		FVector CameraRight = FVector::CrossProduct(CameraForward, FVector::UpVector).GetSafeNormal();
		FVector CameraUp = FVector::CrossProduct(CameraRight, CameraForward).GetSafeNormal();
		FRotator WidgetRotation = UKismetMathLibrary::MakeRotFromXZ(-CameraForward, CameraUp);
        
		HPTextWidgetComp->SetWorldRotation(WidgetRotation);
	}
}

void AGS_Character::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGS_Character, WeaponSlots);
	DOREPLIFETIME(AGS_Character, CharacterSpeed);
	DOREPLIFETIME(AGS_Character, bIsDead);
}

float AGS_Character::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	float CurrentHealth = StatComp->GetCurrentHealth();

	//when damage input start -> for drakhar 6/24
	OnDamageStart();
	
	// SJE
	if (CanHitReact && DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		const FPointDamageEvent* PointEvent = static_cast<const FPointDamageEvent*>(&DamageEvent);
		FVector HitDirection = -PointEvent->ShotDirection;
		if(UGS_HitReactComp* HitReactComponent = GetComponentByClass<UGS_HitReactComp>())
		{
			HitReactComponent->PlayHitReact(EHitReactType::Interrupt, HitDirection);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("%s Damaged %f by %s"), *GetName(), ActualDamage, DamageCauser ? *DamageCauser->GetName() : TEXT("Unknown"));

	float NewHealth = CurrentHealth - ActualDamage;
	StatComp->SetCurrentHealth(NewHealth, false);

	return ActualDamage;
}

void AGS_Character::OnDamageStart()
{
	//
}

void AGS_Character::Multicast_SetCanHitReact_Implementation(bool CanReact)
{
	CanHitReact = CanReact;
}

void AGS_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AGS_Character::OnDeath()
{
	bIsDead = true;

	OnDeathDelegate.Broadcast();

	// 죽음 사운드 재생
	if (DeathSoundEvent)
	{
		UAkGameplayStatics::PostEvent(DeathSoundEvent, this, 0, FOnAkPostEventCallback());
	}

	DestroyAllWeapons();
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

void AGS_Character::SetPlayerInfoWidget(UGS_PlayerInfoWidget* InPlayerInfoWidget)
{
	if (IsValid(InPlayerInfoWidget))
	{
		InPlayerInfoWidget->InitializePlayerInfoWidget(Cast<AGS_Player>(this));
		StatComp->OnCurrentHPChanged.AddUObject(InPlayerInfoWidget, &UGS_PlayerInfoWidget::OnCurrentHPBarChanged);
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

void AGS_Character::SetCharacterSpeed(float InRatio)
{
	if (InRatio >= 0 && InRatio <= 1)
	{
		CharacterSpeed = DefaultCharacterSpeed * InRatio;
		GetCharacterMovement()->MaxWalkSpeed = CharacterSpeed;
	}
}

bool AGS_Character::IsDead() const
{
	return bIsDead;
}

void AGS_Character::Server_SetCharacterSpeed_Implementation(float InRatio)
{
	CharacterSpeed = DefaultCharacterSpeed * InRatio;

	if (HasAuthority())
	{
		OnRep_CharacterSpeed(); // 서버도 직접 반영
	}
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

void AGS_Character::Multicast_PlayImpactVFX_Implementation(UNiagaraSystem* VFXAsset, FVector Scale)
{
	if (VFXAsset)
	{
		UNiagaraComponent* SpawnedVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
			VFXAsset,
			GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true
		);

		if(SpawnedVFX)
		{
			SpawnedVFX->SetWorldScale3D(Scale);
		}
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
		if (!Slot.WeaponClass)
		{
			continue;
		}

		FActorSpawnParameters Params;
		Params.Owner = this;
		Slot.WeaponInstance = World->SpawnActor<AGS_Weapon>(Slot.WeaponClass, Params);
		if (!Slot.WeaponInstance)
		{
			continue;
		}

		Slot.WeaponInstance->AttachToComponent(
			GetMesh(),
			FAttachmentTransformRules::SnapToTargetIncludingScale,
			Slot.SocketName);
	}
}

void AGS_Character::DestroyAllWeapons()
{
	if (!HasAuthority())
	{
		return;
	}
	
	for (FWeaponSlot& Slot : WeaponSlots)
	{
		if (!Slot.WeaponInstance)
		{
			continue;
		}
		
		Slot.WeaponInstance->Destroy();
	}
}


void AGS_Character::OnRep_CharacterSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = CharacterSpeed;
}
