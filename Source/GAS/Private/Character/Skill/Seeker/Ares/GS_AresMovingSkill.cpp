// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Ares/GS_AresMovingSkill.h"
#include "Character/GS_Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Character/Player/Seeker/GS_Ares.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Sound/GS_SeekerAudioComponent.h"
#include "Character/GS_TpsController.h"
#include "Character/Player/GS_Player.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "AkGameplayStatics.h"
#include "Character/Skill/GS_SkillSet.h"


UGS_AresMovingSkill::UGS_AresMovingSkill()
{
	CurrentSkillType = ESkillSlot::Moving;
}

void UGS_AresMovingSkill::ActiveSkill()
{
	Super::ActiveSkill();

	if (AGS_Ares* OwnerPlayer = Cast<AGS_Ares>(OwnerCharacter))
	{
		// 스킬 애니메이션 재생
		OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[0]);

		// 스킬 시작 사운드 재생 (멀티캐스트)
		if (OwnerPlayer->HasAuthority())
		{
			if (UGS_SeekerAudioComponent* AudioComp = OwnerPlayer->SeekerAudioComponent)
			{
				AudioComp->RequestSkillAudio(CurrentSkillType, 0);
			}
		}

		// 차징 루프 사운드는 SeekerAudioComponent를 통해 처리됨

		OwnerPlayer->SetMoveControlValue(false, false);
	}

	// 기본 충돌 설정 저장
	OriginalCapsuleResponseToPawn = OwnerCharacter->GetCapsuleComponent()->GetCollisionResponseToChannel(ECC_Pawn);
	OriginalMeshResponseToPawn = OwnerCharacter->GetMesh()->GetCollisionResponseToChannel(ECC_Pawn);

	// 차징 시작
	ChargingStartTime = OwnerCharacter->GetWorld()->GetTimeSeconds();
	ChargingTime = 0.0f;

	// 일정 주기로 방향과 차징 시간 갱신
	OwnerCharacter->GetWorld()->GetTimerManager().SetTimer(ChargingTimerHandle, this, &UGS_AresMovingSkill::UpdateCharging, 0.05f, true);
}

void UGS_AresMovingSkill::InitializeDelegate()
{
	Super::InitializeDelegate();

	if (OwningComp)
	{
		OwningComp->OnSkillActivated.AddDynamic(this, &UGS_AresMovingSkill::HandleSkillActivated);
	}
}

void UGS_AresMovingSkill::HandleSkillActivated(ESkillSlot ActivatedSkillSlot)
{
	// 이 스킬이 맞는지, 카메라가 Idle 상태인지 확인
	if (ActivatedSkillSlot == CurrentSkillType && CurrentZoomState == EZoomState::Idle)
	{
		// 카메라 줌아웃 시작 (클라이언트에서만)
		if (OwnerCharacter && OwnerCharacter->IsLocallyControlled())
		{
			StartCameraZoomOut();
		}
	}
}

void UGS_AresMovingSkill::OnSkillCanceledByDebuff()
{
}

void UGS_AresMovingSkill::OnSkillAnimationEnd()
{
	Super::OnSkillAnimationEnd();
}

