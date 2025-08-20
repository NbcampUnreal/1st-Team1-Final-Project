// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Seeker/GS_Merci.h"
#include "Sound/GS_CharacterAudioComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon/Projectile/Seeker/GS_SeekerMerciArrow.h"
#include "Character/Component/Seeker/GS_MerciSkillInputHandlerComp.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "AkGameplayTypes.h"
#include "AkComponent.h"
#include "AkAudioEvent.h"
#include "Kismet/GameplayStatics.h"
#include "Templates/Function.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"
#include "UI/Character/GS_ArrowTypeWidget.h"
#include "UI/Character/GS_CrossHairImage.h"
#include "AkGameplayStatics.h"
#include "Character/GS_Character.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/Skill/GS_SkillComp.h"
//#include "Weapon/Equipable/"

// Sets default values
AGS_Merci::AGS_Merci()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Weapon = CreateDefaultSubobject<UChildActorComponent>(TEXT("MerciBow"));
	Weapon->SetupAttachment(GetMesh(), TEXT("BowSocket"));
	
	Quiver = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("QuiverMesh"));
	Quiver->SetupAttachment(GetMesh(), TEXT("QuiverSocket"));
	Quiver->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Quiver->SetSimulatePhysics(false);
	
	CharacterType = ECharacterType::Merci;
	SkillInputHandlerComponent = CreateDefaultSubobject<UGS_MerciSkillInputHandlerComp>(TEXT("SkillInputHandlerComp"));
}

void AGS_Merci::Client_UpdateTargetUI_Implementation(AActor* NewTarget, AActor* OldTarget)
{
	// 이전 타겟 UI 숨기기
	if (OldTarget)
	{
		if (AGS_Monster* Monster = Cast<AGS_Monster>(OldTarget))
		{
			Monster->ShowTargetUI(false); // 일반 함수 호출
		}
		else if (AGS_Guardian* Guardian = Cast<AGS_Guardian>(OldTarget))
		{
			Guardian->ShowTargetUI(false);
		}
	}

	// 새 타겟 UI 표시
	if (NewTarget)
	{
		if (AGS_Monster* Monster = Cast<AGS_Monster>(NewTarget))
		{
			Monster->ShowTargetUI(true);
		}
		else if (AGS_Guardian* Guardian = Cast<AGS_Guardian>(NewTarget))
		{
			Guardian->ShowTargetUI(true);
		}
	}
}

// Called when the game starts or when spawned
void AGS_Merci::BeginPlay()
{
	Super::BeginPlay();

	CurrentArrowType = EArrowType::Normal;

	if (HasAuthority())
	{
		CurrentAxeArrows = MaxAxeArrows;
		CurrentChildArrows = MaxChildArrows;

		// 일정 주기로 화살 재충전 타이머 시작
		GetWorld()->GetTimerManager().SetTimer(AxeArrowRegenTimer, this, &AGS_Merci::RegenAxeArrow, RegenInterval, true);
		GetWorld()->GetTimerManager().SetTimer(ChildArrowRegenTimer, this, &AGS_Merci::RegenChildArrow, RegenInterval, true);
	}

	Mesh = this->GetMesh();
	UE_LOG(LogTemp, Warning, TEXT("AnimInstance: %s"), *GetNameSafe(GetMesh()->GetAnimInstance()));
	if (ZoomCurve)
	{
		FOnTimelineFloat TimelineCallback;
		TimelineCallback.BindUFunction(this, FName("UpdateZoom"));

		ZoomTimeline.AddInterpFloat(ZoomCurve, TimelineCallback);
	}
}

