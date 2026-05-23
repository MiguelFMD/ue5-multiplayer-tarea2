// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MPShooterGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class MPSHOOTER_API AMPShooterGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

	public:
	AMPShooterGameModeBase(const FObjectInitializer& ObjectInitializer);
	virtual void SetPlayerDefaults(class APawn* PlayerPawn) override;

	public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Players")
		TArray<FLinearColor> PlayerColors;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Players")
		int32 LastPlayerColorIndex;
	
};
