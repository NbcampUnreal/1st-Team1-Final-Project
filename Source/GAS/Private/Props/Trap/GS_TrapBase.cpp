#include "Props/Trap/GS_TrapBase.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Engine/DamageEvents.h"

AGS_TrapBase::AGS_TrapBase()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);

	RootSceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComp"));
	RootComponent = RootSceneComp;

	RotationSceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("RotationScene"));
	RotationSceneComp->SetupAttachment(RootComponent);

	MeshParentSceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("MeshParentSceneComp"));
	MeshParentSceneComp->SetupAttachment(RotationSceneComp);

	DamageBoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageBox"));
	DamageBoxComp->SetupAttachment(MeshParentSceneComp);
	// DamageBox 콜리전 설정
	DamageBoxComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	DamageBoxComp->SetCollisionObjectType(ECC_WorldDynamic);
	DamageBoxComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	DamageBoxComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}


void AGS_TrapBase::BeginPlay()
{
	Super::BeginPlay();

	LoadTrapData();
	DamageBoxComp->OnComponentBeginOverlap.AddDynamic(this, &AGS_TrapBase::OnDamageBoxOverlap);
}

void AGS_TrapBase::LoadTrapData()
{
	if (!TrapDataTable) return;
	FTrapData* FoundTrapData = TrapDataTable->FindRow<FTrapData>(TrapID, TEXT("LoadTrapData"));
	if (FoundTrapData)
	{
		TrapData = *FoundTrapData;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TrapData not found for TrapID : %s"), *TrapID.ToString());
	}
}

//데미지 박스에 오버랩된 경우 HandleTrapDamage 함수 실행
void AGS_TrapBase::OnDamageBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	AGS_Seeker* Seeker = Cast<AGS_Seeker>(OtherActor);
	if (!Seeker)
	{
		return;
	}
	if (!HasAuthority())
	{
		//클라이언트
		Server_DamageBoxEffect(Seeker);
		Server_CustomTrapEffect(Seeker);
		Server_HandleTrapDamage(Seeker);
	}
	else
	{
		//서버
		DamageBoxEffect(Seeker);
		CustomTrapEffect(Seeker);
		HandleTrapDamage(Seeker);
		
	}
}

void AGS_TrapBase::Server_HandleTrapDamage_Implementation(AActor* OtherActor)
{
	HandleTrapDamage(OtherActor);
}


void AGS_TrapBase::HandleTrapDamage(AActor* OtherActor)
{
	UE_LOG(LogTemp, Warning, TEXT("Player is damaged"));
	if (!OtherActor) return;
	AGS_Seeker* DamagedSeeker = Cast<AGS_Seeker>(OtherActor);
	if (!DamagedSeeker) return;
	if (TrapData.Effect.Damage <= 0.f) return;
	UE_LOG(LogTemp, Warning, TEXT("Passed lv 1"));
 	//기본 데미지 부여
	FDamageEvent DamageEvent;

	
	if (TrapData.Effect.bDoT)
	{
		UE_LOG(LogTemp, Warning, TEXT("TrapData.Effect.bDoT True"));
		ApplyDotDamage(OtherActor);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TrapData.Effect.bDoT false"));
		DamagedSeeker->TakeDamage(TrapData.Effect.Damage, DamageEvent, nullptr, this);
	}


	//디버프 추가

}

void AGS_TrapBase::HandleTrapAreaDamage(const TArray<AActor*>& AffectedActors)
{
}


void AGS_TrapBase::ApplyDotDamage(AActor* DamagedActor)
{
	if (!DamagedActor || !HasAuthority() || !TrapData.Effect.bDoT) return;

	if (ActiveDoTTimers.Contains(DamagedActor))
	{
		//이미 해당 엑터의 타이머가 작동 중이라면 타이머 초기화(처음부터 다시 시작)
		GetWorld()->GetTimerManager().ClearTimer(ActiveDoTTimers[DamagedActor]);
	}

	int32 CurrentTick = 0;
	FTimerHandle TimerHandle;
	FTimerDelegate Delegate;
	//타이머가 끝나면 실행되는 람다
	Delegate.BindLambda([=, this]() mutable
		{
			if (!DamagedActor || !TrapData.Effect.bDoT) return;

			FDamageEvent DamageEvent;
			DamagedActor->TakeDamage(TrapData.Effect.Damage, DamageEvent, nullptr, this);
			UE_LOG(LogTemp, Warning, TEXT("CurrentTick : %d"), CurrentTick);
			CurrentTick++;

			if (CurrentTick >= TrapData.Effect.DamageCount)
			{
				//current tick이 damage count보다 같거나 크다면 타이머 초기화 후 ActiveDoTTimers 맵에서 제거
				GetWorld()->GetTimerManager().ClearTimer(ActiveDoTTimers[DamagedActor]);
				ActiveDoTTimers.Remove(DamagedActor);
			}
		});

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, Delegate, TrapData.Effect.DamageInterval, true);
	ActiveDoTTimers.Add(DamagedActor, TimerHandle);
}



void AGS_TrapBase::Server_DamageBoxEffect_Implementation(AActor* OtherActor)
{
	UE_LOG(LogTemp, Warning, TEXT("Server_DamageBoxEffect_Implementation called"));
	//Multicast_DamageBoxEffect(OtherActor);
	DamageBoxEffect(OtherActor);
}


void AGS_TrapBase::Multicast_DamageBoxEffect_Implementation(AActor* TargetActor)
{
	UE_LOG(LogTemp, Warning, TEXT("Multicast_DamageBoxEffect_Implementation called"));
	DamageBoxEffect(TargetActor);
}

void AGS_TrapBase::DamageBoxEffect_Implementation(AActor* OtherActor)
{
	UE_LOG(LogTemp, Warning, TEXT("DamageBoxEffect Applied"));
}


void AGS_TrapBase::Server_CustomTrapEffect_Implementation(AActor* TargetActor)
{
	CustomTrapEffect(TargetActor);
}

void AGS_TrapBase::CustomTrapEffect_Implementation(AActor* TargetActor)
{

}


//플레이어가 안에 있는 경우 밀쳐내는 함수
void AGS_TrapBase::PushCharacterInBox(UBoxComponent* CollisionBox, float PushPower)
{
	if (!CollisionBox) return;

	TArray<AActor*> OverlappingActors;
	CollisionBox->GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());

	for (AActor* Actor : OverlappingActors)
	{
		AGS_Character* Character = Cast<AGS_Character>(Actor);
		if (Character)
		{
			FVector LocalCharacterLocation = GetActorTransform().InverseTransformPosition(Character->GetActorLocation());
			FVector PushDirection = (LocalCharacterLocation.Y >= 0.0f)
				? GetActorRightVector()
				: -GetActorRightVector();

			if (IsBlockedInDirection(Character->GetActorLocation(), PushDirection, 100.0f, Character))
			{
				PushDirection *= -1.0f;
			}

			PushDirection.Z = 0.0f;
			PushDirection = PushDirection.GetSafeNormal();

			FVector LaunchVelocity = PushDirection * PushPower + FVector(0, 0, 200.0f);

			Character->LaunchCharacter(LaunchVelocity, true, true);
		}
	}

}


bool AGS_TrapBase::IsBlockedInDirection(const FVector& Start, const FVector& Direction, float Distance,  AGS_Character* CharacterToIgnore)
{
	FHitResult HitResult;
	FVector End = Start + Direction * Distance;
	
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (CharacterToIgnore)
	{
		Params.AddIgnoredActor(CharacterToIgnore);
	}

	#if WITH_EDITOR
		DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1.0f, 0, 2.0f);
	#endif

	return GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, Params);
}