void UGS_AresMovingSkill::OnSkillCommand()
{
	if (!CanActive() || !GetIsActive())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[AresDashCamera] OnSkillCommand CALLED (Server)"));

	if (AGS_Ares* OwnerPlayer = Cast<AGS_Ares>(OwnerCharacter))
	{
		OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[1]);
	}

	Super::OnSkillCommand();

	// Actor의 Multicast RPC를 통해 카메라 원복 (모든 클라이언트에게 전달됨)
	if (AGS_Ares* AresOwner = Cast<AGS_Ares>(OwnerCharacter))
	{
		AresOwner->Multicast_RestoreDashCameraZoom();
	}

	// 사운드 처리 (멀티캐스트)
	if (OwnerCharacter->HasAuthority())
	{
		if (AGS_Seeker* OwnerSeeker = Cast<AGS_Seeker>(OwnerCharacter))
		{
			if (UGS_SeekerAudioComponent* AudioComp = OwnerSeeker->SeekerAudioComponent)
			{
				// 차징 루프 사운드 정지
				AudioComp->RequestSkillAudio(CurrentSkillType, 3); // 3 = 루프 정지

				// 돌진 시작 사운드 재생
				AudioComp->RequestSkillAudio(CurrentSkillType, 0); // 0 = 스킬 시작
			}
		}
	}

	// 차징 종료
	OwnerCharacter->GetWorld()->GetTimerManager().ClearTimer(ChargingTimerHandle);

	// 돌진 거리 계산
	float Ratio = ChargingTime / MaxChargingTime;
	float DashDistance = FMath::Lerp(MinDashDistance, MaxDashDistance, Ratio);

	// 대시 시작
	if (IsValid(OwnerCharacter))
	{
		DashDirection = OwnerCharacter->GetActorForwardVector().GetSafeNormal();
		DashStartLocation = OwnerCharacter->GetActorLocation();
		DashEndLocation = DashStartLocation + DashDirection * DashDistance;
		DashInterpAlpha = 0.0f;
		StartDash();
	}

	// 쿨다운 시작
	StartCoolDown();
}

void UGS_AresMovingSkill::InterruptSkill()
{
	Super::InterruptSkill();

	// 카메라 원복 (Idle 상태가 아닐 때만)
	if (OwnerCharacter && OwnerCharacter->IsLocallyControlled() && CurrentZoomState != EZoomState::Idle)
	{
		RestoreCameraZoom(true); // 강제 복원
	}

	// 타이머 정리
	SafeClearTimer(ChargingTimerHandle);

	SetIsActive(false);
}

void UGS_AresMovingSkill::ApplyEffectToDungeonMonster(AGS_Monster* Target)
{
	// 유효성 체크
	if (!Target || !OwnerCharacter)
	{
		return;
	}

	// 데미지 적용
	UGameplayStatics::ApplyDamage(Target, 50.0f, OwnerCharacter->GetController(), OwnerCharacter, nullptr);
	
	// 타격 사운드 재생 (멀티캐스트) - HasAuthority 체크 불필요 (UpdateDash가 서버에서만 호출됨)
	FVector HitLocation = Target->GetActorLocation();
	Multicast_PlayDashHitSound(EAresDashHitTargetType::Monster, HitLocation);
}

void UGS_AresMovingSkill::ApplyEffectToGuardian(AGS_Guardian* Target)
{
	// 유효성 체크
	if (!Target || !OwnerCharacter)
	{
		return;
	}

	// 데미지 적용
	UGameplayStatics::ApplyDamage(Target, 50.0f, OwnerCharacter->GetController(), OwnerCharacter, nullptr);
	
	// 타격 사운드 재생 (멀티캐스트) - HasAuthority 체크 불필요 (UpdateDash가 서버에서만 호출됨)
	FVector HitLocation = Target->GetActorLocation();
	Multicast_PlayDashHitSound(EAresDashHitTargetType::Guardian, HitLocation);
}

void UGS_AresMovingSkill::UpdateCharging()
{
	if (!IsValid(OwnerCharacter))
	{
		return;
	}

	// 차징 시간 계산
	float CurrentTime = OwnerCharacter->GetWorld()->GetTimeSeconds();
	ChargingTime = FMath::Min(CurrentTime - ChargingStartTime, MaxChargingTime);
}

void UGS_AresMovingSkill::StartDash()
{
	// 충돌 몬스터 초기화
	DamagedActors.Empty();

	// 몬스터는 충돌 막지 않도록 설정
	OwnerCharacter->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	OwnerCharacter->GetMesh()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	OwnerCharacter->GetWorld()->GetTimerManager().SetTimer(DashTimerHandle, this, &UGS_AresMovingSkill::UpdateDash, 0.01f, true);
	
	// =======================
	// VFX 재생 - 컴포넌트 RPC 사용
	// =======================
	if (OwningComp)
	{
		FVector SkillLocation = OwnerCharacter->GetActorLocation();
		FRotator SkillRotation = FRotator(0.f, 0.f, 0.f);


		// 스킬 시전 VFX 재생
		OwningComp->Multicast_PlayCastVFX(CurrentSkillType, SkillLocation, SkillRotation);
	}
}

