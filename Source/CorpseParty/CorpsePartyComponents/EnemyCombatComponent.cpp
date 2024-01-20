// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCombatComponent.h"

#include "CorpseParty/Character/CorpsePartyCharacter.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#include "CorpseParty/Character/EnemyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UEnemyCombatComponent::UEnemyCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.f;
}

void UEnemyCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UEnemyCombatComponent, CombatState);
}

void UEnemyCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	}
}

void UEnemyCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Character && Character->IsLocallyControlled())
	{
		// TODO Select a player as HitTarget
		// HitTarget = ;
	}
}

void UEnemyCombatComponent::Fire()
{
	if (CanFire())
	{
		bCanFire = false;
		FireProjectileWeapon();
		StartFireTimer();
	}
}

void UEnemyCombatComponent::FireProjectileWeapon()
{
	if (Character)
	{
		// TODO Select a player as HitTarget
		// HitTarget = ;
		if (!Character->HasAuthority()) LocalFire(HitTarget);
		ServerFire(HitTarget, DefaultFireDelay);
	}
}

void UEnemyCombatComponent::StartFireTimer()
{
	if (Character == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UEnemyCombatComponent::FireTimerFinished,
		DefaultFireDelay
	);
}

void UEnemyCombatComponent::FireTimerFinished()
{
	bCanFire = true;
}

void UEnemyCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget, float FireDela)
{
	MulticastFire(TraceHitTarget);
}

bool UEnemyCombatComponent::ServerFire_Validate(const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
	return FMath::IsNearlyEqual(FireDelay, FireDelay, 0.001f);
}

void UEnemyCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	LocalFire(TraceHitTarget);
}

void UEnemyCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if (Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage();
	}
}

void UEnemyCombatComponent::Reload()
{
	if (CombatState == ECombatState::ECS_Unoccupied && !bLocallyReloading)
	{
		ServerReload();
		HandleReload();
		bLocallyReloading = true;
	}
}

void UEnemyCombatComponent::ServerReload_Implementation()
{
	if (Character == nullptr) return;
	CombatState = ECombatState::ECS_Reloading;
	if (!Character->IsLocallyControlled()) HandleReload();
}

void UEnemyCombatComponent::FinishReloading()
{
	if (Character == nullptr) return;
	bLocallyReloading = false;
	if (Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
	}
	if (CanFire())
	{
		Fire();
	}
}

void UEnemyCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		if (Character && !Character->IsLocallyControlled()) HandleReload();
		break;
	case ECombatState::ECS_Unoccupied:
		if (CanFire())
		{
			Fire();
		}
		break;
	}
}

void UEnemyCombatComponent::HandleReload()
{
	if (Character)
	{
		// Character->PlayReloadMontage();
	}
}

bool UEnemyCombatComponent::CanFire()
{
	return bCanFire && CombatState == ECombatState::ECS_Unoccupied;
}
