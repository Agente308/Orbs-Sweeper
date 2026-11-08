# Orbs-Sweeper
> [!WARNING]
> No me hago responsable por suspensiones de cuenta o baneos. Modificar el cliente de Discord o falsear el progreso de las Quests va en contra de los Términos de Servicio de Discord y puede conllevar sanciones a tu cuenta. Úsalo bajo tu propio riesgo.

Herramienta de consola para Windows que completa automáticamente las Quests de Discord y te ayuda a conseguir Orbs de manera mas sencilla.

Relanza Discord con el puerto de depuración 9222 abierto, se conecta por WebSocket mediante Chrome DevTools Protocol e inyecta un script que localiza los módulos internos del cliente. Simula el progreso de video, juego, stream y actividad enviando heartbeats a la API y falseando el juego en ejecución.

## Uso

1. Abre Discord (app de escritorio) y **acepta una misión** puede ser cualquiera a excepcion de las que piden autenticacion o conectar una cuenta.
2. Ejecuta `orbs_sweeper.exe`.
3. Elige la opción **`[01] Iniciar`**.
4. Discord se **reiniciará solo** con el puerto de depuración abierto. Espera a que cargue.
5. Vuelve a la pestaña de **Misiones** y listo: el programa completará las misiones pendientes automáticamente.

> [!NOTE]
> - Solo se procesan las misiones que ya aceptaste, así que acepta todas las que quieras completar antes de iniciar.
> - El programa solo es compatible con la version de escritorio de Discord.
> - Deja la consola abierta para ver el progreso. Si algo falla, usa **`[02] Iniciar en debug`** para ver mensajes más detallados y enviame el log por discord (agente_308) si no puedes corregirlo tu mismo.
> - Desde **`[03] Configuración`** puedes activar o desactivar la notificación de Windows al completar una misión.

## Compilación

Requisitos: Windows 10/11 y MSVC (Visual Studio o Build Tools).

Abre **x64 Native Tools Command Prompt for VS** y ejecuta:

    cl /nologo /O2 /W3 /utf-8 /D_CRT_SECURE_NO_WARNINGS orbs_sweeper.c /Fe:orbs_sweeper.exe

<p align="center">
  <img width="511" height="666" alt="ejemplo"
       src="https://github.com/user-attachments/assets/8b2ecb5e-0664-461a-83f3-5e4cc0706ecd" />
</p>
