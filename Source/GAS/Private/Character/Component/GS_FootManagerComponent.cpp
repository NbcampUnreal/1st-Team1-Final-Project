// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Component/GS_FootManagerComponent.h"
#include "Character/Player/Guardian/GS_Drakhar.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/DecalComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "AkGameplayStatics.h"
#include "AkAudioDevice.h"
#include "AkComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "DrawDebugHelpers.h"
#include "HAL/IConsoleManager.h"

#if WITH_EDITOR
// Console variable for debug visualization
static TAutoConsoleVariable<int32> CVarShowFootstepDebug(
	TEXT("FootManager.ShowDebug"),
	0,
	TEXT("Show footstep debug traces and hit points\n")
	TEXT("0: Disabled (default)\n")
	TEXT("1: Enabled"),
	ECVF_Cheat
);
#endif

// Static socket name definitions
const FName UGS_FootManagerComponent::LeftFootSocketName = TEXT("foot_l_Socket");
const FName UGS_FootManagerComponent::RightFootSocketName = TEXT("foot_r_Socket");

// Sets default values for this component's properties
UGS_FootManagerComponent::UGS_FootManagerComponent()
{
	// Set this component to be ticked every frame.  You can turn this off to improve performance if not needed.
	PrimaryComponentTick.bCanEverTick = false;

	// Initialize cached skeletal mesh to nullptr
	CachedSkeletalMesh = nullptr;
	
	// Initialize footstep sound event to nullptr
	FootstepSoundEvent = nullptr;
	
	// Initialize minimum movement speed
	MinimumMovementSpeed = 3.0f;  // 작은 캐릭터용으로 더 높게 설정

	// Initialize footstep anti-spam settings
	FootstepCooldownTime = 0.25f;  // 발자국 간 최소 간격 (초)
	MinimumFootstepDistance = 70.0f;  // 마지막 발자국으로부터 최소 거리 (cm)
	LastFootstepTime = 0.0f;
	LastFootstepLocation = FVector::ZeroVector;

	// Initialize decal properties
	FootDecalSize = FVector(13.0f, 13.0f, 13.0f);
	FootDecalLifeSpan = 10.0f;
	DecalRotationOffsetYaw = 180.0f;

	// Set default switch group name
	SwitchGroupName = TEXT("FootSteps");

	// Initialize surface switch values
	SurfaceSwitchValues.Add(SurfaceType_Default, TEXT("Dirt"));
	SurfaceSwitchValues.Add(SurfaceType1, TEXT("Dirt"));
	SurfaceSwitchValues.Add(SurfaceType2, TEXT("Grass"));
	SurfaceSwitchValues.Add(SurfaceType3, TEXT("Metal"));
	SurfaceSwitchValues.Add(SurfaceType4, TEXT("Stone"));
	SurfaceSwitchValues.Add(SurfaceType5, TEXT("Lava"));

	// Initialize foot dust effects with asset references
	// Note: Asset paths should be adjusted based on your project structure
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> DirtEffect(TEXT("/Game/Material/A_Surface_Footstep/Niagara_FX/ParticleSystems/PSN_Dirt_Surface.PSN_Dirt_Surface"));
	if (DirtEffect.Succeeded())
	{
		FootDustEffects.Add(SurfaceType_Default, DirtEffect.Object);
		FootDustEffects.Add(SurfaceType1, DirtEffect.Object);
	}

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> GrassEffect(TEXT("/Game/Material/A_Surface_Footstep/Niagara_FX/ParticleSystems/PSN_Grass_Surface"));
	if (GrassEffect.Succeeded())
	{
		FootDustEffects.Add(SurfaceType2, GrassEffect.Object);
	}

	// Metal has no effect (intentionally left empty)
	FootDustEffects.Add(SurfaceType3, nullptr);

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> StoneEffect(TEXT("/Game/Material/A_Surface_Footstep/Niagara_FX/ParticleSystems/PSN_General1_Surface"));
	if (StoneEffect.Succeeded())
	{
		FootDustEffects.Add(SurfaceType4, StoneEffect.Object);
	}

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> LavaEffect(TEXT("/Game/Material/A_Surface_Footstep/Niagara_FX/ParticleSystems/PSN_Sparks_Surface"));
	if (LavaEffect.Succeeded())
	{
		FootDustEffects.Add(SurfaceType5, LavaEffect.Object);
	}
}