void AGS_Merci::DrawBow(UAnimMontage* DrawMontage)
{
	if (!HasAuthority())
	{
		// 서버에 요청
		Server_DrawBow(DrawMontage);
		return;
	}

	// DrawBow 가 Client 외에 Server 에서 호출될 일이 있나? Client 에서 해당 함수가 호출되었다면 이미 쥐에서 Return 으로 막히는 거 아닌가?
	if (!GetDrawState())
	{
		if (WidgetCrosshair)
		{
			WidgetCrosshair->PlayAimAnim(true);
		}

		// 줌 시작
		if(!GetSkillComp()->IsSkillActive(ESkillSlot::Ultimate))
		{
			Client_StartZoom();
		}
	}

	if (!GetDrawState())
	{
		UE_LOG(LogTemp, Warning, TEXT("GetDrawState=false pass: %s"), GetDrawState()?TEXT("true") : TEXT("false"));
		UE_LOG(LogTemp, Warning, TEXT("Multicast_PlayDrawMontage()"))
		Multicast_PlayDrawMontage(DrawMontage);

		// 활 상태 업데이트
		SetDrawState(true);
		SetAimState(false);
		Multicast_SetMustTurnInPlace(true);
		
		// 활 당기는 사운드 재생 (멀티캐스트로 변경)
		Multicast_PlayBowPullSound();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("bIsDrawState true"));
	}

	// 걷기 상태 설정
	SetSeekerGait(EGait::Walk);
}

void AGS_Merci::ReleaseArrow(TSubclassOf<AGS_SeekerMerciArrow> ArrowClass, float SpreadAngleDeg, int32 NumArrows)
{
	if (!HasAuthority())
	{
		Server_ReleaseArrow(ArrowClass, SpreadAngleDeg, NumArrows);
		return;
	}

	if (WidgetCrosshair)
	{
		WidgetCrosshair->PlayAimAnim(false);
	}

	// 줌 중지
	if (!(this->GetSkillComp()->IsSkillActive(ESkillSlot::Ultimate)))
	{
		Client_StopZoom();
	}
	
	// 몽타주 정지
	Multicast_StopDrawMontage();
	
	// 조준 완료 시(활을 끝까지 당겼을 때)
	if (GetAimState())
	{
		// 활 놓는 사운드 재생 (멀티캐스트로 변경)
		Multicast_PlayBowReleaseSound();

		// 화살 발사
		Server_FireArrow(ArrowClass, SpreadAngleDeg, NumArrows);
		bIsFullyDrawn = false;  // 상태 초기화

		GetSkillComp()->ResetAllowedSkillsMask(); // SJE
	}

	// 달리기 상태 설정
	SetSeekerGait(EGait::Run);

	// 활 상태 업데이트
	SetAimState(false);
	SetDrawState(false);
}

void AGS_Merci::Server_DrawBow_Implementation(UAnimMontage* DrawMontage)
{
	DrawBow(DrawMontage);
}

void AGS_Merci::Server_ReleaseArrow_Implementation(TSubclassOf<AGS_SeekerMerciArrow> ArrowClass, float SpreadAngleDeg, int32 NumArrows)
{
	ReleaseArrow(ArrowClass, SpreadAngleDeg, NumArrows);
}

void AGS_Merci::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGS_Merci, CurrentAxeArrows);
	DOREPLIFETIME(AGS_Merci, CurrentChildArrows);
	DOREPLIFETIME(AGS_Merci, CurrentArrowType);
	DOREPLIFETIME(AGS_Merci, AutoAimTarget);
}

void AGS_Merci::PlayDrawMontage(UAnimMontage* DrawMontage)
{
	if (Mesh && DrawMontage)
	{
		float Duration = Mesh->GetAnimInstance()->Montage_Play(DrawMontage);
		if (Duration > 0.0f)
		{
			UE_LOG(LogTemp, Warning, TEXT("Montage_Play called, duration: %f"), Duration);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Duration<=0.0f"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Mesh null or DrawMontage null"));
	}
}

void AGS_Merci::Multicast_StopDrawMontage_Implementation()
{
	if (Mesh && Mesh->GetAnimInstance())
	{
		Mesh->GetAnimInstance()->Montage_Stop(0.2f); // BlendOut 0.2초
	}
}

void AGS_Merci::Multicast_PlayDrawMontage_Implementation(UAnimMontage* Montage)
{
	UE_LOG(LogTemp, Warning, TEXT("Multicast_PlayDrawMontage called on %s"), *GetName());
	UE_LOG(LogTemp, Warning, TEXT("Multicast_PlayDrawMontage called on %s, Montage: %s"),
		*GetName(),
		*GetNameSafe(Montage));
	PlayDrawMontage(Montage);
}

