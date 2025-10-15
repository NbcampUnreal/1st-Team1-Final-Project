#include "Character/Player/GS_Player.h"
#include "Animation/Character/GS_SeekerAnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Character/GS_TpsController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/PostProcessComponent.h"
#include "Character/Component/GS_StatComp.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Controller.h"
#include "System/GS_PlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "UI/Character/GS_SteamNameWidgetComp.h"
#include "AkAudioDevice.h"

AGS_Player::AGS_Player()
{
	PrimaryActorTick.bCanEverTick = true;

	SkillComp = CreateDefaultSubobject<UGS_SkillComp>(TEXT("SkillComp"));

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

	//steam name widget
	SteamNameWidgetComp = CreateDefaultSubobject<UGS_SteamNameWidgetComp>(TEXT("SteamWidgetComp"));
	SteamNameWidgetComp->SetupAttachment(RootComponent);
	SteamNameWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	SteamNameWidgetComp->GetBodyInstance()->TermBody(); 
	SteamNameWidgetComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SteamNameWidgetComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	
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

	// 머리 위치 오디오 리스너 컴포넌트 생성
	HeadAudioListenerComponent = CreateDefaultSubobject<UAkComponent>(TEXT("HeadAudioListenerComponent"));
	// 스켈레탈 메시가 아직 설정되지 않았으므로 RootComponent에 임시로 attach
	HeadAudioListenerComponent->SetupAttachment(GetRootComponent());

	// 기본 후보 세팅 (프로젝트 표준 본/소켓명 우선순위)
	HeadListenerCandidates = {
		TEXT("head"),
		TEXT("Head"),
		TEXT("neck_01"),
		TEXT("spine_03")
	};

	bIsObscuring = false;
}

void AGS_Player::BeginPlay()
{
	Super::BeginPlay();

	// 오디오 디바이스 캐싱
	CachedAudioDevice = FAkAudioDevice::Get();
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

	// 머리 위치에 오디오 리스너 설정 (모든 클라이언트에서)
	SetupHeadAudioListener();
	
	// 로컬 플레이어만 추가 오디오 설정
	if (IsLocalPlayer())
	{
		// 자체 AkComponent의 Occlusion도 비활성화
		if (AkComponent)
		{
			AkComponent->OcclusionRefreshInterval = 0.0f;
			UE_LOG(LogTemp, Warning, TEXT("AGS_Player: Player AkComponent occlusion DISABLED."));
		}
	}
}

void AGS_Player::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (ObscureTimeline.IsPlaying())
	{
		ObscureTimeline.TickTimeline(DeltaSeconds);
	}

	//steam widget rotate
	if (IsValid(SteamNameWidgetComp) && !HasAuthority())
	{
		UpdateSteamNameWidgetRotation();
	}
}

void AGS_Player::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AGS_PlayerState* PS = GetPlayerState<AGS_PlayerState>();

	if (PS && StatComp)
	{
		StatComp->SetCurrentHealth(PS->CurrentHealth, true);
		PS->OnPawnStatInitialized();
		UE_LOG(LogTemp, Warning, TEXT("AGS_Player::PossessedBy: Synced StatComp health from PlayerState. PS Health: %f, StatComp Health set to: %f"), PS->CurrentHealth, StatComp->GetCurrentHealth());
	}
	else
	{
		if (!PS) UE_LOG(LogTemp, Error, TEXT("AGS_Player (%s) PossessedBy: PlayerState is NULL!"), *GetName());
		if (!StatComp) UE_LOG(LogTemp, Error, TEXT("AGS_Player (%s) PossessedBy: StatComp is NULL!"), *GetName());
	}
}

void AGS_Player::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	//// 2. 가시성 끄기
	//SteamNameWidgetComp->SetVisibility(false);

	//// 3. 콜리전 비활성화
	//SteamNameWidgetComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//// 4. BodySetup 정리
	//if (SteamNameWidgetComp->GetBodySetup())
	//{
	//	SteamNameWidgetComp->DestroyPhysicsState();
	//}
	//
	//if (IsValid(SteamNameWidgetComp))
	//{
	//	if (UUserWidget* Widget = SteamNameWidgetComp->GetWidget())
	//	{
	//		Widget->RemoveFromParent();
	//	}
	//	SteamNameWidgetComp->SetWidget(nullptr);
	//	SteamNameWidgetComp->DestroyComponent();
	//}
	//
	Super::EndPlay(EndPlayReason);
}