void UGS_AresMovingSkill::UpdateDash()
{
	// 유효성 체크
	if (!IsValid(OwnerCharacter) || !OwnerCharacter->GetWorld())
	{
		return;
	}

	// 서버에서만 실행 (위치 이동, 데미지, 충돌 판정)
	if (!OwnerCharacter->HasAuthority())
	{
		return;
	}

	float Step = 0.01f / DashDuration;
	DashInterpAlpha += Step;

	// 위치 보간 이동 (서버에서 실행, 자동 복제)
	FVector NewLocation = FMath::Lerp(DashStartLocation, DashEndLocation, DashInterpAlpha);
	OwnerCharacter->SetActorLocation(NewLocation, true); // Sweep = true로 충돌 적용
	DashStartLocation = NewLocation;

	// 공격 판정
	TArray<FHitResult> HitResults;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);
	
	if (OwnerCharacter->GetWorld()->SweepMultiByChannel(
		HitResults, 
		DashStartLocation, 
		DashEndLocation,
		FQuat::Identity, 
		ECC_Pawn, 
		FCollisionShape::MakeCapsule(100.0f, 100.0f), 
		Params))
	{
		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			
			// 중복 체크 (GetActor()는 이미 유효성 보장)
			if (!HitActor || DamagedActors.Contains(HitActor))
			{
				continue;
			}

			DamagedActors.Add(HitActor);

			// 타입별 처리 (최적화: 한 번만 Cast)
			if (AGS_Monster* TargetMonster = Cast<AGS_Monster>(HitActor))
			{
				ApplyEffectToDungeonMonster(TargetMonster);
			}
			else if (AGS_Guardian* TargetGuardian = Cast<AGS_Guardian>(HitActor))
			{
				ApplyEffectToGuardian(TargetGuardian);
			}
		}
	}

	// 대시 거리만큼 이동 완료
	if (DashInterpAlpha >= 1.f)
	{
		// 스킬 종료
		DeactiveSkill();
	}
}

void UGS_AresMovingSkill::DeactiveSkill()
{
	// 대시 타이머만 정리합니다.
	if (OwnerCharacter && OwnerCharacter->GetWorld())
	{
		OwnerCharacter->GetWorld()->GetTimerManager().ClearTimer(DashTimerHandle);
	}

	// 입력 제한 설정
	/*AGS_TpsController* Controller = Cast<AGS_TpsController>(OwnerCharacter->GetController());
	Controller->SetMoveControlValue(true, true);*/
	AGS_Ares* AresCharacter = Cast<AGS_Ares>(OwnerCharacter);
	AresCharacter->SetMoveControlValue(true, true);
	//OwnerCharacter->SetSkillInputControl(true, true, true);

	// 원래대로 Block으로 되돌리기
	OwnerCharacter->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, OriginalCapsuleResponseToPawn);
	OwnerCharacter->GetMesh()->SetCollisionResponseToChannel(ECC_Pawn, OriginalMeshResponseToPawn);

	// 스킬 종료 사운드 재생 (멀티캐스트)
	if (OwnerCharacter->HasAuthority())
	{
		if (AGS_Seeker* OwnerSeeker = Cast<AGS_Seeker>(OwnerCharacter))
		{
			if (UGS_SeekerAudioComponent* AudioComp = OwnerSeeker->SeekerAudioComponent)
			{
				AudioComp->RequestSkillAudio(CurrentSkillType, 1);
			}
		}
	}

	Super::DeactiveSkill();
}

