// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Engine/CollisionProfile.h"
#include "DamageType_WeaponFire.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/DamageEvents.h"

// Sets default values
AWeapon::AWeapon(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(
		TEXT("/Script/Engine.StaticMesh'/Engine/EngineMeshes/Cube.Cube'")
	);

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(
		TEXT("/Script/Engine.Material'/Game/Materials/M_Weapon.M_Weapon'")
	);
	

	bReplicates = true;
	bNetUseOwnerRelevancy = true;

	RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));
	MeshComponent = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("MeshComponent"));

	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetStaticMesh(MeshFinder.Object);
	MeshComponent->SetMaterial(0, MaterialFinder.Object);
	MeshComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	MeshComponent->SetRelativeLocation(FVector(20.0f,-15.0f,0.0f));
	MeshComponent->SetRelativeRotation(FRotator(0.0f,0.0f,45.0f));
	MeshComponent->SetRelativeScale3D(FVector(0.5f,0.05f,0.05f));

	AimInterpSpeed = 8.0f;
	DropInterpSpeed = 10.0f;
	DropRotation = FRotator(-30.0f, -80.0f, 0.0f);

	MuzzleHandle = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("MuzzleHandle"));
	MuzzleHandle->SetupAttachment(RootComponent);
	MuzzleHandle->SetRelativeLocation(FVector(50.0f, 0.0f, 0.0f));

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> FireEffectFinder(TEXT("/Script/Niagara.NiagaraSystem'/Game/FX/SYS_Fire.SYS_Fire'"));
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> ImpactEffectFinder(TEXT("/Script/Niagara.NiagaraSystem'/Game/FX/SYS_Impact.SYS_Impact'"));

	FireCooldown = 0.4f;
	LastFireTime = TNumericLimits<float>::Lowest();
	FireEffect = FireEffectFinder.Object;
	ImpactEffect = ImpactEffectFinder.Object;
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bAimLocationIsValid) {

		const FVector AimDisplacement = AimLocation - GetActorLocation();
		const FVector AimDirection = AimDisplacement.GetSafeNormal();

		const FQuat TargetRotation = AimDirection.ToOrientationQuat();
		const FQuat NewRotation = FMath::QInterpTo(GetActorQuat(), TargetRotation, DeltaTime, AimInterpSpeed);
		SetActorRotation(NewRotation);
	}

	else
	{
		AActor* AttachParent = GetAttachParentActor();
		const FQuat TargetRotation = AttachParent ? AttachParent->GetActorTransform().TransformRotation(FQuat(DropRotation)) : FQuat(DropRotation);
		const FQuat NewRotation = FMath::QInterpTo(GetActorQuat(), TargetRotation, DeltaTime, DropInterpSpeed);
		SetActorRotation(NewRotation);
	}

}

void AWeapon::UpdateAimLocation(const FVector& InWorldAimLocation, const FVector& InViewAimLocation)
{
	AimLocation = InWorldAimLocation;
	bAimLocationIsValid = InViewAimLocation.X > MuzzleHandle->GetRelativeLocation().X;


}

void AWeapon::HandleFireInput()
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	const float ElapsedSinceLastFire = CurrentTime - LastFireTime;
	if(ElapsedSinceLastFire>=FireCooldown){
	
		const FVector MuzzleLocation = MuzzleHandle->GetComponentLocation();
		const FVector Direction = MuzzleHandle->GetComponentQuat().Vector();
		Server_TryFire(MuzzleLocation, Direction);
		LastFireTime = CurrentTime;

		if (!HasAuthority()) {

			PlayFireEffects();
			FHitResult Hit;
			if (RunFireTrace(Hit)) {

				const bool bWillProbablyCauseDamage = Hit.GetActor()->IsValidLowLevel() && Hit.GetActor()->CanBeDamaged();
				PlayImpactEffects(Hit.ImpactPoint, Hit.ImpactNormal, bWillProbablyCauseDamage);
			}
		}

	}
}

void AWeapon::Server_TryFire_Implementation(const FVector& MuzzleLocation, const FVector& Direction)
{

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	const float ElapsedSinceLastFire = CurrentTime - LastFireTime;

	if (ElapsedSinceLastFire >= FireCooldown)
	{
		LastFireTime = CurrentTime;
		LastFirePacket.ServerFireTime = CurrentTime;
		FHitResult Hit;
		if (RunFireTrace(Hit))
		{
			float DamageCaused = 0.0f;
			if (Hit.GetActor()->IsValidLowLevel() && Hit.GetActor()->CanBeDamaged())
			{
				const float BaseDamage = 1.0f;
				const FPointDamageEvent DamageEvent(BaseDamage, Hit, Direction, UDamageType_WeaponFire::StaticClass());
				DamageCaused = Hit.GetActor()->TakeDamage(BaseDamage, DamageEvent, GetInstigatorController(), this);

			}

			PlayFireEffects();
			PlayImpactEffects(Hit.ImpactPoint, Hit.ImpactNormal, DamageCaused > 0.0f);

			LastFirePacket.bCausedDamage = DamageCaused > 0.0f;
			LastFirePacket.ImpactPoint = Hit.ImpactPoint;
			LastFirePacket.ImpactNormal = Hit.ImpactNormal;
			
		}
		else
		{
			LastFirePacket.ImpactNormal = FVector::ZeroVector;
		}
	}

}

bool AWeapon::RunFireTrace(FHitResult& OutHit)
{
	const FVector& TraceStart = MuzzleHandle->GetComponentLocation();
	const FVector TraceEnd = TraceStart + (MuzzleHandle->GetForwardVector() * 5000.0f);
	const FName ProfileName = UCollisionProfile::BlockAllDynamic_ProfileName;
	const FCollisionQueryParams QueryParams(TEXT("WeaponFire"), false, GetOwner());
	return GetWorld()->LineTraceSingleByProfile(OutHit, TraceStart, TraceEnd, ProfileName, QueryParams);
}

void AWeapon::PlayFireEffects()
{
	if (FireEffect)
	{
		
		UNiagaraFunctionLibrary::SpawnSystemAttached(FireEffect, MuzzleHandle, NAME_None, FVector(0.f), FRotator(0.f), EAttachLocation::Type::KeepRelativeOffset, true);
	}
}

void AWeapon::PlayImpactEffects(const FVector& ImpactPoint, const FVector& ImpactNormal, bool bCausedDamage)
{
	const FRotator ImpactRotation = ImpactNormal.ToOrientationRotator();
	if (ImpactEffect)
	{
		FVector scale(1.0, 1.0, 1.0);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactEffect, ImpactPoint, ImpactRotation,scale,false,true,ENCPoolMethod::None,true);
	}
}

void AWeapon::OnRep_LastFirePacket()
{
	PlayFireEffects();

	if (!LastFirePacket.ImpactNormal.IsZero())
	{
		PlayImpactEffects(LastFirePacket.ImpactPoint, LastFirePacket.ImpactNormal, LastFirePacket.bCausedDamage);
	}
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{

	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AWeapon, LastFirePacket, COND_SkipOwner);
}

