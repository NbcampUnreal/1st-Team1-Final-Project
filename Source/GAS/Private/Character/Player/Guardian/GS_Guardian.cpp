#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Player/Guardian/GS_DrakharAnimInstance.h"
#include "Animation/AnimInstance.h"
#include "Character/Component/GS_StatComp.h"
#include "Character/Player/Guardian/GS_Drakhar.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Character/Component/GS_CameraShakeComponent.h"
#include "Character/Component/GS_DebuffVFXComponent.h"
#include "Props/Interactables/GS_BridgePiece.h"
#include "Components/WidgetComponent.h"

AGS_Guardian::AGS_Guardian()
{
	PrimaryActorTick.bCanEverTick = false;
	
	CameraShakeComponent = CreateDefaultSubobject<UGS_CameraShakeComponent>(TEXT("CameraShakeComponent"));

	NormalMoveSpeed = GetCharacterMovement()->MaxWalkSpeed;
	SpeedUpMoveSpeed = 1200.f;

	//boss monster tag for user widget
	Tags.Add("Guardian");
	
	//디버프 VFX 컴포넌트 생성
	DebuffVFXComponent = CreateDefaultSubobject<UGS_DebuffVFXComponent>("DebuffVFXComponent");

	// 컴포넌트 생성 및 초기화
	TargetedUIComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("TargetedUI"));
	TargetedUIComponent->SetupAttachment(RootComponent);
	TargetedUIComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	TargetedUIComponent->SetWidgetSpace(EWidgetSpace::Screen);
	TargetedUIComponent->SetDrawSize(FVector2D(100.f, 100.f));
	TargetedUIComponent->SetVisibility(false);

	//AetherComp 연결
	AetherComp = CreateDefaultSubobject<UGS_AetherComp>(TEXT("Aether"));
}

void AGS_Guardian::BeginPlay()
{
	Super::BeginPlay();
}

void AGS_Guardian::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	GuardianAnim = Cast<UGS_DrakharAnimInstance>(GetMesh()->GetAnimInstance());
}

void AGS_Guardian::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, GuardianState);
	DOREPLIFETIME(ThisClass, GuardianDoSkillState);
	DOREPLIFETIME(ThisClass, MoveSpeed);
}

void AGS_Guardian::LeftMouse()
{
}

void AGS_Guardian::Ctrl()
{
}

void AGS_Guardian::CtrlStop()
{
}

void AGS_Guardian::RightMouse()
{
}

void AGS_Guardian::StopCtrl()
{
	
}

void AGS_Guardian::OnRep_MoveSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void AGS_Guardian::MeleeAttackCheck()
{
	if (HasAuthority())
	{
		GuardianState = EGuardianState::CtrlEnd;

		const FVector Start = GetActorLocation() + GetActorForwardVector() * GetCapsuleComponent()->GetScaledCapsuleRadius();
		const float MeleeAttackRange = 200.f;
		const float MeleeAttackRadius = 200.f;
		
		TSet<AGS_Character*> DamagedCharacters = DetectPlayerInRange(Start, MeleeAttackRange, MeleeAttackRadius);
		ApplyDamageToDetectedPlayer(DamagedCharacters, 0.f);
	}
}

TSet<AGS_Character*> AGS_Guardian::DetectPlayerInRange(const FVector& Start, float SkillRange, float Radius)
{
	TArray<FHitResult> OutHitResults;
	TSet<AGS_Character*> DamagedPlayers;
	FCollisionQueryParams Params(NAME_None, false, this);
	Params.AddIgnoredActor(this);

	FVector End = Start + GetActorForwardVector() * SkillRange;
	
	bool bIsHitDetected = GetWorld()->SweepMultiByChannel(OutHitResults, End, End, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(Radius), Params);

	//FOR DEBUGGING
	//MulticastRPCDrawDebugSphere(bIsHitDetected, End, Radius);
	
	if (bIsHitDetected)
	{
		for (auto const& OutHitResult : OutHitResults)
		{
			if (OutHitResult.GetComponent() && OutHitResult.GetComponent()->GetCollisionProfileName() == FName("SoundTrigger"))
			{
				continue;
			}
			AGS_Character* DamagedCharacter = Cast<AGS_Character>(OutHitResult.GetActor());
			if (IsValid(DamagedCharacter))
			{
				DamagedPlayers.Add(DamagedCharacter);
			}
			//break bridge
			if (OutHitResult.GetActor()->IsA<AGS_BridgePiece>())
			{
				AGS_BridgePiece* BridgePiece = Cast<AGS_BridgePiece>(OutHitResult.GetActor());
				if (BridgePiece)
				{
					BridgePiece->BrokeBridge(100.f);
				}
			}
		}
	}

	return DamagedPlayers;
}