void UGS_AresMovingSkill::StartCameraZoomOut()
{
	if (!OwnerCharacter || !OwnerCharacter->IsLocallyControlled())
	{
		return;
	}

	// 이미 줌 진행 중이면 중복 호출 방지
	if (CurrentZoomState != EZoomState::Idle)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AresDashCamera] StartCameraZoomOut IGNORED (Already in progress, State=%d)"), (int32)CurrentZoomState);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[AresDashCamera] StartCameraZoomOut CALLED"));

	AGS_Player* OwnerPlayer = Cast<AGS_Player>(OwnerCharacter);
	if (!OwnerPlayer || !OwnerPlayer->SpringArmComp)
	{
		return;
	}

	CacheCameraMotionBlurDefaults(OwnerPlayer);
	ResetCameraMotionBlur();

	// 원래 거리 저장
	OriginalArmLength = OwnerPlayer->SpringArmComp->TargetArmLength;

	// 커브 시간 계산
	CameraZoomDuration = GetCameraZoomDuration();

	CameraZoomElapsed = 0.0f;
	CurrentZoomState = EZoomState::ZoomingOut;
	bPendingZoomIn = false;
	bMotionBlurActive = false;

	// 기존 타이머 정리 후 새 타이머 시작
	SafeClearTimer(CameraUpdateTimerHandle);
	OwnerCharacter->GetWorld()->GetTimerManager().SetTimer(
		CameraUpdateTimerHandle,
		this,
		&UGS_AresMovingSkill::UpdateCameraZoom,
		0.016f, // ~60fps
		true
	);
}

void UGS_AresMovingSkill::RestoreCameraZoom(bool bForceRestore)
{
	if (!OwnerCharacter || !OwnerCharacter->IsLocallyControlled())
	{
		return;
	}

	// Idle 상태면 이미 원래 상태이므로 무시
	if (CurrentZoomState == EZoomState::Idle)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AresDashCamera] RestoreCameraZoom IGNORED (Already Idle)"));
		return;
	}

	// 강제 복원 모드가 아니고, 줌아웃이 완료되지 않았다면 pending 처리
	if (!bForceRestore && CurrentZoomState != EZoomState::ZoomedOut)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AresDashCamera] RestoreCameraZoom PENDING (State=%d)"), (int32)CurrentZoomState);
		bPendingZoomIn = true;
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[AresDashCamera] RestoreCameraZoom START (Force=%s, State=%d)"),
		bForceRestore ? TEXT("true") : TEXT("false"), (int32)CurrentZoomState);

	AGS_Player* OwnerPlayer = Cast<AGS_Player>(OwnerCharacter);
	if (!OwnerPlayer || !OwnerPlayer->SpringArmComp)
	{
		return;
	}

	CacheCameraMotionBlurDefaults(OwnerPlayer);

	// 커브 시간 계산
	CameraZoomDuration = GetCameraZoomDuration();

	CameraZoomElapsed = 0.0f;
	CurrentZoomState = EZoomState::ZoomingIn;
	bPendingZoomIn = false;
	bMotionBlurActive = bEnableMotionBlur && (MotionBlurPeakAmount > KINDA_SMALL_NUMBER);

	if (bMotionBlurActive)
	{
		UpdateCameraMotionBlur(0.0f, 0.0f);
	}
	else
	{
		ResetCameraMotionBlur();
	}

	// 기존 타이머 정리 후 줌인 타이머 시작
	SafeClearTimer(CameraUpdateTimerHandle);
	OwnerCharacter->GetWorld()->GetTimerManager().SetTimer(
		CameraUpdateTimerHandle,
		this,
		&UGS_AresMovingSkill::UpdateCameraZoom,
		0.016f, // ~60fps
		true
	);
}

void UGS_AresMovingSkill::SetCameraSettings(float InZoomOutDistance, UCurveFloat* InCameraZoomCurve,
	bool bInEnableMotionBlur,
	float InMotionBlurPeakAmount,
	UCurveFloat* InMotionBlurCurve,
	float InMotionBlurExponent)
{
	ZoomOutDistance = InZoomOutDistance;
	CameraZoomCurve = InCameraZoomCurve;
	bEnableMotionBlur = bInEnableMotionBlur;
	MotionBlurPeakAmount = FMath::Clamp(InMotionBlurPeakAmount, 0.0f, 1.0f);
	MotionBlurCurve = InMotionBlurCurve;
	MotionBlurExponent = FMath::Max(0.01f, InMotionBlurExponent);
}