// Called when the game starts
void UGS_FootManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	
	CachedSkeletalMesh = GetOwnerSkeletalMesh();
	
	if (!CachedSkeletalMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("GS_FootManagerComponent: No SkeletalMeshComponent found on owner %s"), 
			*GetOwner()->GetName());
	}
}

void UGS_FootManagerComponent::HandleFootstep(EFootStep Foot)
{
	// Early return if no valid skeletal mesh
	if (!CachedSkeletalMesh)
	{
		return;
	}

	// Anti-spam protection: Check cooldown timer
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastFootstepTime < FootstepCooldownTime)
	{
		return; // Too soon since last footstep
	}

	// Anti-spam protection: Check minimum distance from last footstep
	const FVector CurrentLocation = GetOwner()->GetActorLocation();
	if (LastFootstepLocation != FVector::ZeroVector)
	{
		const float DistanceFromLastFootstep = FVector::Dist(CurrentLocation, LastFootstepLocation);
		if (DistanceFromLastFootstep < MinimumFootstepDistance)
		{
			return; // Too close to last footstep location
		}
	}

	// Perform line trace from foot socket
	FHitResult HitResult;
	if (!PerformFootTrace(Foot, HitResult))
	{
		// No valid ground hit, skip footstep effects
		return;
	}

	// Get the physical surface from hit result
	EPhysicalSurface SurfaceType = SurfaceType_Default;
	if (HitResult.PhysMaterial.IsValid())
	{
		SurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());
	}

	const FVector& HitLocation = HitResult.Location;
	const FVector& HitNormal = HitResult.Normal;

	// Update anti-spam tracking
	LastFootstepTime = CurrentTime;
	LastFootstepLocation = CurrentLocation;

	// Spawn footstep effects based on surface type
	SpawnFootstepDecal(SurfaceType, HitLocation, HitNormal, Foot);
	PlayFootstepSound(SurfaceType, HitLocation);
	SpawnFootDustEffect(SurfaceType, HitLocation, HitNormal);
}

void UGS_FootManagerComponent::OverrideFootDustEffect(UNiagaraSystem* VFXSystem)
{
	OverriddenFootDustEffect = VFXSystem;
}

void UGS_FootManagerComponent::ClearFootDustEffectOverride()
{
	OverriddenFootDustEffect = nullptr;
}

void UGS_FootManagerComponent::HandleFoleyEvent()
{
	// Early return if auto detection is disabled
	if (!bAutoDetectionEnabled)
	{
		return;
	}

	// Early return if no valid skeletal mesh
	if (!CachedSkeletalMesh)
	{
		return;
	}

	// Check if character is moving fast enough to trigger footsteps
	if (AActor* Owner = GetOwner())
	{
		FVector CharacterVelocity;
		
		if (ACharacter* Character = Cast<ACharacter>(Owner))
		{
			CharacterVelocity = Character->GetVelocity();
		}
		else if (APawn* Pawn = Cast<APawn>(Owner))
		{
			CharacterVelocity = Pawn->GetVelocity();
		}
		else
		{
			static FVector PreviousLocation = Owner->GetActorLocation();
			const FVector CurrentLocation = Owner->GetActorLocation();
			const float DeltaTime = GetWorld()->GetDeltaSeconds();
			
			if (DeltaTime > 0.0f)
			{
				CharacterVelocity = (CurrentLocation - PreviousLocation) / DeltaTime;
			}
			else
			{
				CharacterVelocity = FVector::ZeroVector;
			}
			
			PreviousLocation = CurrentLocation;
		}

		const float HorizontalSpeed = FVector(CharacterVelocity.X, CharacterVelocity.Y, 0.0f).Size();
		
		if (HorizontalSpeed < MinimumMovementSpeed)
		{
			return;
		}
	}

	EFootStep DetectedFoot = DetectActiveFootstep();
	
	HandleFootstep(DetectedFoot);
}

