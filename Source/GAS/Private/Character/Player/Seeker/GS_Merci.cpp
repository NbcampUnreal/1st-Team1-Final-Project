// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Seeker/GS_Merci.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon/Projectile/Seeker/GS_SeekerMerciArrow.h"
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
}

void AGS_Merci::LeftClickPressedAttack(UAnimMontage* DrawMontage)
{
	if (!GetDrawState())
	{
		if (Mesh && DrawMontage)
		{
			float Duration = Mesh->GetAnimInstance()->Montage_Play(DrawMontage, 2.0f);
			if (Duration > 0.0f)
			{
				UE_LOG(LogTemp, Warning, TEXT("Draw"));
				FOnMontageEnded EndDelegate;
				EndDelegate.BindUObject(this, &AGS_Merci::OnDrawMontageEnded);
				Mesh->GetAnimInstance()->Montage_SetEndDelegate(EndDelegate, DrawMontage);
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

		SetDrawState(true); // 상태 전환

		// 사운드 재생

		StartZoom(); // 줌인
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("bIsDrawState true"));
	}
}

void AGS_Merci::LeftClickReleaseAttack(TSubclassOf<AGS_SeekerMerciArrow> ArrowClass)
{
	UE_LOG(LogTemp, Warning, TEXT("Frie"));
	SetAimState(false);
	SetDrawState(false);

	// 사운드 재생

	FireArrow(ArrowClass);

	StopZoom();

	SetWidgetVisibility(false);
}

void AGS_Merci::FireArrow(TSubclassOf<AGS_SeekerMerciArrow> ArrowClass)
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

	// 4. 사운드 재생
	if (ArrowShotSound_C)
	{

	}
}

// Called when the game starts or when spawned
void AGS_Merci::BeginPlay()
{
	Super::BeginPlay();
	Mesh = this->GetMesh();

	if (ZoomCurve)
	{
		FOnTimelineFloat TimelineCallback;
		TimelineCallback.BindUFunction(this, FName("UpdateZoom"));

		ZoomTimeline.AddInterpFloat(ZoomCurve, TimelineCallback);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ZoomCurve null"));
	}
}

void AGS_Merci::UpdateZoom(float Alpha)
{
	if (!SpringArmComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpringArmComp null"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Update Zoom"));
	float TargetArmLength = FMath::Lerp(400.0f, 180.0f, Alpha);
	float SocketOffsetY = FMath::Lerp(0.f, 20.f, Alpha);
	float SocketOffsetZ = FMath::Lerp(0.f, -40.0f, Alpha);

	SpringArmComp->TargetArmLength = TargetArmLength;
	FVector OffSet(0.f, SocketOffsetY, SocketOffsetZ);
	SpringArmComp->SocketOffset = OffSet;
}

void AGS_Merci::OnDrawMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!bInterrupted)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnDrawMontageEnded bInterrupted = false"));
		SetWidgetVisibility(true); // 크로스 헤어 보이기
		SetAimState(true);
		SetDrawState(false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("OnDrawMontageEnded bInterrupted = true"));
		SetWidgetVisibility(false); // 실패 시 숨기기
		SetDrawState(false);
	}
}

void AGS_Merci::SetWidgetVisibility(bool bVisible)
{
	if (WidgetCrosshair)
	{
		WidgetCrosshair->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
		UE_LOG(LogTemp, Warning, TEXT("Set Widget Visibility"));
	}
}

void AGS_Merci::StartZoom()
{
	UE_LOG(LogTemp, Warning, TEXT("Start Zoom"));
	ZoomTimeline.Play(); // 줌인
}

void AGS_Merci::StopZoom()
{
	UE_LOG(LogTemp, Warning, TEXT("Stop Zoom"));
	ZoomTimeline.Reverse(); // 줌아웃
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