void UGS_AresMovingSkill::UpdateCameraZoom()
{
	if (!OwnerCharacter || !OwnerCharacter->IsLocallyControlled())
	{
		return;
	}

	AGS_Player* OwnerPlayer = Cast<AGS_Player>(OwnerCharacter);
	if (!OwnerPlayer || !OwnerPlayer->SpringArmComp)
	{
		return;
	}

	// 시간 경과 (타이머 간격 0.016초 사용)
	float DeltaTime = 0.016f;
	CameraZoomElapsed += DeltaTime;

	float Alpha = FMath::Clamp(CameraZoomElapsed / CameraZoomDuration, 0.0f, 1.0f);

	// 커브가 있으면 커브 값 사용, 없으면 선형 보간
	if (CameraZoomCurve)
	{
		Alpha = CameraZoomCurve->GetFloatValue(CameraZoomElapsed);
	}

	float TargetArmLength = OriginalArmLength;
	if (CurrentZoomState == EZoomState::ZoomingOut)
	{
		TargetArmLength = FMath::Lerp(OriginalArmLength, OriginalArmLength + ZoomOutDistance, Alpha);
	}
	else if (CurrentZoomState == EZoomState::ZoomingIn)
	{
		TargetArmLength = FMath::Lerp(OriginalArmLength + ZoomOutDistance, OriginalArmLength, Alpha);
		UpdateCameraMotionBlur(Alpha, CameraZoomElapsed);
	}

	OwnerPlayer->SpringArmComp->TargetArmLength = TargetArmLength;

	// 애니메이션 완료 체크
	if (CameraZoomElapsed >= CameraZoomDuration)
	{
		if (CurrentZoomState == EZoomState::ZoomingOut)
		{
			CurrentZoomState = EZoomState::ZoomedOut;
			SafeClearTimer(CameraUpdateTimerHandle);
			UE_LOG(LogTemp, Warning, TEXT("[AresDashCamera] Zoom-Out FINISHED. State -> ZoomedOut."));

			if (bPendingZoomIn)
			{
				UE_LOG(LogTemp, Warning, TEXT("[AresDashCamera] Pending Zoom-In detected. Starting zoom-in."));
				bPendingZoomIn = false;
				RestoreCameraZoom();
			}
		}
		else if (CurrentZoomState == EZoomState::ZoomingIn)
		{
			CurrentZoomState = EZoomState::Idle;
			SafeClearTimer(CameraUpdateTimerHandle);
			ResetCameraMotionBlur();
			UE_LOG(LogTemp, Warning, TEXT("[AresDashCamera] Zoom-In FINISHED. State -> Idle. Clearing Timer."));
		}
	}
}

void UGS_AresMovingSkill::SafeClearTimer(FTimerHandle& TimerHandle)
{
	if (!TimerHandle.IsValid())
	{
		return;
	}

	if (!IsWorldContextValid())
	{
		TimerHandle.Invalidate();
		return;
	}

	UWorld* World = OwnerCharacter ? OwnerCharacter->GetWorld() : nullptr;
	if (World && World->IsValidLowLevel() && !World->bIsTearingDown)
	{
		World->GetTimerManager().ClearTimer(TimerHandle);
	}
	TimerHandle.Invalidate();
}

bool UGS_AresMovingSkill::IsWorldContextValid() const
{
	if (!OwnerCharacter)
	{
		return false;
	}

	UWorld* World = OwnerCharacter->GetWorld();
	return World &&
	       World->IsValidLowLevel() &&
	       !World->bIsTearingDown &&
	       IsValid(World);
}

void UGS_AresMovingSkill::BeginDestroy()
{
	ResetCameraMotionBlur();

	// 모든 타이머 정리
	SafeClearTimer(ChargingTimerHandle);
	SafeClearTimer(DashTimerHandle);
	SafeClearTimer(CameraUpdateTimerHandle);

	Super::BeginDestroy();
}

