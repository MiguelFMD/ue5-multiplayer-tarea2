// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPawn.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputAction.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Net/UnrealNetwork.h"
#include "Engine/CollisionProfile.h"

// Sets default values
AShooterPawn::AShooterPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UInputAction>
	MoveForwardAsset(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/MoveForward.MoveForward'"));
	inputMoveForward=MoveForwardAsset.Object;
	static ConstructorHelpers::FObjectFinder<UInputAction>
		MoveRightAsset(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/MoveRight.MoveRight'"));
	inputMoveRight=MoveRightAsset.Object;
	static ConstructorHelpers::FObjectFinder<UInputAction>
		MoveUpAsset(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/MoveUp.MoveUp'"));
	inputMoveUp=MoveUpAsset.Object;
	static ConstructorHelpers::FObjectFinder<UInputAction>
		LookAsset(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Look.Look'"));
	inputLook=LookAsset.Object;
	static ConstructorHelpers::FObjectFinder<UInputAction>
		FireAsset(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Fire.Fire'"));
	inputFire=FireAsset.Object;
	
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(
		TEXT("/Script/Engine.SkeletalMesh'/Engine/EngineMeshes/SkeletalCube.SkeletalCube'")
	);

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(
		TEXT("/Script/Engine.Material'/Game/Materials/M_Pawn.M_Pawn'")
	);
	

	BaseEyeHeight = 18.0f;
	bUseControllerRotationPitch = true;

	UCapsuleComponent* CollisionComponent = GetCapsuleComponent();

	if (CollisionComponent) {
		CollisionComponent->SetCapsuleSize(32.0f, 40.0f, false);
	}

	USkeletalMeshComponent* MeshComponent = GetMesh();
	
	if (MeshComponent) {
		MeshComponent->SetSkeletalMesh(MeshFinder.Object);
		MeshComponent->SetMaterial(0, MaterialFinder.Object);
		MeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, -22.0f));
		MeshComponent->SetRelativeScale3D(FVector(2.0f));

	}
	

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();

	if (MovementComponent) {
		MovementComponent->DefaultLandMovementMode = MOVE_Flying;
		MovementComponent->MaxAcceleration = 5000.0f;
		MovementComponent->MaxFlySpeed = 800.0f;
		MovementComponent->BrakingDecelerationFlying = 5000.0f;
	}
}

// Called when the game starts or when spawned
void AShooterPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AShooterPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AShooterPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	//Super::SetupPlayerInputComponent(PlayerInputComponent);
	UEnhancedInputComponent* enhancedInputComponent=Cast<UEnhancedInputComponent>(PlayerInputComponent);

	// this is a pointer to this UClass
	enhancedInputComponent->BindAction(inputMoveForward,
		ETriggerEvent::Triggered,
		this,
		&AShooterPawn::OnMoveForward);

	enhancedInputComponent->BindAction(inputMoveRight,
		ETriggerEvent::Triggered,
		this,
		&AShooterPawn::OnMoveRight);

	enhancedInputComponent->BindAction(inputMoveUp,
		ETriggerEvent::Triggered,
		this,
		&AShooterPawn::OnMoveUp);

	enhancedInputComponent->BindAction(inputLook,
		ETriggerEvent::Triggered,
		this,
		&AShooterPawn::OnLook);

	enhancedInputComponent->BindAction(inputFire,
		ETriggerEvent::Triggered,
		this,
		&AShooterPawn::OnFire);



}
void AShooterPawn::OnMoveForward(const FInputActionValue& inputValue) {
	const float AxisValue = inputValue.Get<float>();
	if (AxisValue != 0.0f) {
		const FRotator ControlRotation = Controller ? Controller->GetControlRotation() : FRotator::ZeroRotator;
		const FVector ViewForward = FRotationMatrix(ControlRotation).GetScaledAxis(EAxis::X);
		AddMovementInput(ViewForward, AxisValue);
	}
}

void AShooterPawn::OnMoveRight( const FInputActionValue& inputValue) {
	const float AxisValue = inputValue.Get<float>();

	if (AxisValue != 0.0f) {
		const FRotator ControlRotation = Controller ? Controller->GetControlRotation() : FRotator::ZeroRotator;
		const FVector ViewRight = FRotationMatrix(ControlRotation).GetScaledAxis(EAxis::Y);
		AddMovementInput(ViewRight, AxisValue);
	}
}

void AShooterPawn::OnMoveUp(const FInputActionValue& inputValue) {
	const float AxisValue = inputValue.Get<float>();

	if (AxisValue != 0.0f) {
		AddMovementInput(FVector::UpVector, AxisValue);
	}
}

void AShooterPawn::OnLook(const FInputActionValue& inputValue) {
	const FVector2D lookVector = inputValue.Get<FVector2D>();
	AddControllerYawInput(lookVector.X);
	AddControllerPitchInput(lookVector.Y);

}

void AShooterPawn::OnFire(const FInputActionValue& inputValue) {
}
