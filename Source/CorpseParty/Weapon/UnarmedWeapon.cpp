// Fill out your copyright notice in the Description page of Project Settings.


#include "UnarmedWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "CorpseParty/Character/CorpsePartyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"
#include "CorpseParty/Character/EnemyCharacter.h"
#include "Kismet/KismetMathLibrary.h"

void AUnarmedWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;

	EnemyOwnerCharacter = EnemyOwnerCharacter == nullptr ? Cast<AEnemyCharacter>(OwnerPawn) : EnemyOwnerCharacter;
	
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* UnarmedWeaponSocket = EnemyOwnerCharacter->GetMesh()->GetSocketByName("UnarmedWeapon");
	if (!UnarmedWeaponSocket)
	{
		UE_LOG(LogTemp, Warning, TEXT("需要给Character骨骼添加UnarmedWeapon插槽"));
		return;
	}
	
	FTransform SocketTransform = UnarmedWeaponSocket->GetSocketTransform(EnemyOwnerCharacter->GetMesh());
	FVector Start = SocketTransform.GetLocation();

	FHitResult FireHit;
	WeaponTraceHit(Start, HitTarget, FireHit);

	ACorpsePartyCharacter* CorpsePartyCharacter = Cast<ACorpsePartyCharacter>(FireHit.GetActor());
	if (CorpsePartyCharacter && InstigatorController)
	{
		const float DamageToCause = FireHit.BoneName.ToString() == FString("head") ? HeadShotDamage : Damage;
				
		UGameplayStatics::ApplyDamage(
			CorpsePartyCharacter,
			DamageToCause,
			InstigatorController,
			this,
			UDamageType::StaticClass()
		);
	}
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ImpactParticles,
			FireHit.ImpactPoint,
			FireHit.ImpactNormal.Rotation()
		);
	}
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			HitSound,
			FireHit.ImpactPoint
		);
	}
	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			FireSound,
			GetActorLocation()
		);
	}
}

void AUnarmedWeapon::BeginPlay()
{
	Super::BeginPlay();

}

void AUnarmedWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;

		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(this); // 忽略当前Actor，防止与自身碰撞
		
		World->SweepSingleByChannel(
			OutHit,
			TraceStart,
			End,
			FQuat::Identity,
			ECollisionChannel::ECC_Visibility,
			FCollisionShape::MakeSphere(20.0f), // 用于形状检测的球体半径
			CollisionParams
		);
		FVector BeamEnd = End;
		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}
		else
		{
			OutHit.ImpactPoint = End;
		}
		
		DrawDebugLine(GetWorld(), TraceStart, End, FColor::Green, false, -1);
	}
}

FVector AUnarmedWeapon::TraceEndWithScatter(const FVector& HitTarget)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return FVector();

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	const FVector EndLoc = SphereCenter + RandVec;
	const FVector ToEndLoc = EndLoc - TraceStart;

	DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
	DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, true);
	DrawDebugLine(
		GetWorld(),
		TraceStart,
		FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()),
		FColor::Cyan,
		true);

	return FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
}