void UGS_AresMovingSkill::ResetCameraMotionBlur()
{
	if (!bMotionBlurDefaultsCached)
	{
		return;
	}

	AGS_Player* OwnerPlayer = Cast<AGS_Player>(OwnerCharacter);
	if (!OwnerPlayer || !OwnerPlayer->CameraComp)
	{
		return;
	}

	OwnerPlayer->CameraComp->PostProcessSettings.MotionBlurAmount = OriginalMotionBlurAmount;
	OwnerPlayer->CameraComp->PostProcessSettings.bOverride_MotionBlurAmount = bOriginalOverrideMotionBlurAmount;
	bMotionBlurActive = false;
}

void UGS_AresMovingSkill::CacheCameraMotionBlurDefaults(AGS_Player* Player)
{
	if (bMotionBlurDefaultsCached || !Player || !Player->CameraComp)
	{
		return;
	}

	OriginalMotionBlurAmount = Player->CameraComp->PostProcessSettings.MotionBlurAmount;
	bOriginalOverrideMotionBlurAmount = Player->CameraComp->PostProcessSettings.bOverride_MotionBlurAmount;
	bMotionBlurDefaultsCached = true;
}

void UGS_AresMovingSkill::UpdateCameraMotionBlur(float NormalizedAlpha, float ElapsedTime)
{
	if (!bMotionBlurActive || !bEnableMotionBlur)
	{
		return;
	}

	AGS_Player* OwnerPlayer = Cast<AGS_Player>(OwnerCharacter);
	if (!OwnerPlayer || !OwnerPlayer->CameraComp)
	{
		return;
	}

	float Weight = 1.0f - NormalizedAlpha;

	if (MotionBlurCurve)
	{
		Weight = MotionBlurCurve->GetFloatValue(ElapsedTime);
	}
	else
	{
		Weight = FMath::Pow(FMath::Clamp(Weight, 0.0f, 1.0f), MotionBlurExponent);
	}

	float BlurAmount = MotionBlurPeakAmount * FMath::Clamp(Weight, 0.0f, 1.0f);

	OwnerPlayer->CameraComp->PostProcessSettings.bOverride_MotionBlurAmount = true;
	OwnerPlayer->CameraComp->PostProcessSettings.MotionBlurAmount = BlurAmount;

	if (BlurAmount <= KINDA_SMALL_NUMBER && CurrentZoomState != EZoomState::ZoomingIn)
	{
		ResetCameraMotionBlur();
	}
}

float UGS_AresMovingSkill::GetCameraZoomDuration() const
{
	if (CameraZoomCurve)
	{
		float MinTime = 0.f;
		float MaxTime = 0.f;
		CameraZoomCurve->GetTimeRange(MinTime, MaxTime);
		float Duration = MaxTime - MinTime;
		return (Duration > 0.0f) ? Duration : 0.3f;
	}
	return 0.3f;
}

void UGS_AresMovingSkill::Multicast_PlayDashHitSound_Implementation(EAresDashHitTargetType TargetType, const FVector& HitLocation)
{
	// 유효성 체크
	if (!OwnerCharacter || !OwnerCharacter->GetWorld())
	{
		return;
	}

	// 데이터 테이블에서 스킬 정보 가져오기
	const FSkillInfo* SkillInfo = GetCurrentSkillInfo();
	if (!SkillInfo)
	{
		return;
	}

	UAkAudioEvent* SoundEventToPlay = nullptr;

	// 타격 대상 타입에 따라 데이터 테이블의 사운드 선택
	switch (TargetType)
	{
	case EAresDashHitTargetType::Guardian:
		SoundEventToPlay = SkillInfo->GuardianCollisionSound;
		break;
	case EAresDashHitTargetType::Monster:
		SoundEventToPlay = SkillInfo->MonsterCollisionSound;
		break;
	case EAresDashHitTargetType::Other:
	default:
		// 기타 오브젝트는 사운드 없음
		break;
	}

	// Wwise 사운드 이벤트 재생
	if (SoundEventToPlay)
	{
		UAkGameplayStatics::PostEventAtLocation(
			SoundEventToPlay,
			HitLocation,
			FRotator::ZeroRotator,
			OwnerCharacter->GetWorld()
		);
	}
}