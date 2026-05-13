#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DPGyroInputSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTiltChangedSignature, FVector2D, NewTilt);

UCLASS()
class PROTODUNGEONSW2_API UDPGyroInputSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Inclinación actual en X (pitch) e Y (roll), valores entre -1 y 1.
	// X positivo = inclinado hacia delante (Norte en vista cenital).
	// Y positivo = inclinado hacia la derecha.
	UPROPERTY(BlueprintReadOnly, Category = "Gyro")
	FVector2D CurrentTilt;

	// Cuánto recoger del delta de input cada vez que llega (sensibilidad).
	UPROPERTY(BlueprintReadWrite, Category = "Gyro")
	float TiltSensitivity = 0.05f;

	// Cuánto se "vuelve al centro" la inclinación pasivamente cuando no hay input (por segundo).
	// 0 = no vuelve. 1 = vuelve al centro completo en 1 segundo.
	UPROPERTY(BlueprintReadWrite, Category = "Gyro")
	float CenteringSpeed = 2.0f;

	// Límite máximo absoluto de inclinación en cada eje.
	UPROPERTY(BlueprintReadWrite, Category = "Gyro")
	float MaxTilt = 1.0f;

	// Delegate cuando cambia el tilt.
	UPROPERTY(BlueprintAssignable, Category = "Gyro")
	FOnTiltChangedSignature OnTiltChanged;

	// Punto de entrada de input desde el caller (BP en PC, plugin de JoyCon en Switch).
	// Delta normalizado: el caller debe pasar el delta ya escalado (ej: delta del ratón).
	UFUNCTION(BlueprintCallable, Category = "Gyro")
	void FeedTiltDelta(FVector2D Delta);

	// Resetea el tilt al centro manualmente.
	UFUNCTION(BlueprintCallable, Category = "Gyro")
	void ResetTilt();

	// Para activar/desactivar el procesamiento (cuando estamos fuera del modo puzzle).
	UFUNCTION(BlueprintCallable, Category = "Gyro")
	void SetGyroActive(bool bActive);

	UPROPERTY(BlueprintReadOnly, Category = "Gyro")
	bool bGyroActive;

	// Lifecycle.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

protected:
	// Tick de centering: los subsystems no tienen tick nativo, usamos FTSTicker.
	void TickCentering(float DeltaTime);

	FTSTicker::FDelegateHandle TickerHandle;
};
