#include "ResourceSystem/Aether/GS_AetherExtractor.h"


AGS_AetherExtractor::AGS_AetherExtractor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	RootSceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComp"));
	RootComponent = RootSceneComp;

	HPTextWidgetComp = CreateDefaultSubobject<UGS_HPTextWidgetComp>("HPTextWidgetComp");
	HPTextWidgetComp->SetupAttachment(RootComponent);
	HPTextWidgetComp->SetWidgetSpace(EWidgetSpace::World);
	HPTextWidgetComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HPTextWidgetComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	HPTextWidgetComp->SetVisibility(true);

	StatComp = CreateDefaultSubobject<UGS_StatComp>(TEXT("StatComp"));
}

void AGS_AetherExtractor::BeginPlay()
{
	Super::BeginPlay();
	StatComp->InitStat(FName("AetherExtractor"));


	if (!HasAuthority())
	{
		return;
	}
	
	FTimerHandle DelayHandle;
	GetWorld()->GetTimerManager().SetTimer(
		DelayHandle,
		this,
		&ThisClass::InitializeAetherComp,
		1.5f,  // 딜레이 후 초기화 시도
		false
	);


	//CachedAetherComp = FindGuardianAetherComp();

	//GetWorld()->GetTimerManager().SetTimer(
	//	AetherExtractTimerHandle,
	//	this,
	//	&ThisClass::ExtractAether,
	//	ExtractionInterval,
	//	true
	//);

}

UGS_AetherComp* AGS_AetherExtractor::FindGuardianAetherComp()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC)
		{
			UE_LOG(LogTemp, Warning, TEXT("PlayerController is null"));
			continue;
		}

		AGS_PlayerState* PS = Cast<AGS_PlayerState>(PC->PlayerState);
		/*if (PS && PS->CurrentPlayerRole == EPlayerRole::PR_Guardian)
		{
			UE_LOG(LogTemp, Warning, TEXT("AetherComp found"));
			return PS->GetAetherComp();
		}*/

		if (!PS)
		{
			UE_LOG(LogTemp, Warning, TEXT("PlayerState is not GS_PlayerState"));
			continue;
		}

		UE_LOG(LogTemp, Warning, TEXT("Current PlayerRole: %s"),
			PS ? *UEnum::GetValueAsString(PS->CurrentPlayerRole) : TEXT("No GS_PlayerState"));


		if (PS->CurrentPlayerRole != EPlayerRole::PR_Guardian)
		{
			UE_LOG(LogTemp, Warning, TEXT("PlayerRole is not Guardian"));
			continue;
		}

		if (!PS->GetAetherComp())
		{
			UE_LOG(LogTemp, Warning, TEXT("AetherComp is null on Guardian PlayerState"));
			continue;
		}

		UE_LOG(LogTemp, Warning, TEXT("AetherComp found successfully!"));
		return PS->GetAetherComp();
	}
	UE_LOG(LogTemp, Warning, TEXT("AetherComp not found"));
	return nullptr;
}


void AGS_AetherExtractor::InitializeAetherComp()
{

	CachedAetherComp = FindGuardianAetherComp();

	GetWorld()->GetTimerManager().SetTimer(
		AetherExtractTimerHandle,
		this,
		&ThisClass::ExtractAether,
		ExtractionInterval,
		true
	);
}



void AGS_AetherExtractor::ExtractAether()
{
	if (IsValid(CachedAetherComp))
	{
		CachedAetherComp->AddResource(ExtractionAmount);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AetherComp in PlayerState is lost"));
	}
}


//공격 받았을 때 데미지 처리
void AGS_AetherExtractor::TakeDamageBySeeker(float DamageAmount, AActor* DamageCauser)
{
	UE_LOG(LogTemp, Warning, TEXT("[AetherExtractor]TakeDamageBySeeker is called"));
	if (!StatComp || DamageAmount <= 0.0f)
	{
		return;
	}
	//float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	float CurrentHealth = StatComp->GetCurrentHealth();
	float NewHealth = CurrentHealth - DamageAmount;

	StatComp->SetCurrentHealth(NewHealth, false);
	////만약 갱신한 값이 0 이하라면(0이라면) 파괴
	//if (NewHealth <= KINDA_SMALL_NUMBER)
	//{
	//	//위젯 삭제 코드 추가 필요
	//	//if (ExtractorWidget && ExtractorWidget->IsInViewport())
	//	/*{
	//		ExtractorWidget->RemoveFromParent();
	//	}*/
	//	Destroy();

	//}
}

void AGS_AetherExtractor::DestroyAetherExtractor()
{
	//위젯 파괴 추가하기
	Destroy();
}

void AGS_AetherExtractor::SetHPTextWidget(UGS_HPText* InHPTextWidget)
{
	UE_LOG(LogTemp, Warning, TEXT("[AetherExtractor]SetHPTextWidget is called"));
	UGS_HPText* HPTextWidget = Cast<UGS_HPText>(InHPTextWidget);
	if (IsValid(HPTextWidget))
	{
		UE_LOG(LogTemp, Warning, TEXT("[AetherExtractor] StatComp: %s, Current=%.1f, Max=%.1f"),
			*GetNameSafe(StatComp),
			StatComp ? StatComp->GetCurrentHealth() : -1.f,
			StatComp ? StatComp->GetMaxHealth() : -1.f);
		HPTextWidget->InitializeHPTextWidget(GetStatComp());
		StatComp->OnCurrentHPChanged.AddUObject(HPTextWidget, &UGS_HPText::OnCurrentHPChanged);
	}
}
