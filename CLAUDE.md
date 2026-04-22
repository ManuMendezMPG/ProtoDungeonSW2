\# ProtoDungeonSW2 — Contexto para Claude



\## Proyecto

Dungeon crawler isométrico, prototipo de 2 niveles, target Nintendo Switch 2.

Desarrollado en PC (Windows) con Unreal Engine 5.7 compilado desde source.



\## Niveles planificados

\- \*\*Nivel 1 (L\_Combat):\*\* combate con movimiento por stick, ataque básico (botón) y 

&#x20; ataque especial agitando JoyCon (simulado con click de ratón en PC).

\- \*\*Nivel 2 (L\_Puzzle):\*\* puzzle que conmuta modo dock (cámara isométrica) y modo 

&#x20; portátil (cámara cenital + giroscopio que mueve una bola por un laberinto).



\## Engine

\- UE 5.7 source build en C:\\UE57 (junction).

\- NO modificar código del engine salvo que se pida explícitamente.

\- Build config habitual: Development Editor | Win64.



\## Proyecto en disco

\- Ruta: C:\\dev\\ProtoDungeonSW2

\- Nombre del módulo: ProtoDungeonSW2

\- Macro API del módulo: PROTODUNGEONSW2\_API



\## Repositorio

\- GitHub (https://github.com/ManuMendezMPG/ProtoDungeonSW2), main.

\- Git LFS configurado para .uasset, .umap y binarios habituales.

\- Ignorar: Binaries/, Intermediate/, DerivedDataCache/, Saved/, .vs/



\## Convenciones de código

\- Prefijo de clases del proyecto: DP (ej: ADPPlayerCharacter, UDPCombatComponent).

\- Prefijos estándar UE: A para Actors, U para UObjects/Components, F para structs, E para enums.

\- Todos los UPROPERTY editables deben tener Category específica.

\- Usar TObjectPtr<T> en lugar de T\* en miembros UObject.

\- Logs con categorías propias: DECLARE\_LOG\_CATEGORY\_EXTERN + DEFINE\_LOG\_CATEGORY.

\- Comentarios en español, código (identificadores, strings) en inglés.



\## Módulos

\- Módulo principal: ProtoDungeonSW2

\- Dependencias actuales: Core, CoreUObject, Engine, InputCore

\- Dependencias que añadiremos según necesidad: EnhancedInput, PhysicsCore, UMG



\## Input

\- Usamos Enhanced Input (NO el sistema viejo).

\- IMCs separados por contexto (Combat, Puzzle) y uno global (Default).



\## Plataforma

\- Dock/portátil y giroscopio se simulan en PC para desarrollo:

&#x20; - Toggle dock/portátil: tecla (por definir).

&#x20; - Giroscopio: ratón o WASD alternativo.

\- Abstracción vía Subsystems para que el código gameplay no sepa de plataforma.

\- El SDK de Switch 2 está en C:\\UE57\\Engine\\Platforms\\Switch2 (presente pero no activo en desarrollo PC).



\## Criterios de aceptación de código generado

\- Compila sin warnings en Development Editor.

\- Incluye headers correctos y GENERATED\_BODY().

\- UPROPERTY/UFUNCTION con specifiers correctos (EditDefaultsOnly vs EditAnywhere, 

&#x20; BlueprintReadOnly vs BlueprintReadWrite, etc.).

\- Separación clara entre header (.h) y implementación (.cpp).

\- Forward declarations en headers cuando sea posible para minimizar includes.



\## Workflow con IA

\- Para clases nuevas: pedir header + cpp juntos, incluir Build.cs si cambia.

\- Para cambios multi-archivo: usar Claude Code; para dudas conceptuales: chat.

\- Commits frecuentes y pequeños cuando algo compila.


## Includes en UE (regla verificada empíricamente)

Los includes dentro del módulo usan rutas relativas al archivo fuente:

- Archivo en MISMA carpeta: #include "Archivo.h"
- Archivo en carpeta DISTINTA: usar ../ para subir y bajar
  Ejemplo: desde GameModes/Algo.cpp incluir Characters/Otro.h:
  #include "../Characters/Otro.h"

Para módulos externos (engine), los paths son relativos a la raíz pública 
de ese módulo, no requieren ../:
  #include "GameFramework/Character.h"
  #include "Camera/CameraComponent.h"

Nota: Si en el futuro queremos paths "absolutos" desde raíz del módulo sin 
../ relativos, se puede añadir PublicIncludePaths en ProtoDungeonSW2.Build.cs.