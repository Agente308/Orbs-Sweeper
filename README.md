# Orbs-Sweeper
Herramienta de consola para Windows que completa automáticamente las Quests de Discord y te ayuda a conseguir Orbs de manera mas sencilla.

Relanza Discord con el puerto de depuración 9222 abierto, se conecta por WebSocket mediante Chrome DevTools Protocol e inyecta un script que localiza los módulos internos del cliente. Simula el progreso de video, juego, stream y actividad enviando heartbeats a la API y falseando el juego en ejecución.

## Compilación

Requisitos: Windows 10/11 y MSVC (Visual Studio o Build Tools).

Abre **x64 Native Tools Command Prompt for VS** y ejecuta:

    cl /nologo /O2 /W3 /utf-8 /D_CRT_SECURE_NO_WARNINGS orbs_sweeper.c /Fe:orbs_sweeper.exe

<p align="center">
  <img width="511" height="666" alt="ejemplo"
       src="https://github.com/user-attachments/assets/8b2ecb5e-0664-461a-83f3-5e4cc0706ecd" />
</p>
