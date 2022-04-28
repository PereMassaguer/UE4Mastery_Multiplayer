// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSAIGuard.h"
#include "Perception/PawnSensingComponent.h"
#include "DrawDebugHelpers.h"
#include "FPSGameMode.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AFPSAIGuard::AFPSAIGuard()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));

	PawnSensingComp->OnSeePawn.AddDynamic(this, &AFPSAIGuard::OnPawnSeen);
	PawnSensingComp->OnHearNoise.AddDynamic(this, &AFPSAIGuard::OnNoiseHeard);

	GuardState = EAIState::Idle;

	CurrentPathIndex = -1;
}

// Called when the game starts or when spawned
void AFPSAIGuard::BeginPlay()
{
	Super::BeginPlay();
	
	OriginalRotation = GetActorRotation();

	MoveToNextPathPoint();
}

void AFPSAIGuard::OnPawnSeen(APawn* Pawn)
{
	if (Pawn == nullptr) return;

	AFPSGameMode* gameMode = Cast<AFPSGameMode>(GetWorld()->GetAuthGameMode());
	if (gameMode)
	{
		gameMode->CompleteMission(Pawn, false);
	}

	SetGuardState(EAIState::Alerted);

	GetController()->StopMovement();
}

void AFPSAIGuard::OnNoiseHeard(APawn* NoiseInstigator, const FVector& Location, float Volume)
{
	if (GuardState == EAIState::Alerted) return;
	if (NoiseInstigator == nullptr) return;

	FVector SourceDirection = Location - GetActorLocation();
	SourceDirection.Z = 0;

	TargetRotation = FRotationMatrix::MakeFromX(SourceDirection).Rotator();

	GetWorldTimerManager().SetTimer(Handle_ResetRotation, this, &AFPSAIGuard::ResetRotation, 3.0f);

	SetGuardState(EAIState::Suspicious);

	GetController()->StopMovement();
}

void AFPSAIGuard::ResetRotation()
{
	if (GuardState == EAIState::Alerted) return;

	TargetRotation = OriginalRotation;

	MoveToActor(GetCurrentPathPoint());

	SetGuardState(EAIState::Idle);
}

void AFPSAIGuard::OnRep_GuardState()
{
	OnStateChanged(GuardState);
}

void AFPSAIGuard::SetGuardState(EAIState NewState)
{
	if (GuardState == NewState) return;

	GuardState = NewState;

	OnRep_GuardState();
}

void AFPSAIGuard::MoveToActor(AActor* TargetActor)
{
	if (TargetActor == nullptr) return;

	UAIBlueprintHelperLibrary::SimpleMoveToActor(GetController(), TargetActor);
}

void AFPSAIGuard::MoveToNextPathPoint()
{
	if (PathPoints.Num() == 0) return;

	CurrentPathIndex = (CurrentPathIndex + 1) % PathPoints.Num();
	
	MoveToActor(PathPoints[CurrentPathIndex]);
}

AActor* AFPSAIGuard::GetCurrentPathPoint()
{
	if (PathPoints.Num() == 0) return nullptr;

	return PathPoints[CurrentPathIndex];
}

// Called every frame
void AFPSAIGuard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, RotationSpeed));

	if (CurrentPathIndex < 0) return;

	float targetDistance = (GetActorLocation() - PathPoints[CurrentPathIndex]->GetActorLocation()).Size();
	
	if (targetDistance < 100.0f)
	{
		MoveToNextPathPoint();
	}
}

void AFPSAIGuard::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFPSAIGuard, GuardState);
}