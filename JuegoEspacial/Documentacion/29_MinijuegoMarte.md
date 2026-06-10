# 29 · MinijuegoMarte — "Caza de artefactos" (vuelo libre)

**Archivos:** `MinijuegoMarte.h/.cpp` · **Objetivo:** recoger 6 artefactos · **Vidas:** 3

## La idea

Primer minijuego con movimiento **en las dos dimensiones**: vuelas con el jetpack
(W A S D / flechas) por todo el escenario recogiendo artefactos, mientras 4 rocas
rebotan eternamente como bolas de billar. El peligro aquí no te persigue: ES el
terreno, y tú decides la ruta.

## Atributos

| Atributo | Para qué sirve |
|---|---|
| `rocas` | `List<Asteroide^>` — las 4 rocas rebotonas |
| `actual` | El `Artefacto^` visible (solo existe UNO a la vez) |
| `naveEstacionada` | Decorado |
| `recogidos` | Contador del objetivo (6 gana) |

## `iniciar` — el setup

```cpp
jugador->setJetpack(true);          // ¡se enciende la llama de la mochila!
rocas->Add(gcnew Asteroide(
    posición aleatoria central,
    Azar::entreF(1.6f, 2.9f) * (moneda() ? 1 : -1),   // velocidad y sentido al azar
    Azar::entreF(1.4f, 2.6f) * (moneda() ? 1 : -1),
    Azar::entreF(16.0f, 26.0f), Color::FromArgb(150, 80, 50)));
nuevoArtefacto();
```

## El vuelo (suma de ejes)

```cpp
float dx = 0, dy = 0;
if (entrada->izquierda) dx -= 5;   if (entrada->derecha) dx += 5;
if (entrada->arriba)    dy -= 5;   if (entrada->abajo)   dy += 5;
jugador->volar(dx, dy, 40, 150, 960, SUELO);
```
Cada eje aporta por separado → con dos teclas vuelas en diagonal. `volar` limita a la
zona (no más arriba de y=150, no más abajo del suelo) y enciende la animación + el
fuego del jetpack.

## Las rocas eternas

```cpp
roca->mover();
roca->rebotar(30, 110, 970, SUELO);   // rebote en las 4 "paredes" (doc 14)
```
No se destruyen ni aparecen nuevas: las mismas 4 rebotan todo el minijuego. A los pocos
segundos el jugador "aprende" sus trayectorias — la habilidad aquí es leer el patrón.

## `nuevoArtefacto` — aparición justa

```cpp
for (int intento = 0; intento < 12; intento++) {
    ax = Azar::entreF(80, 920);  ay = Azar::entreF(170, 520);
    float dx = ax - jugador->getX(), dy = ay - jugador->getY();
    if (Math::Sqrt(dx*dx + dy*dy) > 220.0) break;    // lejos del jugador: OK
}
actual = gcnew Artefacto(ax, ay, 50);
```
Sortea hasta 12 veces buscando un punto a **más de 220 px de ti**. ¿Por qué? Si
apareciera encima del jugador sería un regalo sin mérito; lejos te obliga a cruzar el
campo de rocas. (Y si las 12 fallan, usa el último: nunca se cuelga.)

## Recoger

`actual->zona()` contra `jugador->zona()` → `recolectarItem` (+50 vía `aplicarEfecto`)
→ explosión cian → `recogidos++` → ¿6? `ganar()` : `nuevoArtefacto()`.

## El dibujo (dos detalles propios)

1. **Tormenta de arena**: cada 4 ticks nace una partícula que cruza horizontal
   (ambiente + sensación de velocidad).
2. **La flecha guía**: si el artefacto está a más de 320 px, un triangulito cian
   orbita al jugador apuntándole el camino (vector normalizado, doc 18). Sin ella el
   minijuego era "¿y ahora dónde está?"; con ella es "sé dónde, el problema es llegar".