void AGS_Merci::Server_FireArrow_Implementation(TSubclassOf<AGS_SeekerMerciArrow> ArrowClass, float SpreadAngleDeg, int32 NumArrows)
{
	if (!ArrowClass || !Weapon)
	{
		return;
	}

	// 현재 화살 수량 체크
	if (CurrentArrowType == EArrowType::Axe && CurrentAxeArrows <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Axe Empty"));
		Multicast_PlayArrowEmptySound(); // 빈 화살 사운드 재생
		return;
	}
	if (CurrentArrowType == EArrowType::Child && CurrentChildArrows <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Child Empty"));
		Multicast_PlayArrowEmptySound(); // 빈 화살 사운드 재생
		return;
	}

	// 수량 감소
	if (NumArrows == 1)
	{
		if (CurrentArrowType == EArrowType::Axe)
		{
			--CurrentAxeArrows;
			UE_LOG(LogTemp, Log, TEXT("Axe Shot: %d"), CurrentAxeArrows);
		}
		else if (CurrentArrowType == EArrowType::Child)
		{
			--CurrentChildArrows;
			UE_LOG(LogTemp, Log, TEXT("Child Shot: %d"), CurrentChildArrows);
		}
	}

	// 1. 카메라 위치와 회전값(플레이어의 시점) 가져오기
	FVector ViewLoc;
	FRotator ViewRot;
	Controller->GetPlayerViewPoint(ViewLoc, ViewRot); // 카메라 기준 시점

	// 2. 카메라에서 정면으로 Ray를 쏨
	FVector TraceStart = ViewLoc;
	FVector TraceEnd = TraceStart + ViewRot.Vector() * 2000.0f;
	//Multicast_DrawDebugLine(TraceStart, TraceEnd, FColor::Green);
	
	// 3. Ray가 무언가에 부딪히면 그 위치를 목표로 설정, 아니면 끝 지점 사용
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FVector TargetLocation = TraceEnd;

	if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
	{
		TargetLocation = Hit.ImpactPoint;

		AActor* HitActor = Hit.GetActor();
		UPrimitiveComponent* HitComponent = Hit.GetComponent();

		UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), HitActor ? *HitActor->GetName() : TEXT("None"));
		UE_LOG(LogTemp, Warning, TEXT("Hit Component: %s"), HitComponent ? *HitComponent->GetName() : TEXT("None"));
	}
	

	// 4. 무기의 소켓 위치에서 목표 위치로 향하는 방향 계산
	FVector SpawnLocation = Weapon->GetSocketLocation("BowstringSocket");
	float Distance = FVector::Dist(TargetLocation, SpawnLocation);
	UE_LOG(LogTemp, Warning, TEXT("Distance From Spawn To Target = %f"), Distance);

	const float MinFireDistance = 250.0f;

	FVector LaunchDirection = (TargetLocation - SpawnLocation).GetSafeNormal();
	FRotator BaseRotation = LaunchDirection.Rotation();

	FVector VFXLocation = SpawnLocation;
	FRotator VFXRotation = BaseRotation;

	
	// 선형 보정 (예: 최대 2000 거리까지, 최대 20도 상승)
	//float MaxDistance = 5000.0f;
	//float MaxPitch = 40.0f;
	//// 제곱 보정 (거리가 멀수록 더 빠르게 증가)
	//float PitchAdjustment = FMath::Clamp(FMath::Square(Distance / MaxDistance) * MaxPitch, 0.0f, MaxPitch);
	////float PitchAdjustment = FMath::Clamp((Distance / MaxDistance) * MaxPitch, 0.0f, MaxPitch);
	//BaseRotation.Pitch += PitchAdjustment;

	// 5. 여러 발 발사 처리 (SpreadAngleDeg를 기준으로 좌우로 퍼지게 만듦)
	int32 HalfCount = NumArrows / 2;
	for (int32 i = 0; i < NumArrows; ++i)
	{
		// 중심 기준 각도 차이 계산
		float OffsetAngle = (i - HalfCount) * SpreadAngleDeg;

		// 짝수일 경우 중심에서 양옆으로 대칭 유지
		if (NumArrows % 2 == 0)
		{
			OffsetAngle += SpreadAngleDeg / 2.0f;
		}

		// Yaw(좌우)만 회전 적용하여 퍼지는 방향 구함
		FRotator SpreadRot = BaseRotation;
		SpreadRot.Yaw += OffsetAngle;
		FVector SpreadDir = SpreadRot.Vector();
		FRotator ArrowRot = SpreadDir.Rotation();

		// 6. 화살 스폰
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.Instigator = this;

		AGS_SeekerMerciArrow* SpawnedArrow =GetWorld()->SpawnActor<AGS_SeekerMerciArrow>(ArrowClass, SpawnLocation, ArrowRot, SpawnParams);
		
		if(SpawnedArrow)
		{
			SpawnedArrow->SetOwner(this);
		}
		
		// 7. 화살 타입 설정
		if (AGS_SeekerMerciArrowNormal* NormalArrow = Cast<AGS_SeekerMerciArrowNormal>(SpawnedArrow)) // 연기화살 제외
		{
			// 멀티샷일 경우
			if (NumArrows > 1)
			{
				NormalArrow->ChangeArrowType(EArrowType::Normal);
			}
			else // 한 발일 경우
			{
				NormalArrow->ChangeArrowType(CurrentArrowType);
			}
		}

		// 8. 유도 화살
		if (this->GetSkillComp()->IsSkillActive(ESkillSlot::Ultimate))
		{
			if (AGS_SeekerMerciArrow* HomingArrow = Cast<AGS_SeekerMerciArrow>(SpawnedArrow))
			{
				if (AutoAimTarget)
				{
					HomingArrow->Multicast_InitHomingTarget(AutoAimTarget);
				}
			}
		}
		else
		{
			if (AGS_SeekerMerciArrow* HomingArrow = Cast<AGS_SeekerMerciArrow>(SpawnedArrow))
			{
				HomingArrow->Multicast_InitHomingTarget(nullptr);
			}
		}
		//Multicast_DrawDebugLine(SpawnLocation, TargetLocation, FColor::Red);
	}
	// 8. 화살 발사 VFX 및 사운드 호출 (멀티캐스트로 모든 클라이언트에서 재생)
	Multicast_PlayArrowShotVFX(VFXLocation, VFXRotation, NumArrows);
	
	// 실제로 화살이 발사될 때만 사운드 재생
	Multicast_PlayArrowShotSound();

	
}

