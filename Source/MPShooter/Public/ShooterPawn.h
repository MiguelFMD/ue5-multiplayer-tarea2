// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterPawn.generated.h"

struct FInputActionValue;

UCLASS()
class MPSHOOTER_API AShooterPawn : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterPawn(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
		class UMaterialInstanceDynamic* MeshMID;


public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Input")
	class UInputAction* inputMoveForward;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Input")
	class UInputAction* inputMoveRight;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Input")
	class UInputAction* inputMoveUp;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Input")
	class UInputAction* inputLook;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Input")
	class UInputAction* inputFire;		

private:
	void OnMoveForward(const FInputActionValue& value);
	void OnMoveRight(const FInputActionValue& value);
	void OnMoveUp(const FInputActionValue& value);
	void OnLook(const FInputActionValue& value);
	void OnFire(const FInputActionValue& value);	

};
