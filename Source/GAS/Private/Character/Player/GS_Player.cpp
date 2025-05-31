#include "Character/Player/GS_Player.h"
#include "Camera/CameraComponent.h"
#include "Character/GS_TpsController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/PostProcessComponent.h"
#include "Character/Component/GS_StatComp.h"
#include "GameFramework/Controller.h"
#include "System/GS_PlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "System/GS_PlayerState.h"

AGS_Player::AGS_Player()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->TargetArmLength = 400.f;
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->bUsePawnControlRotation = false;
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);

	PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessComponent"));
	PostProcessComponent->SetupAttachment(GetRootComponent());
	PostProcessComponent->bUnbound = true; // 시야 안 전체에만 적용할 경우 false
	PostProcessComponent->BlendWeight = 0.f; // 기본은 비활성화

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BlurMat(TEXT("/Game/VFX/MI_AbscureDebuff"));
	if (BlurMat.Succeeded())
	{
		PostProcessMat = BlurMat.Object;
	}

	static ConstructorHelpers::FObjectFinder<UCurveFloat> CurveObj(TEXT("/Game/VFX/ObscureCurve"));
	if (CurveObj.Succeeded())
	{
		ObscureCurve = CurveObj.Object;
	}

	TeamId = FGenericTeamId(1);

	// AkComponent 생성
	AkComponent = CreateDefaultSubobject<UAkComponent>(TEXT("AkComponent"));
	AkComponent->SetupAttachment(GetRootComponent());
}

void AGS_Player::BeginPlay()
{
	Super::BeginPlay();
	if (ObscureCurve)
	{
		FOnTimelineFloat TimelineCallback;
		TimelineCallback.BindUFunction(this, FName("HandleTimelineProgress"));

		FOnTimelineEvent TimelineFinishedCallback;
		TimelineFinishedCallback.BindUFunction(this, FName("OnTimelineFinished"));

		ObscureTimeline.AddInterpFloat(ObscureCurve, TimelineCallback);
		ObscureTimeline.SetTimelineFinishedFunc(TimelineFinishedCallback);
		ObscureTimeline.SetLooping(false);
	}

	BlurMID = UMaterialInstanceDynamic::Create(PostProcessMat, this);
	PostProcessComponent->Settings.WeightedBlendables.Array.Add(FWeightedBlendable(1.0f, BlurMID));

	// 로컬 플레이어만 오디오 리스너 설정
	if (IsLocalPlayer())
	{
		SetupLocalAudioListener();
	}
}

void AGS_Player::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (ObscureTimeline.IsPlaying())
	{
		ObscureTimeline.TickTimeline(DeltaSeconds);
	}
}

void AGS_Player::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AGS_PlayerState* PS = GetPlayerState<AGS_PlayerState>();

	if (PS && StatComp)
	{
		float HealthToRestoreFromPS = PS->CurrentHealth;
		bool bShouldBeAliveFromPS = PS->bIsAlive;

		float ClampedHealthForStatComp = FMath::Clamp(HealthToRestoreFromPS, 0.f, StatComp->GetMaxHealth());

		if (!bShouldBeAliveFromPS)
		{
			ClampedHealthForStatComp = 0.f;
		}
		else if (ClampedHealthForStatComp <= 0.f && StatComp->GetMaxHealth() > KINDA_SMALL_NUMBER)
		{
			ClampedHealthForStatComp = FMath::Min(1.f, StatComp->GetMaxHealth());
		}

		UE_LOG(LogTemp, Warning, TEXT("AGS_Player (%s) PossessedBy: Restoring state from PlayerState. PS.Health: %f, PS.bIsAlive: %s. Setting StatComp Health to: %f"),
			*GetName(), PS->CurrentHealth, PS->bIsAlive ? TEXT("True") : TEXT("False"), ClampedHealthForStatComp);

		StatComp->SetCurrentHealth(ClampedHealthForStatComp, false);
		PS->OnPawnStatInitialized();
	}
	else
	{
		if (!PS) UE_LOG(LogTemp, Error, TEXT("AGS_Player (%s) PossessedBy: PlayerState is NULL!"), *GetName());
		if (!StatComp) UE_LOG(LogTemp, Error, TEXT("AGS_Player (%s) PossessedBy: StatComp is NULL!"), *GetName());
	}
}

void AGS_Player::Client_StartVisionObscured_Implementation()
{
	StartVisionObscured();
}

void AGS_Player::StartVisionObscured()
{ 
	if(GetLocalRole() == ROLE_AutonomousProxy)
	{
		if (!bIsObscuring && ObscureCurve)
		{
			bIsObscuring = true;
			ObscureTimeline.PlayFromStart();
		}

		if (PostProcessComponent)
		{
			PostProcessComponent->BlendWeight = 1.0f;

			if (BlurMID)
			{
				BlurMID->SetScalarParameterValue("InnerRadius", 0.3f); // 완전 차단 반경
				BlurMID->SetScalarParameterValue("OuterRadius", 0.8f); // 차단 종료 반경
			}
		}
	}
}