void AGS_Player::BeginDestroy()
{
	// 1. 먼저 Super::BeginDestroy() 호출 (중요!)
	Super::BeginDestroy();

	//// 2. IsValid() 체크와 함께 안전하게 정리
	//if (IsValid(SteamNameWidgetComp) && !SteamNameWidgetComp->IsBeingDestroyed())
	//{
	//	SteamNameWidgetComp->SetWidget(nullptr);
	//	SteamNameWidgetComp->SetVisibility(false);
	//	SteamNameWidgetComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//	// BodySetup 정리 (필요한 경우만)
	//	if (SteamNameWidgetComp->GetBodySetup())
	//	{
	//		SteamNameWidgetComp->DestroyPhysicsState();
	//	}

	//	// DestroyComponent() 호출하지 않음! - 자동으로 소멸됨
	//}

	// 3. 다른 참조들도 안전하게 정리
	if (IsValid(AkComponent) && !AkComponent->IsBeingDestroyed())
	{
		AkComponent->Stop();
	}

	// 4. HeadAudioListenerComponent 정리
	if (IsValid(HeadAudioListenerComponent) && !HeadAudioListenerComponent->IsBeingDestroyed())
	{
		// 기본 리스너에서 제거
		FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
		if (AudioDevice && HeadAudioListenerComponent->IsDefaultListener)
		{
			AudioDevice->RemoveDefaultListener(HeadAudioListenerComponent);
		}
		HeadAudioListenerComponent->Stop();
	}

	// 5. 타임라인 정리
	if (ObscureTimeline.IsPlaying())
	{
		ObscureTimeline.Stop();
	}

	// 6. 다이나믹 머티리얼 참조 해제
	BlurMID = nullptr;

	UE_LOG(LogTemp, Warning, TEXT("AGS_Player::BeginDestroy() completed for %s"), *GetName());
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

void AGS_Player::Multicast_SetUseControllerRotationYaw_Implementation(bool UseControlRotationYaw)
{
	bUseControllerRotationYaw = UseControlRotationYaw;
}

void AGS_Player::OnDeath()
{
	Super::OnDeath();

	// 추가적인 플레이어 죽음 처리 로직을 여기에 구현할 수 있다
	// 예: 카메라 연출, UI 변경, 리스폰 타이머 등
	
	GetCharacterMovement()->DisableMovement();

	AGS_PlayerState* GS_PS = Cast<AGS_PlayerState>(GetPlayerState());
	if (GS_PS)
	{
		GS_PS->SetIsAlive(false);
	}
	AGS_TpsController* GS_PC = Cast<AGS_TpsController>(GetController());
	if (IsValid(GS_PC) && GS_PS->CurrentPlayerRole == EPlayerRole::PR_Seeker)
	{
		GS_PC->ServerRPCSpectatePlayer();
	}
}

void AGS_Player::SetSkillInputControl(bool CanLeftClick, bool CanRightClick, bool CanRollClick, bool CanCtrlClick)
{
	SkillInputControl.CanInputLC = CanLeftClick;
	SkillInputControl.CanInputRC = CanRightClick;
	SkillInputControl.CanInputRoll= CanRollClick;
	SkillInputControl.CanInputCtrl = CanCtrlClick;
}

FSkillInputControl AGS_Player::GetSkillInputControl()
{
	return SkillInputControl;
}

void AGS_Player::SetCanUseSkill(bool bCanUse)
{
	if (SkillComp)
	{
		SkillComp->SetCanUseSkill(bCanUse);
	}
}

void AGS_Player::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGS_Player, SkillInputControl);
}

void AGS_Player::Multicast_StopSkillMontage_Implementation(UAnimMontage* Montage)
{
	StopAnimMontage(Montage);
}

void AGS_Player::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AGS_Player::SetupLocalAudioListener()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC && PC->IsLocalPlayerController())
	{
		// 카메라 매니저에서 기본 리스너 컴포넌트를 찾습니다.
		APlayerCameraManager* CameraManager = PC->PlayerCameraManager;
		if (CameraManager)
		{
			UAkComponent* ListenerComponent = nullptr;
			TArray<UAkComponent*> AkComponents;
			CameraManager->GetComponents<UAkComponent>(AkComponents);

			for (UAkComponent* Component : AkComponents)
			{
				if (Component && Component->IsDefaultListener)
				{
					ListenerComponent = Component;
					break;
				}
			}

			// 만약 카메라 매니저에 리스너가 없다면 새로 생성하여 추가합니다.
			if (!ListenerComponent)
			{
				ListenerComponent = NewObject<UAkComponent>(CameraManager);
				if (ListenerComponent)
				{
					ListenerComponent->AttachToComponent(CameraManager->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
					ListenerComponent->RegisterComponent();
					FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
					if(AudioDevice)
					{
						AudioDevice->AddDefaultListener(ListenerComponent);
					}
				}
			}

			if (ListenerComponent)
			{
				// 가장 중요한 부분: 카메라 리스너의 Occlusion을 비활성화합니다.
				ListenerComponent->OcclusionRefreshInterval = 0.0f;
				UE_LOG(LogTemp, Warning, TEXT("AGS_Player: Camera audio listener occlusion DISABLED for local player."));
			}
		}
	}
}

