# 31 · MinijuegoSaturno — "Carrera de anillos" (pilotas la NAVE)

**Archivos:** `MinijuegoSaturno.h/.cpp` · **Objetivo:** cruzar 12 anillos · **Sin corazones:** aquí la vida es el casco de la nave

## La idea

Cambio total de protagonista: no controlas al astronauta sino a la **Nave** (la clase
del UML con `integridad`, `recibirDanio`, `reparar`, `mostrarEstado`). Vuelas hacia la
derecha (el mundo "scrollea" hacia ti) y pasas por los huecos de barreras doradas hechas
de roca de anillo. Chocar daña el casco; las estrellas lo reparan.

## La clase auxiliar `AnilloCarrera`

```cpp
ref class AnilloCarrera {
public:
    float x;             // posición (se mueve hacia la izquierda)
    float brechaY;       // centro del hueco
    float brechaAlto;    // alto del hueco
    bool pasado;         // ¿ya lo cruzaste? (para no contar dos veces)
    bool golpeado;       // ¿ya te dañó? (para no dañarte 60 veces en el roce)
};
```
Las dos banderas son la lección de esta clase: un evento que dura muchos ticks
(atravesar/rozar) debe **dispararse una sola vez**.

## El pseudo-desplazamiento (cómo se simula avanzar)

La nave está **fija en x = 150**; lo que se mueve es el mundo:
```cpp
float velocidadScroll = 4.6f + pasados * 0.12f;     // el mundo acelera con tu progreso
anillo->x -= velocidadScroll;                       // todo viaja hacia la izquierda
```
Truco universal de los juegos de scroll: mover el fondo hacia atrás se siente como
volar hacia adelante. La `velocidad` del HUD también sube con cada anillo (+6 km/s).

## El control

```cpp
float dy = 0;
if (entrada->arriba) dy -= 5.4f;
if (entrada->abajo)  dy += 5.4f;
nave->setPosicion(150, clamp(y + dy, 95, 655));
nave->setInclinacion(dy * 3.2f);       // ¡se ladea hacia donde va!
```
La inclinación proporcional al movimiento es puro feedback visual: sin cambiar nada de
la física, la nave "se siente" viva. Cada 2 ticks se suelta una partícula naranja detrás
(la estela del motor).

## Generación infinita de anillos

```cpp
if (anillos->Count == 0 || ultimo->x < 1000 - 280) {            // cada 280 px
    float alto = Math::Max(150.0f, 215.0f - pasados * 5.0f);    // hueco cada vez menor
    anillos->Add(gcnew AnilloCarrera(1080, Azar::entreF(150, 560), alto));
}
```
La dificultad: el hueco pasa de 215 px a 150 conforme cruzas. Los anillos que salen por
la izquierda (`x < −60`) se borran.

## Choques, cruces y reparaciones

Cada anillo son **dos rectángulos** (barra superior: de 0 al borde del hueco; inferior:
del borde al fondo). Colisión contra `nave->zona()`:
```cpp
if (!anillo->golpeado && (zonaNave.IntersectsWith(barraSup) || ...Inf)) {
    anillo->golpeado = true;        // este anillo ya no daña otra vez
    nave->recibirDanio(20);         // método del UML (flash rojo + sonido)
    if (nave->destruida()) perder();
}
if (!anillo->pasado && !anillo->golpeado && anillo->x < nave->getX()) {
    anillo->pasado = true;
    pasados++;  sumar(20);  Sonido::anillo();
    if (pasados >= 12) ganar();
}
```
Detalle de justicia: un anillo `golpeado` **no cuenta** como pasado — cruzar rompiendo
no premia.

Las **estrellas reparadoras** aparecen cada 130–210 ticks, scrollean con el mundo y al
tocarlas: `recolectarItem` (+10 pts) **y** `nave->reparar(15)` — el único minijuego
donde un ítem cura.

## El dibujo

Espacio + **Saturno gigante** (`dibujarPlaneta` a radio 105) + polvo de anillos (líneas
veloces con módulo, cada una a velocidad distinta `8 + i % 4` = paralaje barato) +
barreras doradas con motas de roca y **bordes del hueco en cian brillante** (verde si ya
lo pasaste) + estrellas + la nave con `dibujarHorizontal` + un circulito del color de tu
traje en la ventana: **el piloto eres tú**.

## El HUD redefinido

```cpp
void MinijuegoSaturno::dibujarHUD(Graphics^ g) {
    Minijuego::dibujarHUD(g);     // barra común (sin corazones: usaVidas = false)
    nave->mostrarEstado(g);       // el panel del casco/velocidad (método del UML)
}
```