void AGS_Merci::Server_ChangeArrowType_Implementation(int32 Direction)
{
	int32 NumTypes = 3;

	int32 CurrentIndex = static_cast<int32>(CurrentArrowType);

	CurrentIndex = (CurrentIndex + Direction + NumTypes) % NumTypes;

	CurrentArrowType = static_cast<EArrowType>(CurrentIndex);

	UE_LOG(LogTemp, Log, TEXT("Arrow Changed to: %d"), CurrentIndex);
}

void AGS_Merci::SetAutoAimTarget(AActor* Target)
{ 
	if (HasAuthority())
	{
		// 타겟 설정
		if (Target)
		{
			AutoAimTarget = Target;
			OnRep_AutoAimTarget(); // 즉시 로컬 처리
		}
	}
}

void AGS_Merci::UpdateZoom(float Alpha)
{
	if (!SpringArmComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpringArmComp null"));
		return;
	}

	float TargetArmLength = FMath::Lerp(400.0f, 180.0f, Alpha);
	float SocketOffsetY = FMath::Lerp(67.f, 87.f, Alpha);
	float SocketOffsetZ = FMath::Lerp(174.f, 134.f, Alpha);

	SpringArmComp->TargetArmLength = TargetArmLength;
	FVector OffSet(0.f, SocketOffsetY, SocketOffsetZ);
	SpringArmComp->SocketOffset = OffSet;
}

void AGS_Merci::Multicast_DrawDebugLine_Implementation(FVector Start, FVector End, FColor Color)
{
	DrawDebugLine(GetWorld(), Start, End, Color, false, 5.0f, 0, 3.0f);
}

void AGS_Merci::OnDrawMontageEnded()
{
	bIsFullyDrawn = true;  // 활 완전히 당김 상태 설정

	// 서버로 전달
	if (HasAuthority() == false)
	{
		Server_NotifyDrawMontageEnded();
	}
}

void AGS_Merci::Server_NotifyDrawMontageEnded_Implementation()
{
	SetAimState(true);
	SetDrawState(false);
}