void AGS_Player::SetupHeadAudioListener()
{
	if (!HeadAudioListenerComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("AGS_Player::SetupHeadAudioListener: HeadAudioListenerComponent is null!"));
		return;
	}

	// 스켈레탈 메시에서 머리 본이나 head 소켓을 찾아 리스너를 부착
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (MeshComp)
	{
		bool bAttached = false;
		for (const FName& Candidate : HeadListenerCandidates)
		{
			if (MeshComp->DoesSocketExist(Candidate))
			{
				HeadAudioListenerComponent->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetIncludingScale, Candidate);
				// UE_LOG(LogTemp, Warning, TEXT("AGS_Player: Head audio listener attached to socket '%s'."), *Candidate.ToString());
				bAttached = true;
				break;
			}
			if (MeshComp->GetBoneIndex(Candidate) != INDEX_NONE)
			{
				HeadAudioListenerComponent->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetIncludingScale, Candidate);
				// UE_LOG(LogTemp, Warning, TEXT("AGS_Player: Head audio listener attached to bone '%s'."), *Candidate.ToString());
				bAttached = true;
				break;
			}
		}

		if (!bAttached)
		{
			HeadAudioListenerComponent->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetIncludingScale);
			HeadAudioListenerComponent->SetRelativeLocation(FVector(0.0f, 0.0f, HeadListenerZOffset));
			// UE_LOG(LogTemp, Warning, TEXT("AGS_Player: Head audio listener attached to mesh root with offset Z=%.1f."), HeadListenerZOffset);
		}

		// 로컬 플레이어만 기본 리스너로 설정
		if (IsLocalPlayer())
		{
			if (CachedAudioDevice)
			{
				// 기존 카메라 리스너 제거
				APlayerController* PC = Cast<APlayerController>(GetController());
				if (PC && PC->PlayerCameraManager)
				{
					TArray<UAkComponent*> CameraAkComponents;
					PC->PlayerCameraManager->GetComponents<UAkComponent>(CameraAkComponents);
					for (UAkComponent* Component : CameraAkComponents)
					{
						if (Component && Component->IsDefaultListener)
						{
							CachedAudioDevice->RemoveDefaultListener(Component);
							// UE_LOG(LogTemp, Warning, TEXT("AGS_Player: Removed camera audio listener."));
						}
					}
				}

				// 새로운 머리 위치 리스너를 기본 리스너로 설정
				CachedAudioDevice->AddDefaultListener(HeadAudioListenerComponent);
				HeadAudioListenerComponent->OcclusionRefreshInterval = 0.0f; // Occlusion 비활성화
				// UE_LOG(LogTemp, Warning, TEXT("AGS_Player: Head audio listener set as default listener for local player."));
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AGS_Player::SetupHeadAudioListener: Skeletal mesh component is null!"));
	}
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

void AGS_Player::Multicast_PlaySkillMontage_Implementation(UAnimMontage* Montage, FName Section)
{
	UGS_SeekerAnimInstance* AnimInstance = Cast<UGS_SeekerAnimInstance>(GetMesh()->GetAnimInstance());
	if (AnimInstance && Montage)
	{
		if (Section == NAME_None)
		{
			AnimInstance->Montage_Play(Montage);
		}
		else
		{
			if (AnimInstance->Montage_IsPlaying(Montage))
			{
				AnimInstance->Montage_Stop(0.0f, Montage); // 이걸 꼭 해줘야 새로 PlayRate가 반영됨
			}
			
			AnimInstance->Montage_Play(Montage);
			AnimInstance->Montage_JumpToSection(Section, Montage);
		}
	}
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

void AGS_Player::UpdateSteamNameWidgetRotation()
{
	if (!IsValid(SteamNameWidgetComp))
	{
		return;
	}
    
	if (APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0))
	{
		FVector CameraForward = CameraManager->GetCameraRotation().Vector();
		FVector CameraRight = FVector::CrossProduct(CameraForward, FVector::UpVector).GetSafeNormal();
		FVector CameraUp = FVector::CrossProduct(CameraRight, CameraForward).GetSafeNormal();
		FRotator WidgetRotation = UKismetMathLibrary::MakeRotFromXZ(-CameraForward, CameraUp);
        
		SteamNameWidgetComp->SetWorldRotation(WidgetRotation);
	}
}

/*
void AGS_Player::Server_RestKey_Implementation()
{
	SetSkillInputControl(true, true, true, true);
	UGS_SeekerAnimInstance * SeekerAnim = Cast<UGS_SeekerAnimInstance>(GetMesh()->GetAnimInstance());
	if (SeekerAnim)
	{
		SeekerAnim->IsPlayingUpperBodyMontage = false;
		SeekerAnim->IsPlayingFullBodyMontage = false;
	};
}
*/
