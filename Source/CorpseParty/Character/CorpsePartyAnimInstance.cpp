// Fill out your copyright notice in the Description page of Project Settings.


#include "CorpsePartyAnimInstance.h"
#include "CorpsePartyCharacter.h"
#include "CorpseParty/Weapon/Weapon.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "CorpseParty/CorpsePartyTypes/CombatState.h"

void UCorpsePartyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	CorpsePartyCharacter = Cast<ACorpsePartyCharacter>(TryGetPawnOwner());
}

void UCorpsePartyAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (CorpsePartyCharacter == nullptr)
	{
		CorpsePartyCharacter = Cast<ACorpsePartyCharacter>(TryGetPawnOwner());
	}
	if (CorpsePartyCharacter == nullptr) return;

	FVector Velocity = CorpsePartyCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = CorpsePartyCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = CorpsePartyCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bWeaponEquipped = CorpsePartyCharacter->IsWeaponEquipped();
	EquippedWeapon = CorpsePartyCharacter->GetEquippedWeapon();
	bIsCrouched = CorpsePartyCharacter->bIsCrouched;
	bAiming = CorpsePartyCharacter->IsAiming();
	TurningInPlace = CorpsePartyCharacter->GetTurningInPlace();
	bRotateRootBone = CorpsePartyCharacter->ShouldRotateRootBone();
	bElimmed = CorpsePartyCharacter->IsElimmed();
	bHoldingTheFlag = CorpsePartyCharacter->IsHoldingTheFlag();

	// Offset Yaw for Strafing
	FRotator AimRotation = CorpsePartyCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(CorpsePartyCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = CorpsePartyCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = CorpsePartyCharacter->GetAO_Yaw();
	AO_Pitch = CorpsePartyCharacter->GetAO_Pitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && CorpsePartyCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		CorpsePartyCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		
		
		if (CorpsePartyCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Hand_R"), ERelativeTransformSpace::RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - CorpsePartyCharacter->GetHitTarget()));
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.f);

			// 枪口的debug射线
			// FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), RTS_World);
			// FVector MuzzleX(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
			// DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MuzzleTipTransform.GetLocation() + MuzzleX * 1000.f, FColor::Red);
			// DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), CorpsePartyCharacter->GetHitTarget(), FColor::Orange);
		}
	}

	bUseFABRIK = CorpsePartyCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;
	bool bFABRIKOverride = CorpsePartyCharacter->IsLocallyControlled() &&
		CorpsePartyCharacter->GetCombatState() != ECombatState::ECS_ThrowingGrenade &&
		CorpsePartyCharacter->bFinishedSwapping;
	if (bFABRIKOverride)
	{
		bUseFABRIK = !CorpsePartyCharacter->IsLocallyReloading();
	}
	bUseAimOffsets = CorpsePartyCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !CorpsePartyCharacter->GetDisableGameplay();
	bTransformRightHand = CorpsePartyCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !CorpsePartyCharacter->GetDisableGameplay();
}
