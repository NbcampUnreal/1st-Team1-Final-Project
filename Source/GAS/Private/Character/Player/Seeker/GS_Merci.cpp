// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Seeker/GS_Merci.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon/Projectile/Seeker/GS_SeekerMerciArrow.h"
#include "Character/Component/Seeker/GS_MerciSkillInputHandlerComp.h"
#include "AkGameplayTypes.h"
#include "AkComponent.h"
#include "AkAudioEvent.h"
#include "Kismet/GameplayStatics.h"
#include "Templates/Function.h"
#include "DrawDebugHelpers.h"
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

void AGS_Merci::LeftClickPressedAttack(UAnimMontage* DrawMontage)
{
	if (!HasAuthority())
	{
		// 서버에 요청
		Server_LeftClickPressedAttack(DrawMontage);
		return;
	}

	if (!GetDrawState())
	{
		UE_LOG(LogTemp, Warning, TEXT("GetDrawState=false pass: %s"), GetDrawState()?TEXT("true") : TEXT("false"));
		Multicast_PlayDrawMontage(DrawMontage);
		//PlayDrawMontage(DrawMontage);
		SetDrawState(true); // 상태 전환
		
		// 활 당기는 사운드 재생
		PlaySound(BowPullSound);
		Client_StartZoom(); // 줌인
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("bIsDrawState true"));
	}
}

void AGS_Merci::LeftClickReleaseAttack(TSubclassOf<AGS_SeekerMerciArrow> ArrowClass, float SpreadAngleDeg, int32 NumArrows)
{
	if (!HasAuthority())
	{
		Server_LeftClickReleaseAttack(ArrowClass);
		return;
	}

	SetAimState(false);
	SetDrawState(false);
	Multicast_StopDrawMontage();
	
	if (bIsFullyDrawn)
	{
		// 활 놓는 사운드 재생
		PlaySound(BowReleaseSound);

		Server_FireArrow(ArrowClass, SpreadAngleDeg, NumArrows);

		bIsFullyDrawn = false;  // 상태 초기화

		// 화살 발사 사운드 재생
		PlaySound(ArrowShotSound);  // 부모 클래스의 PlaySound 함수 사용
	}

	Client_StopZoom();
	Client_SetWidgetVisibility(false);
}

void AGS_Merci::Server_LeftClickPressedAttack_Implementation(UAnimMontage* DrawMontage)
{
	LeftClickPressedAttack(DrawMontage);
}

void AGS_Merci::Server_LeftClickReleaseAttack_Implementation(TSubclassOf<AGS_SeekerMerciArrow> ArrowClass, float SpreadAngleDeg, int32 NumArrows)
{
	LeftClickReleaseAttack(ArrowClass);
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

	// 1. 카메라 위치와 회전값(플레이어의 시점) 가져오기
	FVector ViewLoc;
	FRotator ViewRot;
	Controller->GetPlayerViewPoint(ViewLoc, ViewRot); // 카메라 기준 시점

	// 2. 카메라에서 정면으로 Ray를 쏨
	FVector TraceStart = ViewLoc;
	FVector TraceEnd = TraceStart + ViewRot.Vector() * 5000.0f;
	//Multicast_DrawDebugLine(TraceStart, TraceEnd, FColor::Green);

	// 3. Ray가 무언가에 부딪히면 그 위치를 목표로 설정, 아니면 끝 지점 사용
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FVector TargetLocation = TraceEnd;

	if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
	{
		TargetLocation = Hit.ImpactPoint;
	}

	// 4. 무기의 소켓 위치에서 목표 위치로 향하는 방향 계산
	FVector SpawnLocation = Weapon->GetSocketLocation("BowstringSocket");
	FVector LaunchDirection = (TargetLocation - SpawnLocation).GetSafeNormal();
	FRotator BaseRotation = LaunchDirection.Rotation();

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
		FRotator SpreadRot = FRotator(0.f, OffsetAngle, 0.f);
		FVector SpreadDir = SpreadRot.RotateVector(LaunchDirection);
		FRotator ArrowRot = SpreadDir.Rotation();

		// 6. 화살 스폰
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.Instigator = this;

		GetWorld()->SpawnActor<AGS_SeekerMerciArrow>(ArrowClass, SpawnLocation, ArrowRot, SpawnParams);

		// 7. 화살 방향 시각화 (Spread 적용된 방향)
		//Multicast_DrawDebugLine(SpawnLocation, SpawnLocation + SpreadDir * 1000.f, FColor::Red);
	}
}

// Called when the game starts or when spawned
void AGS_Merci::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocallyControlled()) // 꼭 필요!
	{
		if (WidgetCrosshairClass)
		{
			WidgetCrosshair = CreateWidget<UUserWidget>(GetWorld(), WidgetCrosshairClass);
			if (WidgetCrosshair)
			{
				WidgetCrosshair->AddToViewport();
				WidgetCrosshair->SetVisibility(ESlateVisibility::Hidden);
			}
		}
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

void AGS_Merci::UpdateZoom(float Alpha)
{
	if (!SpringArmComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpringArmComp null"));
		return;
	}

	float TargetArmLength = FMath::Lerp(400.0f, 180.0f, Alpha);
	float SocketOffsetY = FMath::Lerp(0.f, 20.f, Alpha);
	float SocketOffsetZ = FMath::Lerp(0.f, -40.0f, Alpha);

	SpringArmComp->TargetArmLength = TargetArmLength;
	FVector OffSet(0.f, SocketOffsetY, SocketOffsetZ);
	SpringArmComp->SocketOffset = OffSet;
}

//void AGS_Merci::PlayBowPullSound(UAkComponent* AkComp)
//{
//	if (!PullSoundComp || !BowPullEvent)
//	{
//		UE_LOG(LogTemp, Warning, TEXT("AkComponent or Event is null"));
//		return;
//	}
//
//	FOnAkPostEventCallback Callback;
//	Callback.BindLambda([](EAkCallbackType CallbackType, AkCallbackInfo* CallbackInfo)
//		{
//			if (CallbackType == EAkCallbackType::EndOfEvent)
//			{
//				UE_LOG(LogTemp, Log, TEXT("Bow Pull Sound finished playing."));
//				// 여기에 완료 후 로직 추가
//			}
//		});
//
//	PullSoundComp->PostAkEvent(
//		BowPullEvent,
//		0,                   // Callback Mask
//		Callback,            // Callback Delegate
//		nullptr              // Cookie
//	);
//}

void AGS_Merci::Multicast_DrawDebugLine_Implementation(FVector Start, FVector End, FColor Color)
{
	DrawDebugLine(GetWorld(), Start, End, Color, false, 5.0f, 0, 3.0f);
}

void AGS_Merci::OnDrawMontageEnded()
{
	bIsFullyDrawn = true;  // 활 완전히 당김 상태 설정
	Client_SetWidgetVisibility(true); // 크로스 헤어 보이기

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
	//if (!bInterrupted)
	//{
	//	// 서버에서 처리할 로직
	//	SetAimState(true);
	//	SetDrawState(false);
	//}
	//else
	//{
	//	SetDrawState(false);
	//}
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