void AGS_Guardian::ApplyDamageToDetectedPlayer(const TSet<AGS_Character*>& DamagedCharacters, float PlusDamge)
{
	for (auto const& DamagedCharacter : DamagedCharacters)
	{
		//[TODO] only damage logic in server 
		//ServerRPCMeleeAttack(DamagedCharacter);
		
		UGS_StatComp* DamagedCharacterStat = DamagedCharacter->GetStatComp();
		if (IsValid(DamagedCharacterStat))
		{
			float Damage = DamagedCharacterStat->CalculateDamage(this, DamagedCharacter);
			FDamageEvent DamageEvent;
			DamagedCharacter->TakeDamage(Damage + PlusDamge, DamageEvent, GetController(),this);

			//hit stop
			MulticastRPCApplyHitStop(DamagedCharacter);
			
			//server
			AGS_Drakhar* Drakhar = Cast<AGS_Drakhar>(this);
			
			if (!Drakhar->GetIsFeverMode())
			{
				Drakhar->SetFeverGauge(10.f);
			}
			else if (Drakhar->GetIsFeverMode())
			{
				Drakhar->bIsAttckingDuringFever = true;
				Drakhar->ResetIsAttackingDuringFeverMode();
			}
			
			// === 히트 사운드 재생 (Drakhar인 경우) ===
			if (Drakhar)
			{
				Drakhar->MulticastPlayAttackHitSound();
			}
		}
	}
}


void AGS_Guardian::OnRep_GuardianState()
{
	ClientGuardianState = GuardianState;
}

void AGS_Guardian::OnRep_GuardianDoSkillState()
{
	ClientGuardianDoSkillState = GuardianDoSkillState;
}

void AGS_Guardian::QuitGuardianSkill()
{
	//reset skill state
	GuardianState = EGuardianState::CtrlEnd;
	GuardianDoSkillState = EGuardianDoSkill::None;
	
	AGS_Drakhar* Drakhar = Cast<AGS_Drakhar>(this);
	if (Drakhar)
	{
		Drakhar->ServerRPCResetValue();
	}
	//fly end
	GetSkillComp()->Server_TryDeactiveSkill(ESkillSlot::Ready);
}

void AGS_Guardian::FinishCtrlSkill()
{
	StopCtrl();
}

void AGS_Guardian::ShowTargetUI(bool bIsActive)
{
	if (TargetedUIComponent)
	{
		TargetedUIComponent->SetVisibility(bIsActive);
	}
}

void AGS_Guardian::MulticastRPCApplyHitStop_Implementation(AGS_Character* InDamagedCharacter)
{
	if (HasAuthority())
	{
		if (CameraShakeComponent)
		{
			CameraShakeComponent->PlayCameraShake(HitStopShakeInfo);
		}
	}
	if (!HasAuthority())
	{
		if (!IsValid(InDamagedCharacter))
		{
			return;
		}
		CustomTimeDilation = 0.1f;
		InDamagedCharacter->CustomTimeDilation = 0.1f;

		FTimerDelegate HitStopTimerDelegate;
		FTimerHandle HitStopTimerHandle;
		HitStopTimerDelegate.BindUFunction(this, FName("MulticastRPCEndHitStop"), InDamagedCharacter);
		GetWorld()->GetTimerManager().SetTimer(HitStopTimerHandle, HitStopTimerDelegate, HitStopDurtaion, false);
	}
}

void AGS_Guardian::MulticastRPCEndHitStop_Implementation(AGS_Character* InDamagedCharacter)
{
	if (!HasAuthority())
	{
		CustomTimeDilation = 1.f;
		InDamagedCharacter->CustomTimeDilation = 1.f;
	}
}

void AGS_Guardian::MulticastRPCDrawDebugSphere_Implementation(bool bIsOverlap, const FVector& Location, float CapsuleRadius)
{
	FColor DebugColor = bIsOverlap ? FColor::Green : FColor::Red;
	DrawDebugSphere(GetWorld(), Location,  CapsuleRadius, 16,DebugColor, false, 2.0f, 0, 1.0f);
}
