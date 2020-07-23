// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SGrenade.h"
#include "SFrag.generated.h"

/**
 * 
 */
UCLASS()
class SIMPLESHOOTER_API ASFrag : public ASGrenade
{
	GENERATED_BODY()

public:
	ASFrag();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	virtual void Explode() override;
	
};
