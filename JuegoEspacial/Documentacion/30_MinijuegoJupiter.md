# 30 · MinijuegoJupiter — "Tormenta eléctrica"

**Archivos:** `MinijuegoJupiter.h/.cpp` · **Objetivo:** sobrevivir 25 segundos · **Vidas:** 3

## La idea

Caminas sobre la cubierta de nubes de Júpiter mientras caen rayos. Cada rayo **avisa
antes de caer** (señal + línea guía) y además el viento te empuja: esquivar exige
anticipación, no reflejos.

## La clase auxiliar `RayoTormenta`

```cpp
ref class RayoTormenta {
public:
    float x;          // dónde caerá
    int t;            // su edad en ticks
    literal int AVISO = 50;   // 0..50  = fase de advertencia
    literal int FIN  = 72;    // 50..72 = fase de DESCARGA
};
```
Cada rayo es una pequeña **máquina de estados por edad**: nace avisando, descarga 22
ticks y muere. El minijuego guarda una `List<RayoTormenta^>`.

## `actualizar`, bloque por bloque

**1. El viento por ciclos:**
```cpp
cicloViento--;
if (cicloViento <= 0) { cicloViento = 240; viento = ±(1.6..2.6); }
if (cicloViento < 150) viento = 0;     // la racha dura 90 ticks, luego calma
...
float dx = viento;                              // el viento empuja SIEMPRE
if (entrada->izquierda) dx -= 5.6f;             // tu input se SUMA
if (entrada->derecha)  dx += 5.6f;
jugador->caminar(dx, 25, 975);
```
Cada 4 segundos sopla una racha de 1.5 s. Tu velocidad (5.6) siempre gana al viento
(2.6): **molesta pero nunca te domina** — la diferencia entre desafío y frustración.
El HUD avisa la dirección ("Viento →").

**2. Rayos cada vez más frecuentes:**
```cpp
enfriamientoRayo = Math::Max(36, 68 - (1500 - restante) / 60);
```

**3. La descarga:**
```cpp
if (rayo->t == AVISO) Sonido::tono(1500, 30);          // ¡chasquido al caer!
if (rayo->t > AVISO && rayo->t <= FIN) {
    if (Math::Abs(jugador->getX() - rayo->x) < 38)     // ¿estás en la columna?
        jugador->perderVida();
    // + chispas con GRAVEDAD que saltan del suelo
}
```
La colisión es por **distancia horizontal** (es una columna vertical, la altura no
importa). Las chispas usan `conGravedad = true`: saltan y caen en parábola.

## El dibujo del rayo (lo más vistoso del juego)

**Fase aviso** — todo parpadea con `sin(t * 0.5)`:
línea guía vertical tenue + triángulo amarillo + "!" en el punto de impacto.

**Fase descarga** — el zigzag que VIBRA:
```cpp
for (int p = 0; p < 8; p++) {
    float py = 50 + p * (SUELO - 50) / 7.0f;             // 8 alturas fijas
    float px = rayo->x + ((p == 0 || p == 7) ? 0 : Azar::entreF(-16, 16));
    zigzag[p] = PointF(px, py);                          // ¡desvío ALEATORIO!
}
g->DrawLines(%halo, zigzag);     // línea gruesa translúcida (el resplandor)
g->DrawLines(%nucleo, zigzag);   // línea fina casi blanca (el núcleo)
```
El secreto: los desvíos laterales se sortean **en cada cuadro** → el rayo se redibuja
distinto 60 veces por segundo y *vibra* como electricidad de verdad. (El primer y
último punto van fijos: nace en la nube y muere en su columna.) Más una columna de luz
translúcida y un destello en el suelo.

**El viento visible:** mientras sopla, 7 líneas cruzan la pantalla en su dirección
(módulo con desplazamiento por índice). Lo invisible no se puede esquivar; por eso el
viento se dibuja.
