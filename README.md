# Orbs-Sweeper
Herramienta de consola para Windows que completa automáticamente las Quests de Discord y te ayuda a conseguir Orbs de manera mas sencilla.

Relanza Discord con el puerto de depuración 9222 abierto, se conecta por WebSocket mediante Chrome DevTools Protocol e inyecta un script que localiza los módulos internos del cliente. Simula el progreso de video, juego, stream y actividad enviando heartbeats a la API y falseando el juego en ejecución.