void AGS_Merci::Client_SetWidgetVisibility_Implementation(bool bVisible)
{
	if (!IsLocallyControlled()) return;

	if (WidgetCrosshair)
	{
		UE_LOG(LogTemp, Warning, TEXT("WidgetVisibility"));
		WidgetCrosshair->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void AGS_Merci::Client_StartZoom_Implementation()
{
	ZoomTimeline.Play(); // 줌인
}

void AGS_Merci::Client_StopZoom_Implementation()
{
	ZoomTimeline.Reverse(); // 줌아웃
}

void AGS_Merci::SetCrosshairWidget(UGS_CrossHairImage* InCrosshairWidget)
{
	WidgetCrosshair = InCrosshairWidget;

	if (WidgetCrosshair)
	{
		WidgetCrosshair->SetCrosshairVisibility(true);

		WidgetCrosshair->InitArrowCounters();
		WidgetCrosshair->UpdateArrowType(CurrentArrowType);

		if (CurrentArrowType == EArrowType::Axe)
		{
			WidgetCrosshair->UpdateArrowCnt(EArrowType::Axe, CurrentAxeArrows);
		}
		else if (CurrentArrowType == EArrowType::Child)
		{
			WidgetCrosshair->UpdateArrowCnt(EArrowType::Child, CurrentChildArrows);
		}
	}
}

void AGS_Merci::Client_ShowCrosshairHitFeedback_Implementation()
{
	if (!WidgetCrosshair)
	{
		return;
	}

	WidgetCrosshair->PlayHitFeedback();
}

void AGS_Merci::Client_PlaySound_Implementation(UAkComponent* SoundComp)
{
	if (SoundComp)
	{
		SoundComp->PostAssociatedAkEvent(0, FOnAkPostEventCallback());
	}
}

// Called every frame
void AGS_Merci::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ZoomTimeline.TickTimeline(DeltaTime);
}

// Called to bind functionality to input
void AGS_Merci::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AGS_Merci::LeftClickPressed_Implementation()
{
	IGS_AttackInterface::LeftClickPressed_Implementation();
}

void AGS_Merci::LeftClickRelease_Implementation()
{
	IGS_AttackInterface::LeftClickRelease_Implementation();
}

float AGS_Merci::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// 활을 들고 있는 경우
	if (GetDrawState() || GetAimState())
	{
		// 활 쏘기 애니메이션 재생 정지
		if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
		{
			AnimInst->StopAllMontages(0.2f);
		}

		// 활 쏘기 줌 아웃 (궁극기 상태가 아닐 때만)
		if (!this->GetSkillComp()->IsSkillActive(ESkillSlot::Ultimate))
		{
			Client_StopZoom();
		}

		// 활 쏘기 조준 상태 해제
		SetDrawState(false);
		SetAimState(false);

		// 달리기 상태 설정
		SetSeekerGait(EGait::Run);
	}

	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	return ActualDamage;
}

int32 AGS_Merci::GetMaxAxeArrows()
{
	return MaxAxeArrows;
}

int32 AGS_Merci::GetMaxChildArrows()
{
	return MaxChildArrows;
}

void AGS_Merci::OnRep_CurrentArrowType()
{
	if(ArrowTypeWidget)
	{
		ArrowTypeWidget->UpdateArrowImage(CurrentArrowType);
		if (CurrentArrowType == EArrowType::Normal)
		{
			ArrowTypeWidget->UpdateArrowCount(9999);
		}
		else if (CurrentArrowType == EArrowType::Axe)
		{
			ArrowTypeWidget->UpdateArrowCount(CurrentAxeArrows);
		}
		else if (CurrentArrowType == EArrowType::Child)
		{
			ArrowTypeWidget->UpdateArrowCount(CurrentChildArrows);
		}
	}

	if (WidgetCrosshair)
	{
		WidgetCrosshair->UpdateArrowType(CurrentArrowType);
	}

	// 화살 타입 변경 사운드 재생
	if (ArrowTypeChangeSound && IsLocallyControlled())
	{
		if (CharacterAudioComponent)
		{
			CharacterAudioComponent->PlaySound(ArrowTypeChangeSound, true);
		}
	}
}

