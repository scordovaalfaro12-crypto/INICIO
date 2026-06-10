# 28 · MinijuegoTierra — "Limpieza orbital" (estilo Space Invaders)

**Archivos:** `MinijuegoTierra.h/.cpp` · **Objetivo:** destruir las 15 piezas · **Vidas:** 3

## La idea

Homenaje a *Space Invaders*: una formación 3×5 de basura espacial marcha de lado a lado
sobre la plataforma orbital, baja un escalón en cada borde, y tú la destruyes a láser
(ESPACIO) antes de que llegue abajo. Primer minijuego con **disparo**.

## Atributos

| Atributo | Para qué sirve |
|---|---|
| `piezas` | `List<BasuraEspacial^>` — la formación (3 filas × 5 columnas) |
| `laseres` | `List<Proyectil^>` — tus disparos vivos |
| `totalPiezas` | 15, para calcular la velocidad según las destruidas |
| `direccion` | +1 derecha / −1 izquierda (de TODA la formación) |
| `enfriamientoDisparo` | 16 ticks entre láser y láser |

## La formación (en `iniciar`)

```cpp
for (int fila = 0; fila < 3; fila++)
    for (int columna = 0; columna < 5; columna++)
        piezas->Add(gcnew BasuraEspacial(
            190 + columna * 120,  110 + fila * 80,
            (fila + columna) % 3));        // alterna panel/satélite/tuerca
```

## La marcha del enjambre (el corazón del Invaders)

```cpp
float velocidad = 1.1f + (totalPiezas - piezas->Count) * 0.16f;   // ¡LA regla de oro!
bool bajar = false;
for each (pieza) {                       // ¿alguna tocaría el borde?
    float nx = pieza->getX() + direccion * velocidad;
    if (nx > 955 || nx < 45) bajar = true;
}
if (bajar) {
    direccion = -direccion;              // TODA la formación da la vuelta
    for each (pieza) pieza->setPosicion(x, y + 18);   // y baja un escalón
}
```
La regla de oro heredada del arcade original: **cada pieza destruida acelera a las
restantes** (de 1.1 hasta ~3.3 px/tick). La última pieza vuela — el final siempre es
tenso. Nota el orden: primero se pregunta si ALGUNA tocaría el borde, después se mueven
todas; así ninguna se sale ni baja dos veces.

## Disparar y acertar

```cpp
if (entrada->disparo && enfriamientoDisparo <= 0) {
    laseres->Add(jugador->disparar());     // el método del UML
    enfriamientoDisparo = 16;              // ~4 disparos por segundo máximo
}
```
Cada láser se mueve hacia arriba; contra cada pieza:
`pieza->recibirGolpe(1)` → como su vida es 1, `destruido()` → explosión cian +
`Sonido::explosion()` + **15 puntos** + se borra pieza y láser. El doble bucle se
recorre al revés y con `break` al acertar (un láser solo destruye una pieza).

## Las derrotas (dos maneras de perder)

1. Una pieza **te toca** → pierdes vida (y la pieza estalla).
2. Una pieza cruza la **línea de peligro** (`y > 505`) → derrota directa
   (la basura "cayó a la atmósfera"). La línea está dibujada: roja, discontinua
   (`DashStyle::Dash`) y pulsando con seno — el jugador siempre sabe dónde está el límite.

Victoria: `piezas->Count == 0` → +50 de bono.

## El dibujo

Fondo especial (doc 21): el planeta Tierra ASOMANDO tras el horizonte de la plataforma
metálica con balizas parpadeantes. Las piezas se balancean y rotan solas (doc 15), lo
que disimula que la formación es una cuadrícula rígida.
