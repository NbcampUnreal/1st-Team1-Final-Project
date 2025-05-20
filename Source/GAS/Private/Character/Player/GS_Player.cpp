#include "Character/Player/GS_Player.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AGS_Player::AGS_Player()
{
	PrimaryActorTick.bCanEverTick = false;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->TargetArmLength = 400.f;
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->bUsePawnControlRotation = false;
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);

	TeamId = FGenericTeamId(1);
}

void AGS_Player::BeginPlay()
{
	Super::BeginPlay();
}

void AGS_Player::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

FCharacterWantsToMove AGS_Player::GetWantsToMove()
{
	return WantsToMove;
}

