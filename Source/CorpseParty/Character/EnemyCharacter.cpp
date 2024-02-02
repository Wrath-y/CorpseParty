// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"

#include "CorpsePartyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CorpseParty/CorpseParty.h"
#include "CorpseParty/CorpsePartyComponents/CombatComponent.h"
#include "CorpseParty/PlayerController/CorpsePartyPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "CorpseParty/GameMode/CorpsePartyGameMode.h"
#include "TimerManager.h"
#include "Components/BoxComponent.h"
#include "CorpseParty/CorpsePartyComponents/LagCompensationComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "AI/NavigationSystemBase.h"
#include "CorpseParty/CorpsePartyComponents/EnemyBuffComponent.h"
#include "CorpseParty/CorpsePartyComponents/EnemyCombatComponent.h"
#include "GameFramework/NavMovementComponent.h"

class ACorpsePartyGameMode;

AEnemyCharacter::AEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	// GetCharacterMovement()->bUseAccelerationForPaths = true;
	// GetCharacterMovement()->bUseFixedBrakingDistanceForPaths = false;

	Combat = CreateDefaultSubobject<UEnemyCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	Buff = CreateDefaultSubobject<UEnemyBuffComponent>(TEXT("BuffComponent"));
	Buff->SetIsReplicated(true);

	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensation"));
	
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	// InYaw must not be zero, eitherwise Character can not rotation to movement direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 850.f);

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	
	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));
}

void AEnemyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEnemyCharacter, Shield);
	DOREPLIFETIME(AEnemyCharacter, Health);
	DOREPLIFETIME(AEnemyCharacter, bDisableGameplay);
}

void AEnemyCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	TimeSinceLastMovementReplication = 0.f;
}

void AEnemyCharacter::Elim(bool bPlayerLeftGame)
{
	MulticastElim(bPlayerLeftGame);
}

void AEnemyCharacter::MulticastElim_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;
	if (CorpsePartyPlayerController)
	{
		CorpsePartyPlayerController->SetHUDWeaponAmmo(0);
	}
	bElimmed = true;
	PlayElimMontage();

	// Start dissolve effect
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	StartDissolve();
	
	// Disable character movement
	bDisableGameplay = true;
	GetCharacterMovement()->DisableMovement();
	if (Combat)
	{
		Combat->Fire();
	}
	
	// Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Spawn elim bot
	if (ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ElimBotEffect,
			ElimBotSpawnPoint,
			GetActorRotation()
		);
	}
	if (ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(
			this,
			ElimBotSound,
			GetActorLocation()
		);
	}
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&AEnemyCharacter::ElimTimerFinished,
		ElimDelay
	);
}

void AEnemyCharacter::ElimTimerFinished()
{
}

void AEnemyCharacter::Destroyed()
{
	Super::Destroyed();

	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	UpdateHUDHealth();
	UpdateHUDShield();
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AEnemyCharacter::ReceiveDamage);
	}
}

void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEnemyCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Combat)
	{
		Combat->Character = this;
	}
	if (Buff)
	{
		Buff->Character = this;
		Buff->SetInitialSpeeds(
			GetCharacterMovement()->MaxWalkSpeed, 
			GetCharacterMovement()->MaxWalkSpeedCrouched
		);
		Buff->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}
	if (LagCompensation)
	{
		LagCompensation->EnemyCharacter = this;
		if (Controller)
		{
			LagCompensation->Controller = Cast<ACorpsePartyPlayerController>(Controller);
		}
	}
}

void AEnemyCharacter::PlayFireMontage()
{
	if (Combat == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AEnemyCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void AEnemyCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AEnemyCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatorController, AActor* DamageCauser)
{
	UE_LOG(LogTemp, Warning, TEXT("ReceiveDamage"));
	CorpsePartyGameMode = CorpsePartyGameMode == nullptr ? GetWorld()->GetAuthGameMode<ACorpsePartyGameMode>() : CorpsePartyGameMode;
	if (bElimmed || CorpsePartyGameMode == nullptr) return;
	Damage = CorpsePartyGameMode->CalculateDamage(InstigatorController, Controller, Damage);
	
	float DamageToHealth = Damage;
	if (Shield > 0.f)
	{
		if (Shield >= Damage)
		{
			Shield = FMath::Clamp(Shield - Damage, 0.f, MaxShield);
			DamageToHealth = 0.f;
		}
		else
		{
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0.f, Damage);
			Shield = 0.f;
		}
	}

	Health = FMath::Clamp(Health - DamageToHealth, 0.f, MaxHealth);

	UpdateHUDHealth();
	UpdateHUDShield();
	PlayHitReactMontage();

	if (Health == 0.f)
	{
		if (CorpsePartyGameMode)
		{
			CorpsePartyPlayerController = CorpsePartyPlayerController == nullptr ? Cast<ACorpsePartyPlayerController>(Controller) : CorpsePartyPlayerController;
			ACorpsePartyPlayerController* AttackerController = Cast<ACorpsePartyPlayerController>(InstigatorController);
			CorpsePartyGameMode->EnemyEliminated(this, CorpsePartyPlayerController, AttackerController);
		}
	}
}

float AEnemyCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void AEnemyCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	if (Health < LastHealth)
	{
		PlayHitReactMontage();
	}
}

void AEnemyCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	if (Shield < LastShield)
	{
		PlayHitReactMontage();
	}
}

void AEnemyCharacter::UpdateHUDHealth()
{
	CorpsePartyPlayerController = CorpsePartyPlayerController == nullptr ? Cast<ACorpsePartyPlayerController>(Controller) : CorpsePartyPlayerController;
	if (CorpsePartyPlayerController)
	{
		CorpsePartyPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void AEnemyCharacter::UpdateHUDShield()
{
	CorpsePartyPlayerController = CorpsePartyPlayerController == nullptr ? Cast<ACorpsePartyPlayerController>(Controller) : CorpsePartyPlayerController;
	if (CorpsePartyPlayerController)
	{
		CorpsePartyPlayerController->SetHUDShield(Shield, MaxShield);
	}
}

void AEnemyCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void AEnemyCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &AEnemyCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

FVector AEnemyCharacter::GetHitTarget() const
{
	if (Combat == nullptr) return FVector();
	return Combat->HitTarget;
}

ECombatState AEnemyCharacter::GetCombatState() const
{
	if (Combat == nullptr) return ECombatState::ECS_MAX;
	return Combat->CombatState;
}
