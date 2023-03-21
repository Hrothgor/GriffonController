// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "NiagaraSystem.h"
#include "ShapeShiftMenu.h"
#include "DruidControllerCharacter.generated.h"

UCLASS()
class GRIFFONCONTROLLER_API ADruidControllerCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	/** ShapeShift Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ShapeShiftAction;

public:
	// Sets default values for this character's properties
	ADruidControllerCharacter();

protected:
	
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
	
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	// Called every frame
	virtual void Tick(float DeltaTime) override;//

	bool bCanMove = true;
	
	///////////////////////////////
	/// SHAPESHIFT

	void StartShapeShifting();
	void ShapeShift(EShapeShiftForm form);

	bool bChargeShapeShift = false;
	UFUNCTION(BlueprintPure)
	bool IsChargingShapeShift() const;
	UFUNCTION(BlueprintCallable)
	void EndShapeShiftCastNotify();

	UPROPERTY(EditAnywhere)
    UNiagaraSystem *NS_ShapeShiftCharging;
	UPROPERTY()
	UNiagaraComponent *NS_ShapeShiftChargingInstance;

	UPROPERTY(EditAnywhere)
	UNiagaraSystem *NS_ShapeShiftCast;

	UPROPERTY()
	UAnimInstance* AnimInstance;
	UPROPERTY(EditAnywhere)
	UAnimMontage* CastShapeShiftMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UUserWidget> W_ShapeShiftMenu;
	UPROPERTY()
	UShapeShiftMenu *ShapeShiftMenuInstance;
};
