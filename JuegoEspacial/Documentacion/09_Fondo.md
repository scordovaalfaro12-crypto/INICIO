# 09 · Fondo — el cielo estrellado animado

**Archivos:** `Fondo.h`, `Fondo.cpp`

## ¿Para qué sirve?

Pinta el espacio: degradado oscuro + 130 estrellas que titilan + una estrella fugaz
ocasional. Lo usan el menú, personajes, controles, créditos, el sistema solar, Saturno y
la pantalla final; y `Superficie` reutiliza solo las estrellas para los cielos nocturnos
de los planetas.

## Atributos (todos estáticos: hay un solo cielo)

| Atributo | Para qué sirve |
|---|---|
| `ex[], ey[]` | Posición de cada estrella |
| `etam[]` | Tamaño (1.0 a 2.8 px) — variedad visual |
| `efase[]` | Fase del parpadeo de cada una (0 a 2π) |
| `fugazX/Y/VX/VY` | Posición y velocidad de la estrella fugaz |
| `fugazVida` | Ticks que le quedan a la fugaz (40 al nacer, 0 = no hay) |

## Bloque 1 — `crear()` (perezoso: la primera vez que se dibuja)

```cpp
if (ex == nullptr) crear();    // "lazy init": se generan al primer uso
```
Sortea las 130 posiciones/tamaños/fases **una sola vez**. Si se sortearan en cada cuadro
el cielo herviría de ruido.

## Bloque 2 — el degradado

```cpp
LinearGradientBrush cielo(rect,
    Color::FromArgb(6, 6, 18),      // casi negro arriba
    Color::FromArgb(24, 14, 48),    // morado abajo
    90.0f);                         // ángulo: vertical
g->FillRectangle(%cielo, 0, 0, ANCHO, ALTO);
```
Un degradado de dos azules-morados oscuros se siente más "espacio profundo" que el negro
plano, y deja que las estrellas resalten.

## Bloque 3 — `dibujarEstrellas(g, tick, alfaMaximo)`

```cpp
float brillo = 0.45f + 0.55f * Math::Abs(Math::Sin(tick * 0.04 + efase[i]));
int alfa = (int)(alfaMaximo * brillo);
SolidBrush pincel(Color::FromArgb(alfa, 255, 255, 255));
g->FillEllipse(%pincel, ex[i], ey[i], etam[i], etam[i]);
```
- `Math::Sin(tick * 0.04 + efase[i])`: el reloj global + la fase **propia** de cada
  estrella → cada una parpadea a su propio ritmo.
- `Abs` evita brillos negativos; el resultado oscila entre 45% y 100% del brillo.
- El parámetro `alfaMaximo` permite reusar la función atenuada: 255 en el espacio,
  130 sobre los cielos de los planetas (estrellas tenues tras la atmósfera).

## Bloque 4 — la estrella fugaz

```cpp
if (fugazVida <= 0 && Azar::entre(0, 300) == 7) {   // ~1 vez cada 5 segundos
    fugazX = ...; fugazVX = 5..9 (izquierda o derecha); fugazVida = 40;
}
if (fugazVida > 0) {
    fugazVida--;  fugazX += fugazVX;  fugazY += fugazVY;
    int alfa = Math::Min(255, fugazVida * 8);        // se apaga al final
    Pen pluma(Color::FromArgb(alfa, 255, 255, 220), 2.0f);
    g->DrawLine(%pluma, fugazX, fugazY, fugazX - fugazVX * 5, fugazY - fugazVY * 5);
}
```
La fugaz es **una línea**: desde la posición actual hacia "5 ticks atrás"
(`x - vx*5`) → estela proporcional a su velocidad. Nace al azar (probabilidad 1/300 por
tick), cruza el cielo 40 ticks y muere desvaneciéndose. Detalle sutil que da vida al menú.
