#include "ResourceSystem/Aether/GS_AetherExtractor.h"
#include "AI/RTS/GS_RTSController.h"

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
	
	UE_LOG(LogTemp, Warning, TEXT("[AetherExtractor::BeginPlay] NumControllers=%d"),
		GetWorld()->GetNumPlayerControllers());


	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AGS_RTSController* RTSController = Cast<AGS_RTSController>(It->Get());
		if (!RTSController)
		{
			continue;
		}

		//찾은 경우
		if (IsValid(RTSController->AetherComp))
		{
			CachedAetherComp = RTSController->AetherComp;
			UE_LOG(LogTemp, Warning, TEXT("  -> RTSController %s, AetherComp=%s"),
				*GetNameSafe(RTSController),
				*GetNameSafe(RTSController->AetherComp));
			InitializeAetherComp();
			break;
		}
	}
}



void AGS_AetherExtractor::RegisterRTSController(AGS_RTSController* InController)
{
	if (!InController)
	{
		return;
	}
	if (IsValid(InController->GetAetherComp()))
	{
		UE_LOG(LogTemp, Error, TEXT("[RegisterRTSController]RTS Controller found!"));
		CachedAetherComp = InController->GetAetherComp();
		InitializeAetherComp();
	}
	else
	{
		InController->OnAetherCompReady.AddDynamic(this, &ThisClass::HandleAetherCompReady);
	}

}

void AGS_AetherExtractor::HandleAetherCompReady(UGS_AetherComp* AetherComp)
{
	UE_LOG(LogTemp, Error, TEXT("[HandleAetherCompReady] AetherComp broadcasted and found"));
	CachedAetherComp = AetherComp;
	InitializeAetherComp();
}



void AGS_AetherExtractor::InitializeAetherComp()
{
	if (!HasAuthority())
	{
		return;
	}
	UE_LOG(LogTemp, Error, TEXT("[InitializeAetherComp] Timer Start!"));
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
	if (!HasAuthority())
	{
		return;
	}
	UE_LOG(LogTemp, Error, TEXT("[ExtractAether] AetherExtractor Extracting..."));
	if (IsValid(CachedAetherComp))
	{
		CachedAetherComp->AddResource(ExtractionAmount);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AetherComp in RTSController is lost"));
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



void AGS_AetherExtractor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (!GetWorld())
	{
		return;
	}
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{
		if (APlayerController* PC = It->Get())
		{
			if (AGS_RTSController* RTSController = Cast<AGS_RTSController>(PC))
			{
				if (IsValid(RTSController))
				{
					RTSController->OnAetherCompReady.RemoveDynamic(this, &ThisClass::HandleAetherCompReady);
				}
			}
		}
	}
}