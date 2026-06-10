# 13 · Proyectil — láser, rayo y gota ácida

**Archivos:** `Proyectil.h`, `Proyectil.cpp`
**Hereda de:** `EntidadEspacial`

## ¿Para qué sirve?

Es **una sola clase para tres disparos distintos**, diferenciados por el campo `tipo`:

```cpp
literal int LASER = 0;   // disparo del jugador (sube)
literal int RAYO  = 1;   // rayo de energía del jefe final
literal int GOTA  = 2;   // gota ácida de las nubes de Venus
```

## Atributos

| Atributo | Para qué sirve |
|---|---|
| `vx, vy` | Velocidad. El láser usa (0, −9.5): recto hacia arriba. El rayo del jefe usa un vector **apuntado al jugador** (lo calcula `JefeFinal::dispararRayo`). La gota cae (0, +3.6..5.2) |
| `tipo` | Decide el dibujo y la hitbox |

## Métodos

- `mover()` — simplemente `x += vx; y += vy;` (línea recta).
- `fuera()` — true si salió de la pantalla (con 40–60 px de margen). Los minijuegos
  borran los proyectiles fuera para que las listas no crezcan infinito:
```cpp
if (laser->fuera()) { laseres->RemoveAt(i); continue; }
```
- `zona()` — hitbox según el tipo (el rayo es más gordo que el láser).
- `dibujar()` — tres recetas:

**El láser (el efecto "neón"):**
```cpp
Pen halo(Color::FromArgb(90, 90, 255, 255), 7.0f);     // línea gruesa translúcida
g->DrawLine(%halo, x, y, x, y + 14);
Pen nucleo(Color::FromArgb(240, 170, 255, 255), 3.0f); // línea fina casi blanca
g->DrawLine(%nucleo, x, y, x, y + 14);
```
> 📌 Truco general: **una línea gruesa semitransparente + una fina brillante encima**
> = resplandor láser. Se usa igual en el rayo del jefe y en el relámpago de Júpiter.

**El rayo del jefe:** mismo truco en magenta, pero la "cola" se dibuja hacia atrás
siguiendo su velocidad (`colaX = x - vx * 3.5`), así el trazo siempre apunta hacia
donde viaja, aunque vaya en diagonal.

**La gota ácida:** una elipse verde + un triángulo arriba (la puntita de la gota)
+ una motita clara de brillo. Tres figuras = una gota creíble.

## ¿De dónde salen los proyectiles?

| Tipo | Lo crea | Código |
|---|---|---|
| LASER | `Astronauta::disparar()` | `gcnew Proyectil(x, y - 52*escala, 0, -9.5f, LASER)` — nace a la altura del pecho y sube |
| RAYO | `JefeFinal::dispararRayo(objetivoX, objetivoY)` | normaliza el vector hacia el jugador y lo multiplica por la velocidad |
| GOTA | `MinijuegoVenus` | nace arriba de la pantalla en una x aleatoria |

El que dispara crea el proyectil y **el minijuego lo guarda en su lista**
(`List<Proyectil^>^`), lo mueve cada tick y comprueba sus colisiones.
