// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSLaunchPad.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AFPSLaunchPad::AFPSLaunchPad()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	LaunchArea = CreateDefaultSubobject<UBoxComponent>(TEXT("LaunchBox"));
	LaunchArea->SetBoxExtent(FVector(80.0f, 80.0f, 30.0f));
	RootComponent = LaunchArea;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);

	LaunchArea->OnComponentBeginOverlap.AddDynamic(this, &AFPSLaunchPad::LaunchActor);

	LaunchForce = 1000.0f;
	LaunchPitchAngle = 25.0f;
}


void AFPSLaunchPad::LaunchActor(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//if (OtherActor == nullptr) return;

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("uwu"));
	
	FRotator LaunchDirection = GetActorRotation();
	LaunchDirection.Pitch += LaunchPitchAngle;
	FVector LaunchVelocity = LaunchDirection.Vector() * LaunchForce;

	ACharacter* otherCharacter = Cast<ACharacter>(OtherActor);
	if (otherCharacter)
	{
		otherCharacter->LaunchCharacter(LaunchVelocity, true, true);
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ActivateLaunchPadVFX, GetActorLocation());
	}
	else if (OtherComp && OtherComp->IsSimulatingPhysics())
	{
		OtherComp->AddImpulse(LaunchVelocity, NAME_None, true);
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ActivateLaunchPadVFX, GetActorLocation());
	}
}

