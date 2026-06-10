# 26 · MinijuegoMercurio — "Lluvia de fuego"

**Archivos:** `MinijuegoMercurio.h/.cpp` · **Objetivo:** sobrevivir 30 segundos · **Vidas:** 3

## La idea

Estás tan cerca del Sol que llueven meteoritos ardientes. Solo te mueves a los lados
(A/D o ←/→) y esquivas. Es el minijuego "tutorial": una sola mecánica, fácil de
entender, pero que se pone serio al final.

## Atributos

| Atributo | Para qué sirve |
|---|---|
| `meteoros` | `List<Asteroide^>` — los que están cayendo |
| `naveEstacionada` | El cohete decorativo en (80, SUELO), recién aterrizado |
| `restante` | Ticks para ganar: `30 * Config::TICKS_SEGUNDO` = 1800 |
| `enfriamientoSpawn` | Ticks hasta el próximo meteorito |

## `actualizar`, bloque por bloque

**1. El reloj de la victoria:**
```cpp
restante--;
if (restante <= 0) { sumar(60); ganar(); return; }
```

**2. La dificultad creciente (lo importante):**
```cpp
float velocidadExtra = (1800 - restante) / 600.0f;       // 0.0 → 3.0 al final
meteoros->Add(gcnew Asteroide(x_aleatoria, -40,
    Azar::entreF(-0.9f, 0.9f),                           // leve deriva lateral
    Azar::entreF(3.0f, 5.2f) + velocidadExtra,           // caen MÁS rápido
    Azar::entreF(9.0f, 20.0f),                           // tamaños variados
    Color::FromArgb(225, 120, 50)));                     // color fuego
enfriamientoSpawn = Math::Max(13, 32 - (1800 - restante) / 80);  // y MÁS seguido
```
Dos curvas a la vez: con el paso del tiempo los meteoritos caen más rápido Y aparecen
más seguido (de uno cada 32 ticks a uno cada 13). Los últimos 10 segundos se sienten
como tormenta — exactamente la sensación buscada.

**3. Por cada meteorito (recorriendo la lista al revés para poder borrar):**
- estela de fuego: 1 partícula naranja cada 3 ticks;
- choque con el jugador (si no es invulnerable): `m->atacar()` (destello),
  `jugador->perderVida()`, explosión, se elimina; con 0 vidas → `perder()`;
- llegada al suelo: explosión pequeña, a veces suena (`Azar::entre(0,4)==0` — si
  sonaran TODAS sería una metralleta), se elimina y **+2 puntos** (esquivado = premiado).

## El dibujo (con el detalle de diseño más útil)

```cpp
// SOMBRAS: se pintan ANTES que los meteoros
int alfa = Math::Max(15.0f, 80.0f - (SUELO - m->getY()) * 0.12f);
g->FillEllipse(%sombra, m->getX() - r*0.8f, SUELO - 5, r*1.6f, 10);
```
Cada meteorito proyecta una **elipse oscura en el suelo justo debajo**, más opaca cuanto
más cerca está de caer. No es decoración: es la información con la que el jugador decide
dónde pararse. *Avisar el peligro = juego difícil pero justo.*

Luego: fondo de Mercurio (sol gigante), cohete estacionado, meteoros (polígonos
irregulares girando, ver doc 14), el astronauta y las partículas.

## `progreso()`

```cpp
String::Format(L"Sobrevive: {0} s", Math::Max(0, (restante + 59) / 60));
```
El `+59` redondea hacia arriba: con 1 tick restante aún dice "1 s" (nunca un confuso "0 s").
