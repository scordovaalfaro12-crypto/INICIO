# 14 · Asteroide — la roca giratoria (UML)

**Archivos:** `Asteroide.h`, `Asteroide.cpp`
**UML:** métodos `atacar()`, `mover()` · **Hereda de:** `Enemigo` (vida = 2)

## ¿Para qué sirve?

Es el enemigo más reutilizado del juego:
- **Mercurio**: llueven como meteoritos ardientes (color fuego).
- **Marte**: rebotan por el escenario como rocas rodantes (color óxido).
- **Neptuno**: el jefe los lanza con `lanzarAsteroide()` (color rojizo oscuro).

El **color se pasa por constructor**, por eso la misma clase sirve para los tres planetas.

## Atributos

| Atributo | Para qué sirve |
|---|---|
| `vx, vy` | Velocidad de caída/desplazamiento |
| `radio` | Tamaño (9–26 px según el minijuego) |
| `angulo` | Rotación actual (grados) |
| `velocidadGiro` | Cuánto rota por tick (±1.0..3.2) — `moneda()` decide el sentido |
| `colorBase` | El color que le dio quien lo creó |
| `forma[8]` | **La clave de su look**: 8 radios relativos (0.72..1.0) sorteados al crearse |

## Por qué cada asteroide es único (el polígono irregular)

```cpp
// En el constructor: la "huella digital" de la roca
for (int i = 0; i < 8; i++) forma[i] = Azar::entreF(0.72f, 1.0f);

// En dibujar(): 8 vértices alrededor del centro
for (int i = 0; i < 8; i++) {
    double a = (angulo + i * 45.0) * Math::PI / 180.0;   // cada 45°
    float r = radio * forma[i];                          // radio "abollado"
    puntos[i] = PointF(x + Math::Cos(a) * r, y + Math::Sin(a) * r);
}
g->FillPolygon(%cuerpo, puntos);
```
Es un octágono donde cada vértice está a una distancia ligeramente distinta del centro:
eso lo hace verse como roca y no como señal de tránsito. Como `forma[]` se sortea UNA
vez, la silueta es estable; y como en `dibujar` se suma `angulo` (que crece en `mover`),
**el polígono entero rota**.

Encima se pintan: 2 elipses oscuras translúcidas (cráteres), una elipse clara arriba
(luz) y el contorno con el color oscurecido al 55%.

## Métodos

- `mover()` — `x += vx; y += vy; angulo += velocidadGiro;` y descuenta el `destello`.
- `atacar()` — pone `destello = 14`: al chocar contigo "se enciende" (aro blanco que se
  apaga en 14 ticks). El daño real lo aplica el minijuego llamando `jugador->perderVida()`.
- `rebotar(minX, minY, maxX, maxY)` — usado en Marte: si el borde del asteroide toca una
  pared **y va hacia ella**, invierte la componente:
```cpp
if (x - radio < minX && vx < 0) vx = -vx;
```
  El doble chequeo (posición Y dirección) evita el clásico bug del objeto que se queda
  "pegado" vibrando en la pared.
- `zona()` — rectángulo del 80% del radio (un pelín perdonador con el jugador).

## Detalle visual extra (Mercurio)

El minijuego le añade **sombra en el suelo** (elipse negra translúcida bajo cada
meteorito, más opaca cuanto más cerca está) y **estela de fuego** (partículas naranjas
cada 3 ticks). La sombra no es decoración: es la pista que usa el jugador para saber
dónde NO pararse.