void AGS_Player::Client_StopVisionObscured_Implementation()
{
	StopVisionObscured();
}

void AGS_Player::StopVisionObscured()
{
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		ObscureTimeline.ReverseFromEnd();
		bIsObscuring = false;
		if (PostProcessComponent)
		{
			PostProcessComponent->BlendWeight = 0.0f;
		}
	}
	
}

void AGS_Player::HandleTimelineProgress(float Value)
{
	UE_LOG(LogTemp, Warning, TEXT("Timeline Progress: %f"), Value);

	if (PostProcessComponent)
	{
		PostProcessComponent->BlendWeight = 1.0f;

		if (BlurMID)
		{
			BlurMID->SetScalarParameterValue("InnerRadius", Value);
			BlurMID->SetScalarParameterValue("OuterRadius", 1.1f);
		}
	}
}

void AGS_Player::OnTimelineFinished()
{
	bIsObscuring = false;
}

void AGS_Player::OnDeath()
{
	Super::OnDeath();

	// 죽음 사운드 재생
	if (DeathSoundEvent)
	{
		PlaySound(DeathSoundEvent);
	}

	// 추가적인 플레이어 죽음 처리 로직을 여기에 구현할 수 있다
	// 예: 카메라 연출, UI 변경, 리스폰 타이머 등
	
	GetCharacterMovement()->DisableMovement();

	AGS_PlayerState* GS_PS = Cast<AGS_PlayerState>(GetPlayerState());
	if (GS_PS)
	{
		GS_PS->bIsAlive = false;
	}
	SpectateNextPlayer();
}

void AGS_Player::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AGS_Player::PlaySkillMontage(UAnimMontage* Montage)
{
	if (this->GetMesh() && Montage)
	{
		this->GetMesh()->GetAnimInstance()->Montage_Play(Montage);
		UE_LOG(LogTemp, Warning, TEXT("Skill Montage Play!!!!!!!!!!!!!!"));
	}
}

FCharacterWantsToMove AGS_Player::GetWantsToMove()
{
	return WantsToMove;
}

void AGS_Player::SetupLocalAudioListener()
{
	if (!AkComponent)
	{
		UE_LOG(LogAudio, Warning, TEXT("AkComponent is null in SetupLocalAudioListener"));
		return;
	}

	// 로컬 플레이어 확인
	if (!IsLocalPlayer())
	{
		UE_LOG(LogAudio, Warning, TEXT("SetupLocalAudioListener called on non-local player: %s"), *GetName());
		return;
	}

	// 가장 기본적인 방법: AkComponent를 직접 사용 : 이 방법이 가장 안전하고 호환성이 좋다
	UE_LOG(LogAudio, Log, TEXT("Setting up audio listener for local player: %s"), *GetName());
    
	// AkComponent가 자동으로 리스너 역할을 하도록 설정. 별도의 API 호출 없이 AkComponent 자체가 리스너가 된다
}

// 로컬 플레이어 확인 함수
bool AGS_Player::IsLocalPlayer() const
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		return PC->IsLocalController();
	}
	return false;
}

void AGS_Player::SpectateNextPlayer()
{
	for (const auto& PS : GetWorld()->GetGameState()->PlayerArray)
	{
		AGS_PlayerState* GS_PS = Cast<AGS_PlayerState>(PS);
		if (IsValid(GS_PS))
		{
			if (GS_PS->bIsAlive) //[TODO] GS_PS->CurrentPlayerRole == EPlayerRole::PR_Seeker 
			{
				AGS_TpsController* AlivePlayerController = Cast<AGS_TpsController>(GS_PS->GetPlayerController());
				
				if (IsValid(AlivePlayerController))
				{
					APlayerController* DeadPlayerPC = Cast<APlayerController>(GetController());
					if (DeadPlayerPC)
					{
						DeadPlayerPC->UnPossess();
						DeadPlayerPC->SetViewTargetWithBlend(AlivePlayerController);
						SetLifeSpan(2.f);
					}
				}
			}
		}
		//Game Over?
		//all players dead
	}
}

void AGS_Player::ServerRPCSpectateNextPlayer_Implementation()
{
	SpectateNextPlayer();
}


void AGS_Player::Multicast_PlaySkillMontage_Implementation(UAnimMontage* Montage)
{
	PlaySkillMontage(Montage);
}

void AGS_Player::PlaySound(UAkAudioEvent* SoundEvent)
{
	if (!AkComponent || !SoundEvent)
	{
		UE_LOG(LogAudio, Warning, TEXT("AkComponent or SoundEvent is null in PlaySound"));
		return;
	}

	AkComponent->PostAkEvent(SoundEvent);
}

void AGS_Player::PlaySoundWithCallback(UAkAudioEvent* SoundEvent, const FOnAkPostEventCallback& Callback)
{
	if (!AkComponent || !SoundEvent)
	{
		UE_LOG(LogAudio, Warning, TEXT("AkComponent or SoundEvent is null in PlaySoundWithCallback"));
		return;
	}

	AkComponent->PostAkEvent(SoundEvent, 0, Callback);
}