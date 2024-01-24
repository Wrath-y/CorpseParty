// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "UnarmedWeapon.generated.h"

/**
 * 
 */
UCLASS()
class CORPSEPARTY_API AUnarmedWeapon : public AWeapon
{
	GENERATED_BODY()
public:
	virtual void Fire(const FVector& HitTarget) override;
	
protected:
	virtual void BeginPlay() override;

	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);
	
	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	USoundCue* HitSound;

private:
	UPROPERTY(EditAnywhere)
	USoundCue* FireSound;
	
};