EFootStep UGS_FootManagerComponent::DetectActiveFootstep()
{
	if (!CachedSkeletalMesh)
	{
		return EFootStep::LeftFoot;
	}

	// Get foot socket locations
	const FVector LeftFootLocation = CachedSkeletalMesh->GetSocketLocation(LeftFootSocketName);
	const FVector RightFootLocation = CachedSkeletalMesh->GetSocketLocation(RightFootSocketName);

	// Method 1: Height-based detection (lower foot is more likely to be in contact)
	const float LeftFootHeight = LeftFootLocation.Z;
	const float RightFootHeight = RightFootLocation.Z;
	const float HeightDifference = FMath::Abs(LeftFootHeight - RightFootHeight);
	
	// Use the foot that is currently lower (closer to ground)
	if (HeightDifference > 1.0f)
	{
		return (LeftFootHeight < RightFootHeight) ? EFootStep::LeftFoot : EFootStep::RightFoot;
	}

	// Method 2: Velocity-based detection (fallback)
	static FVector PrevLeftFootLocation = LeftFootLocation;
	static FVector PrevRightFootLocation = RightFootLocation;
	
	const FVector LeftFootVelocity = LeftFootLocation - PrevLeftFootLocation;
	const FVector RightFootVelocity = RightFootLocation - PrevRightFootLocation;
	
	// Update previous locations for next frame
	PrevLeftFootLocation = LeftFootLocation;
	PrevRightFootLocation = RightFootLocation;
	
	// The foot with lower velocity is more likely to be in contact with ground
	const float LeftFootSpeed = LeftFootVelocity.Size();
	const float RightFootSpeed = RightFootVelocity.Size();
	
	// Method 3: Alternating fallback
	static EFootStep LastDetectedFoot = EFootStep::RightFoot;
	
	if (FMath::Abs(LeftFootSpeed - RightFootSpeed) < 0.5f)
	{
		// Speeds are similar, alternate between feet
		LastDetectedFoot = (LastDetectedFoot == EFootStep::LeftFoot) ? EFootStep::RightFoot : EFootStep::LeftFoot;
		return LastDetectedFoot;
	}
	
	// Return the foot with lower speed
	LastDetectedFoot = (LeftFootSpeed < RightFootSpeed) ? EFootStep::LeftFoot : EFootStep::RightFoot;
	return LastDetectedFoot;
}

bool UGS_FootManagerComponent::PerformFootTrace(EFootStep Foot, FHitResult& OutHitResult)
{
	if (!CachedSkeletalMesh)
	{
		return false;
	}

	// Get foot socket name based on foot type
	const FName SocketName = GetFootSocketName(Foot);
	
	// Check if socket exists (cache this check for better performance)
	if (!CachedSkeletalMesh->DoesSocketExist(SocketName))
	{
		return false;
	}

	// Get socket world location
	const FVector SocketLocation = CachedSkeletalMesh->GetSocketLocation(SocketName);
	const FVector TraceStart = SocketLocation;
	const FVector TraceEnd = SocketLocation + FVector(0.0f, 0.0f, -TraceDistance);

	// Setup optimized trace parameters
	FCollisionQueryParams TraceParams(FName("FootTrace"), false, GetOwner());
	TraceParams.bReturnPhysicalMaterial = true;
	TraceParams.bTraceComplex = false;  // 성능 최적화: Simple collision 사용
	
	// 성능 최적화: 불필요한 오브젝트 제외
	TraceParams.AddIgnoredActor(GetOwner());  // 자기 자신 제외
	
	// 추가 최적화: ResponseParams 설정으로 특정 오브젝트 타입만 체크
	FCollisionResponseParams ResponseParams;
	ResponseParams.CollisionResponse.SetResponse(ECC_Pawn, ECR_Ignore);
	ResponseParams.CollisionResponse.SetResponse(ECC_Vehicle, ECR_Ignore);

	// Perform optimized line trace
	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		OutHitResult,
		TraceStart,
		TraceEnd,
		TraceChannel,
		TraceParams,
		ResponseParams
	);

	#if WITH_EDITOR
	if (CVarShowFootstepDebug.GetValueOnGameThread() > 0)
	{
		const FColor DebugColor = bHit ? FColor::Green : FColor::Red;
		DrawDebugLine(GetWorld(), TraceStart, TraceEnd, DebugColor, false, 2.0f, 0, 2.0f);
		
		if (bHit)
		{
			DrawDebugPoint(GetWorld(), OutHitResult.Location, 8.0f, FColor::Yellow, false, 2.0f);
			DrawDebugLine(GetWorld(), OutHitResult.Location, OutHitResult.Location + (OutHitResult.Normal * 20.0f), FColor::Blue, false, 2.0f, 0, 1.0f);
		}
	}
	#endif

	return bHit;
}

