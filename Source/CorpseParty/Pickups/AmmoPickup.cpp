// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickup.h"
#include "CorpseParty/Character/CorpsePartyCharacter.h"
#include "CorpseParty/CorpsePartyComponents/CombatComponent.h"

void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ACorpsePartyCharacter* CorpsePartyCharacter = Cast<ACorpsePartyCharacter>(OtherActor);
	if (CorpsePartyCharacter)
	{
		UCombatComponent* Combat = CorpsePartyCharacter->GetCombat();
		if (Combat)
		{
			Combat->PickupAmmo(WeaponType, AmmoAmount);
		}
	}
	Destroy();
}

