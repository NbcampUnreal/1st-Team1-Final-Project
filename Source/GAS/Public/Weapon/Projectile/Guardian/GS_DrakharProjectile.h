#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile/GS_WeaponProjectile.h"
#include "GS_DrakharProjectile.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;

UCLASS()
class GAS_API AGS_DrakharProjectile : public AGS_WeaponProjectile
{
	GENERATED_BODY()
	
public:
	AGS_DrakharProjectile();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse,
		const FHitResult& Hit) override;

	// 인디케이터 VFX 설정 함수 (InIndicatorRadius는 Scale_All 계산에 사용)
	UFUNCTION(BlueprintCallable, Category = "Indicator")
	void SetIndicatorVFX(UNiagaraSystem* InIndicatorVFX, float InIndicatorRadius);

protected:
	// === 인디케이터 생성 관련 메서드 ===

	// 지면에 인디케이터 생성 (메인 함수)
	void SpawnGroundIndicator();

	// 투사체 궤적을 예측하여 충돌 지점 반환
	bool PredictProjectileImpactLocation(FVector& OutImpactLocation);

	// 특정 지점에서 지면을 찾아 위치 반환
	bool FindGroundLocation(const FVector& TraceStartPoint, FVector& OutGroundLocation);

	// 인디케이터 나이아가라 컴포넌트 생성 및 설정
	void CreateAndConfigureIndicator(const FVector& Location);

	// 딜레이 후 인디케이터 활성화 스케줄링
	void ScheduleIndicatorActivation();

	// === 충돌 처리 헬퍼 메서드 ===

	// 캐릭터에게 데미지 적용 시도 (성공 시 true 반환)
	bool TryApplyDamageToCharacter(AActor* HitActor);

	// 소유자(Drakhar)에게 충돌 이벤트 알림
	void NotifyOwnerOfImpact(const FHitResult& Hit, bool bHitCharacter);

	// OnRep 함수 - IndicatorVFX가 리플리케이트될 때 호출
	UFUNCTION()
	void OnRep_IndicatorVFX();

	// === 설정 가능한 프로퍼티 ===

	// 디버그: 투사체 궤적 시각화 여부
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bShowProjectilePath = false;

	// 인디케이터 VFX 에셋 (리플리케이트)
	UPROPERTY(ReplicatedUsing = OnRep_IndicatorVFX)
	UNiagaraSystem* IndicatorVFX;

	// 인디케이터 나이아가라 컴포넌트
	UPROPERTY()
	UNiagaraComponent* IndicatorComponent;

	// 인디케이터 반경 (리플리케이트)
	UPROPERTY(Replicated)
	float IndicatorRadius;

	// === 궤적 예측 설정값 ===

	// 지면 탐지 트레이스 상향 오프셋
	UPROPERTY(EditAnywhere, Category = "Indicator|Trace", meta = (ClampMin = "0.0"))
	float GroundTraceUpOffset = 5000.0f;

	// 지면 탐지 트레이스 하향 오프셋
	UPROPERTY(EditAnywhere, Category = "Indicator|Trace", meta = (ClampMin = "0.0"))
	float GroundTraceDownOffset = 10000.0f;

	// 궤적 예측 최대 시뮬레이션 시간 (초)
	UPROPERTY(EditAnywhere, Category = "Indicator|Prediction", meta = (ClampMin = "0.1", ClampMax = "30.0"))
	float MaxPredictionTime = 10.0f;

	// 궤적 예측 시뮬레이션 주파수
	UPROPERTY(EditAnywhere, Category = "Indicator|Prediction", meta = (ClampMin = "10.0", ClampMax = "120.0"))
	float SimulationFrequency = 30.0f;

	// 디버그 트레이스 표시 시간 (초)
	UPROPERTY(EditAnywhere, Category = "Debug", meta = (ClampMin = "0.1", ClampMax = "60.0"))
	float DebugTraceDisplayTime = 5.0f;

	// 인디케이터 활성화 딜레이 (초) - 반짝거림 방지
	UPROPERTY(EditAnywhere, Category = "Indicator|Visual", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float IndicatorActivationDelay = 0.1f;

	// 인디케이터 스폰 딜레이 파라미터 값 (나이아가라)
	UPROPERTY(EditAnywhere, Category = "Indicator|Visual", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float IndicatorSpawnDelay = 0.15f;

	// 인디케이터 기본 반경 (스케일 계산용 기준값)
	UPROPERTY(EditAnywhere, Category = "Indicator|Visual", meta = (ClampMin = "1.0"))
	float DefaultIndicatorRadius = 250.0f;

	// 지면을 찾지 못했을 때 사용할 기본 Z 위치
	UPROPERTY(EditAnywhere, Category = "Indicator|Trace")
	float FallbackGroundZPosition = -1000.0f;

private:
	// 인디케이터 활성화 타이머 핸들 (레벨 전환 시 정리용)
	FTimerHandle IndicatorActivateTimerHandle;

	// 중복 충돌 방지 플래그
	bool bHasHitTarget;

	// 인디케이터 스케일 캐싱 (성능 최적화)
	float CachedIndicatorScale;

	// 안전한 정리를 위한 헬퍼 함수
	void CleanupIndicator();
	bool IsWorldContextValid() const;

	// 타이머 정리 함수 (레벨 전환 시 크래시 방지)
	void SafeClearTimer(FTimerHandle& TimerHandle);

	// 투사체 파괴 함수
	void SafeDestroyProjectile();
};