void AGS_Merci::OnRep_CurrentAxeArrows()
{
	if (ArrowTypeWidget)
	{
		if(CurrentArrowType==EArrowType::Axe)
		{
			ArrowTypeWidget->UpdateArrowCount(CurrentAxeArrows);
		}
	}

	if (WidgetCrosshair)
	{
		WidgetCrosshair->UpdateArrowCnt(EArrowType::Axe, CurrentAxeArrows);
	}
}

void AGS_Merci::OnRep_CurrentChildArrows()
{
	if (ArrowTypeWidget)
	{
		if (CurrentArrowType == EArrowType::Child)
		{
			ArrowTypeWidget->UpdateArrowCount(CurrentChildArrows);
		}
	}

	if (WidgetCrosshair)
	{
		WidgetCrosshair->UpdateArrowCnt(EArrowType::Child, CurrentChildArrows);
	}
}

void AGS_Merci::RegenAxeArrow()
{
	if (!HasAuthority()) 
	{
		return;
	}

	if (CurrentAxeArrows < MaxAxeArrows)
	{
		++CurrentAxeArrows;
		UE_LOG(LogTemp, Log, TEXT("Axe regen: %d"), CurrentAxeArrows);
	}
}

void AGS_Merci::RegenChildArrow()
{
	if (!HasAuthority()) 
	{
		return;
	}

	if (CurrentChildArrows < MaxChildArrows)
	{
		++CurrentChildArrows;
		UE_LOG(LogTemp, Log, TEXT("Child regen: %d"), CurrentChildArrows);
	}
}

void AGS_Merci::OnRep_AutoAimTarget()
{
	if (AutoAimTarget)
	{
		// 예: 조준 HUD 표시, 자동 공격 유도 등
	}
}

void AGS_Merci::Client_DrawDebugSphere_Implementation(FVector Loc, float Radius, FColor Color, float Duration)
{
	DrawDebugSphere(
		GetWorld(),
		Loc,
		Radius,              // 반지름
		16,                 // 세그먼트
		Color,        // 색상
		false,              // 지속 여부
		Duration,               // 지속 시간 (2초)
		0,
		2.0f                // 선 두께
	);
}

void AGS_Merci::Multicast_PlayArrowShotVFX_Implementation(FVector Location, FRotator Rotation, int32 NumArrows)
{
	if (!GetWorld()) return;
	
	// 화살 수에 따라 다른 VFX 선택
	UNiagaraSystem* VFXToPlay = (NumArrows > 1) ? MultiShotVFX : ArrowShotVFX;
	
	if (VFXToPlay)
	{
		FVector PositionOffset = (NumArrows > 1) ? MultiShotVFXOffset : ArrowShotVFXOffset;
		
		// 로컬 오프셋을 월드 스페이스로 변환하여 적용
		FVector WorldOffset = Rotation.RotateVector(PositionOffset);
		FVector FinalLocation = Location + WorldOffset;
		
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			VFXToPlay,
			FinalLocation,
			Rotation,
			FVector::OneVector,
			true,
			true
		);
	}
}


void AGS_Merci::Multicast_PlayArrowShotSound_Implementation()
{
	if (CharacterAudioComponent)
	{
		CharacterAudioComponent->PlaySound(ArrowShotSound);
	}
}

void AGS_Merci::Multicast_PlayArrowEmptySound_Implementation()
{
	if (CharacterAudioComponent)
	{
		CharacterAudioComponent->PlaySound(ArrowEmptySound, true); // 로컬 플레이어에게만 재생
	}
}

void AGS_Merci::Client_PlayHitFeedbackSound_Implementation()
{
	if (CharacterAudioComponent)
	{
		CharacterAudioComponent->PlaySound(HitFeedbackSound, true); // 로컬 플레이어에게만 재생
	}
}

void AGS_Merci::Multicast_PlayBowPullSound_Implementation()
{
	if (CharacterAudioComponent)
	{
		CharacterAudioComponent->PlaySound(BowPullSound);
	}
}

void AGS_Merci::Multicast_PlayBowReleaseSound_Implementation()
{
	if (CharacterAudioComponent)
	{
		CharacterAudioComponent->PlaySound(BowReleaseSound);
	}
}

