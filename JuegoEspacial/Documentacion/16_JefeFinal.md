# 16 · JefeFinal — el platillo alienígena (UML)

**Archivos:** `JefeFinal.h`, `JefeFinal.cpp`
**UML:** métodos `atacar()`, `dispararRayo()`, `lanzarAsteroide()` · **Hereda de:** `Enemigo` (vida = 24)

## ¿Para qué sirve?

Es el jefe de Neptuno: un platillo con corona de púas giratorias y un ojo que te sigue.
Tiene dos ataques (rayo dirigido y asteroides) y un modo **enfurecido** cuando le queda
poca vida.

## Atributos

| Atributo | Para qué sirve |
|---|---|
| `vidaMax` | Para dibujar la barra de vida proporcional (`vida / vidaMax`) |
| `velocidadX` | Velocidad horizontal; se invierte al tocar los márgenes (rebota) |
| `objetivoX, objetivoY` | Hacia dónde mira su ojo (la posición del jugador, actualizada cada tick por el minijuego con `setObjetivo`) |
| `giro` | Ángulo de la corona de púas |
| `ticks` | Reloj propio: flotación y parpadeo de las luces |

## Métodos del UML

**`mover()`** — patrulla horizontal con rebote y rabia:
```cpp
giro += enfurecido() ? 2.6f : 1.2f;                  // las púas giran MÁS rápido
x += velocidadX * (enfurecido() ? 1.7f : 1.0f);      // y se mueve MÁS rápido
if (x < 130 && velocidadX < 0) velocidadX = -velocidadX;   // rebote en bordes
```

**`atacar()`** — pone `destello = 22`. Es el **telegraph** (aviso): durante esos ticks
se dibuja una bolita de energía cargándose y una línea tenue hacia el jugador. El
minijuego espera 24 ticks tras llamar `atacar()` antes de disparar de verdad: ese
tiempo es la oportunidad justa del jugador para moverse. *Un jefe sin avisos es
injusto; con avisos es difícil pero justo.*

**`dispararRayo(objetivoX, objetivoY)`** — crea un `Proyectil` tipo RAYO **apuntado**:
```cpp
float dx = objetivoX - x, dy = objetivoY - origenY;
float largo = Math::Sqrt(dx*dx + dy*dy);              // longitud del vector
float velocidad = enfurecido() ? 8.2f : 6.4f;
return gcnew Proyectil(x, origenY, dx/largo*velocidad, dy/largo*velocidad, RAYO);
```
Dividir el vector por su longitud lo **normaliza** (mide 1); multiplicarlo por la
velocidad da un proyectil que viaja exactamente hacia donde estabas al disparar.

**`lanzarAsteroide()`** — devuelve un `Asteroide` con velocidad aleatoria
(vx ±2.6, vy 2.2..3.6) desde su panza. En modo enfurecido el minijuego lanza DOS.

## `enfurecido()`

```cpp
return vida <= vidaMax / 3;      // último tercio de vida
```
Cambia 5 cosas a la vez: velocidad, giro de púas, color de púas (morado → rojo), aura
roja pulsante y la cadencia de ataques (el minijuego baja el enfriamiento de 115 a 70
ticks). El jugador NOTA el cambio de fase sin que nadie se lo diga.

## El dibujo, capa por capa

1. **Flotación**: `yv = y + sin(ticks * 0.05) * 6` — nunca está quieto del todo.
2. **Aura de furia** (si enfurecido): elipse roja con alfa oscilante.
3. **Corona de púas**: 10 triángulos colocados cada 36°:
```cpp
double a = (giro + i * 36.0) * Math::PI / 180.0;
triangulo[0] = punta a 62 px;  triangulo[1] y [2] = base a 38 px (a ± 0.16 rad);
```
   El `* 0.55` en las Y aplasta el círculo de púas → perspectiva de platillo.
4. **Platillo**: elipse morada 104×42 con borde más claro.
5. **Cúpula**: `FillPie(..., 180°, 180°)` = media elipse translúcida (se ve "de vidrio"
   porque su alfa es 150).
6. **Luces**: 5 circulitos que alternan amarillo/gris con `((ticks/9) + i) % 2` —
   el `+ i` hace el efecto "luces de feria" en cadena.
7. **El ojo que te sigue** (el detalle con más personalidad):
```cpp
float dx = objetivoX - x, dy = objetivoY - yv;
float largo = sqrt(dx*dx + dy*dy);
pupilaX = x + dx/largo * 5;       // la pupila se desplaza 5px hacia ti
```
   Mismo truco de normalizar: la pupila roja siempre apunta al jugador. Más un puntito
   blanco de brillo para que parezca esfera.
8. **Carga de ataque** (si `destello > 0`): línea rosa hacia el objetivo + bolita.

## `dibujarBarraVida(g)`

Panel arriba: fondo oscuro redondeado, barra proporcional
(`bancho * vida / vidaMax`), borde claro y el texto "JEFE FINAL". En furia la barra
pasa de morado a rojo. Se dibuja desde `MinijuegoNeptuno::dibujarHUD`.
