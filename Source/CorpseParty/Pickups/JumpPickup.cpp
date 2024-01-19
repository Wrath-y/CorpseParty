// Fill out your copyright notice in the Description page of Project Settings.


#include "JumpPickup.h"
#include "CorpseParty/Character/CorpsePartyCharacter.h"
#include "CorpseParty/CorpsePartyComponents/BuffComponent.h"

void AJumpPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ACorpsePartyCharacter* CorpsePartyCharacter = Cast<ACorpsePartyCharacter>(OtherActor);
	if (CorpsePartyCharacter)
	{
		UBuffComponent* Buff = CorpsePartyCharacter->GetBuff();
		if (Buff)
		{
			Buff->BuffJump(JumpZVelocityBuff, JumpBuffTime);
		}
	}

	Destroy();
}
