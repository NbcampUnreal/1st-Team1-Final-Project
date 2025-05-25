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

	ShotSoundComp = CreateDefaultSubobject<UAkComponent>(TEXT("Shot Sound"));
	ShotSoundComp->SetupAttachment(GetMesh());
	ShotSoundComp->bAutoActivate = false;

	ReleaseSoundComp = CreateDefaultSubobject<UAkComponent>(TEXT("Release Sound"));
	ReleaseSoundComp->SetupAttachment(GetMesh());
	ReleaseSoundComp->bAutoActivate = false;

	PullSoundComp = CreateDefaultSubobject<UAkComponent>(TEXT("Pull Sound"));
	PullSoundComp->SetupAttachment(GetMesh());
	PullSoundComp->bAutoActivate = false;

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
		Multicast_PlayDrawMontage(DrawMontage);
		//PlayDrawMontage(DrawMontage);
		SetDrawState(true); // 상태 전환

		// 사운드 재생
		// 사운드 재생
		/*PlayBowPullSound(PullSoundComp);*/

		Client_StartZoom(); // 줌인
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("bIsDrawState true"));
	}
}

void AGS_Merci::LeftClickReleaseAttack(TSubclassOf<AGS_SeekerMerciArrow> ArrowClass)
{
	if (!HasAuthority())
	{
		Server_LeftClickReleaseAttack(ArrowClass);
		return;
	}

	SetAimState(false);
	SetDrawState(false);

	Multicast_StopDrawMontage();

	// 사운드 재생

	if (ReleaseSoundComp)
	{
		ReleaseSoundComp->PostAssociatedAkEvent(0, FOnAkPostEventCallback());
		UE_LOG(LogTemp, Warning, TEXT("ReleaseSound"));
	}

	Server_FireArrow(ArrowClass);

	if (ShotSoundComp)
	{
		ShotSoundComp->PostAssociatedAkEvent(0, FOnAkPostEventCallback());
		UE_LOG(LogTemp, Warning, TEXT("ShotSound"));
	}

	Client_StopZoom();

	Client_SetWidgetVisibility(false);
}

void AGS_Merci::Server_LeftClickPressedAttack_Implementation(UAnimMontage* DrawMontage)
{
	LeftClickPressedAttack(DrawMontage);
}

void AGS_Merci::Server_LeftClickReleaseAttack_Implementation(TSubclassOf<AGS_SeekerMerciArrow> ArrowClass)
{
	LeftClickReleaseAttack(ArrowClass);
}

void AGS_Merci::PlayDrawMontage(UAnimMontage* DrawMontage)
{
	UE_LOG(LogTemp, Warning, TEXT("DrawMontage valid: %s"), *GetNameSafe(DrawMontage));
	if (Mesh && DrawMontage)
	{
		float Duration = Mesh->GetAnimInstance()->Montage_Play(DrawMontage);
		if (Duration > 0.0f)
		{
			//FOnMontageEnded EndDelegate;
			//EndDelegate.BindUObject(this, &AGS_Merci::OnDrawMontageEnded);
			//Mesh->GetAnimInstance()->Montage_SetEndDelegate(EndDelegate, DrawMontage);
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
	Mesh->GetAnimInstance()->Montage_Stop(0.2f); // BlendOut 0.2초
}

void AGS_Merci::Multicast_PlayDrawMontage_Implementation(UAnimMontage* Montage)
{
	UE_LOG(LogTemp, Warning, TEXT("Multicast_PlayDrawMontage called on %s"), *GetName());
	UE_LOG(LogTemp, Warning, TEXT("Multicast_PlayDrawMontage called on %s, Montage: %s"),
		*GetName(),
		*GetNameSafe(Montage));
	PlayDrawMontage(Montage);
}

void AGS_Merci::Server_FireArrow_Implementation(TSubclassOf<AGS_SeekerMerciArrow> ArrowClass)
{
	if (!ArrowClass || !Weapon)
	{
		return;
	}

	// 1. 소켓 위치 얻기
	FVector SpawnLocation = Weapon->GetSocketLocation("BowstringSocket");

	// 2. 회전 방향 얻기(컨트롤러의 에임 방향)
	FRotator SpawnRotation = GetController()->GetDesiredRotation();

	// 3. 액터 스폰
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Instigator = this;

	GetWorld()->SpawnActor<AGS_SeekerMerciArrow>(ArrowClass, SpawnLocation, SpawnRotation, SpawnParams);
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

void AGS_Merci::OnDrawMontageEnded()
{
	Client_SetWidgetVisibility(true); // 크로스 헤어 보이기
	//if (!bInterrupted)
	//{
	//	Client_SetWidgetVisibility(true); // 크로스 헤어 보이기
	//}
	//else
	//{
	//	Client_SetWidgetVisibility(false); // 실패 시 숨기기
	//}

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