void UGS_FootManagerComponent::SpawnFootstepDecal(EPhysicalSurface /*Surface*/, const FVector& Location, const FVector& Normal, EFootStep Foot)
{
	UMaterialInterface* DecalMaterial = (Foot == EFootStep::LeftFoot) ? LeftFootDecal : RightFootDecal;
	if (!DecalMaterial)
	{
		return;
	}

	// Normal 방향으로 1.0f만큼 올려서 Z파이팅 방지
	const FVector DecalSpawnLocation = Location + Normal * 1.0f;

	const FName SocketName = GetFootSocketName(Foot);
	const FRotator SocketRotation = CachedSkeletalMesh->GetSocketRotation(SocketName);

	// Normal 대신 고정된 Up 벡터를 사용하여 일관된 방향 계산
	const FVector FixedUpVector = FVector(0.0f, 0.0f, 1.0f);
	const FVector NegatedUp = -FixedUpVector;
	FRotator DecalRotation = FRotationMatrix::MakeFromX(NegatedUp).Rotator();

	// 캐릭터가 바라보는 방향으로 발자국 방향 결정
	const FRotator ActorRotation = GetOwner()->GetActorRotation();
	DecalRotation = FRotator(DecalRotation.Pitch, ActorRotation.Yaw, DecalRotation.Roll);

	// 방향 조정을 위해 Z축으로 -90도 회전 추가
	DecalRotation += FRotator(0.0f, 0.0f, -90.0f);
	
	// 에디터에서 설정 가능한 발자국 회전 오프셋 적용
	DecalRotation += FRotator(0.0f, DecalRotationOffsetYaw, 0.0f);

	UDecalComponent* SpawnedDecal = UGameplayStatics::SpawnDecalAtLocation(
		GetWorld(),
		DecalMaterial,
		FootDecalSize,
		DecalSpawnLocation,
		DecalRotation,
		FootDecalLifeSpan
	);

	if (SpawnedDecal)
	{
		OnFootDecalSpawned(SpawnedDecal, Foot);
	}
}

void UGS_FootManagerComponent::PlayFootstepSound(EPhysicalSurface Surface, const FVector& Location)
{
	if (!FAkAudioDevice::IsInitialized())
	{
		return;
	}

	FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get();
	if (!AkAudioDevice)
	{
		return;
	}

	// Check if we have a footstep sound event
	if (!FootstepSoundEvent)
	{
		return;
	}

	// Find switch value for this surface
	const FString* SwitchValue = SurfaceSwitchValues.Find(Surface);
	if (!SwitchValue || SwitchValue->IsEmpty())
	{
		return;
	}

	// Set switch value before playing sound
	AkAudioDevice->SetSwitch(
		*SwitchGroupName,
		**SwitchValue,
		GetOwner()
	);

	UAkComponent* AkComp = GetOwner()->FindComponentByClass<UAkComponent>();
	if (!AkComp)
	{
		return;
	}

	AkComp->SetWorldLocation(Location);

	const int32 PlayingID = AkComp->PostAkEvent(FootstepSoundEvent);

	if (PlayingID != AK_INVALID_PLAYING_ID)
	{
		OnFootSoundPlayed.Broadcast(Surface, Location);
	}
}

void UGS_FootManagerComponent::SpawnFootDustEffect(EPhysicalSurface Surface, const FVector& Location, const FVector& Normal)
{
	UNiagaraSystem* VFXToSpawn = nullptr;

	if (OverriddenFootDustEffect)
	{
		VFXToSpawn = OverriddenFootDustEffect;
	}
	else
	{
		UNiagaraSystem* const* FoundVFX = FootDustEffects.Find(Surface);
		if (FoundVFX && *FoundVFX)
		{
			VFXToSpawn = *FoundVFX;
		}
	}

	if (!VFXToSpawn)
	{
		return;
	}

	const FRotator EffectRotation = FRotationMatrix::MakeFromZ(Normal).Rotator();

	UNiagaraComponent* SpawnedEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		VFXToSpawn,
		Location,
		EffectRotation
	);

	if (SpawnedEffect)
	{
		OnFootDustSpawned(Surface, Location);
	}
}

FName UGS_FootManagerComponent::GetFootSocketName(EFootStep Foot) const
{
	switch (Foot)
	{
	case EFootStep::LeftFoot:
		return LeftFootSocketName;
	case EFootStep::RightFoot:
		return RightFootSocketName;
	default:
		return NAME_None;
	}
}

USkeletalMeshComponent* UGS_FootManagerComponent::GetOwnerSkeletalMesh() const
{
	if (!GetOwner())
	{
		return nullptr;
	}

	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		return Character->GetMesh();
	}

	return GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
} 