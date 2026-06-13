# Manuscrito de Huarochirí — Las Crónicas Antiguas

Videojuego educativo de aventura/acción inspirado en el **Manuscrito de Huarochirí**
(c. 1608), la fuente más importante de la mitología andina. Proyecto del curso,
enfocado en Humanidades Digitales y programación orientada a objetos.

> Año 2026. La corporación **Oblivion Corp** lanzó un virus para borrar de internet
> las lenguas originarias y los mitos antiguos. Eres un **Ciber-Guardián de la
> Memoria**: entra a la simulación de los mitos y rescata el manuscrito antes de
> que el contador llegue a cero.

## Cómo compilar

1. Abrir `ManuscritoHuarochiri.sln` con **Visual Studio 2022** (con el workload
   *Desarrollo de escritorio con C++* y soporte **C++/CLI**).
2. Seleccionar configuración **Debug | x64** (o Release).
3. Compilar (F5). Un evento post-build copia las carpetas de assets
   (`imgMenu`, `imgPersonajes`, `imgNiveles`, `msc`) junto al `.exe`, así el juego
   también corre con doble clic desde `x64/Debug`.

## Controles

| Tecla | Acción |
|---|---|
| WASD o Flechas | Moverse |
| E (mantener) | Interactuar / cargar pilares |
| K o Espacio | Ray-Trace (se desbloquea al entregar la ofrenda) |
| Enter | Confirmar / avanzar paneles |
| P | Pausa |
| R | Reintentar nivel (en Game Over) |
| ESC | Volver al menú |

## Los 3 niveles

1. **Reconstrucción en la Red** — Recupera los 7 nodos de datos en quechua del
   mito de Pariacaca (cada uno se "traduce" en pantalla) esquivando las llamas
   digitales de Huallallo Carhuincho.
2. **La Ofrenda Digital** — Escolta la ofrenda hasta el Santuario de Macahuisa
   guiado por el algoritmo ancestral. Si un firewall te toca, vuelves al inicio
   del camino. Recompensa: **Poder de los Apus (Ray-Trace)**.
3. **El Código Madre** — Compila el manuscrito activando 3 pilares de información
   (mantén E cerca). **Huallallo** drena el pilar más avanzado: decide cuál
   proteger y desfragmenta enemigos con el Ray-Trace. Al compilar los 3 pilares,
   el manuscrito se sube a un servidor de acceso abierto: *el conocimiento es libre*.

## Arquitectura (POO)

```
Entidad (base: posición, vida, hitbox, mover/mostrar virtuales)
├── DeidadLluvia      → jugador (animaciones del sprite sheet 9x7, espejado, invulnerabilidad)
├── LlamaDigital      → enemigo patrulla/persecución (dibujo procedural)
│   └── HuallalloJefe → jefe final (corrompe pilares, barra de vida)
├── NodoDato          → fragmento quechua coleccionable
├── PilarInformacion  → objetivo del nivel 3 (progreso de compilación)
├── SantuarioMacahuisa→ meta del nivel 2
├── AliadoAlgoritmo   → espíritu guía
└── RayoTrace         → proyectil del Poder de los Apus

Juego (ref class controladora) → estados (Intro/Jugando/Pausa/Completado/Victoria/Derrota),
                                  niveles, cámara con límites, HUD, audio WMPLib
Forms: MenuForm → JuegoForm / MenuInstruccionesForm / CreditosForm
```

## Gestión de archivos (FILES/)

El juego lee y escribe dos archivos con las librerías estándar de C++ (`fstream`),
validando su existencia e integridad (muestra un `MessageBox` si hay problemas):

- **`FILES/PARAMETERS.txt`** (texto): configuración leída al iniciar — vidas,
  tiempo de cada nivel, número de enemigos por nivel, aliados, nodos y pilares.
  Si no existe, se crea con valores por defecto. Editarlo cambia la dificultad.
- **`FILES/SCORES.bin`** (binario): registros de puntuación (nombre del jugador,
  puntaje y fecha). Se agrega uno al terminar cada partida y la tabla de mejores
  puntajes se muestra en las pantallas YOU WIN / YOU LOST.

## Equipo

| Integrante | Rol |
|---|---|
| Manuel Gustavo Arimana Huachuhillca | Diseñador del videojuego |
| Lenin Gabriel Palomino Díaz | Artista 2D/3D/Animador |
| Bryan Sebastián Córdova Alfaro | Programador |
| Mustafa Altuntas | Programador |
| Biary Edver Tineo Palacios | Programador |
