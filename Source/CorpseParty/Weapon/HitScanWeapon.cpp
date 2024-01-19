// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "CorpseParty/Character/CorpsePartyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "WeaponTypes.h"
#include "DrawDebugHelpers.h"
#include "CorpseParty/CorpsePartyComponents/LagCompensationComponent.h"
#include "CorpseParty/PlayerController/CorpsePartyPlayerController.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);

		ACorpsePartyCharacter* CorpsePartyCharacter = Cast<ACorpsePartyCharacter>(FireHit.GetActor());
		if (CorpsePartyCharacter && InstigatorController)
		{
			bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
			if (HasAuthority() && bCauseAuthDamage)
			{
				UE_LOG(LogTemp, Warning,  TEXT("%s"), *FireHit.BoneName.ToString());
				const float DamageToCause = FireHit.BoneName.ToString() == FString("head") ? HeadShotDamage : Damage;
				
				UGameplayStatics::ApplyDamage(
					CorpsePartyCharacter,
					DamageToCause,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
			if (!HasAuthority() && bUseServerSideRewind)
			{
				CorpsePartyOwnerCharacter = CorpsePartyOwnerCharacter == nullptr ? Cast<ACorpsePartyCharacter>(OwnerPawn) : CorpsePartyOwnerCharacter;
				CorpsePartyOwnerController = CorpsePartyOwnerController == nullptr ? Cast<ACorpsePartyPlayerController>(InstigatorController) : CorpsePartyOwnerController;
				if (CorpsePartyOwnerController && CorpsePartyOwnerCharacter && CorpsePartyOwnerCharacter->GetLagCompensation() && CorpsePartyOwnerCharacter->IsLocallyControlled())
				{
					CorpsePartyOwnerCharacter->GetLagCompensation()->ServerScoreRequest(
						CorpsePartyCharacter,
						Start,
						HitTarget,
						CorpsePartyOwnerController->GetServerTime() - CorpsePartyOwnerController->SingleTripTime
					);
				}
			}
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
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				MuzzleFlash,
				SocketTransform
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
}


void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;

		World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			End,
			ECollisionChannel::ECC_Visibility
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
		
		DrawDebugSphere(GetWorld(), BeamEnd, 16.f, 12, FColor::Orange, true);
		
		if (BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				World,
				BeamParticles,
				TraceStart,
				FRotator::ZeroRotator,
				true
			);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}
