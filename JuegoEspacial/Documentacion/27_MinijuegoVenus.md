# 27 · MinijuegoVenus — "Atrapa estrellas"

**Archivos:** `MinijuegoVenus.h/.cpp` · **Objetivo:** atrapar 15 estrellas · **Vidas:** 3

## La idea

El espejo de Mercurio: allá esquivabas TODO, acá tienes que **buscar el contacto** con
las estrellas… mientras esquivas las gotas de lluvia ácida. Perseguir y evitar a la
vez = decisiones cada segundo ("¿voy por esa estrella aunque venga una gota?").

## Atributos

| Atributo | Para qué sirve |
|---|---|
| `estrellas` | `List<Estrella^>` cayendo (vy 2.2..3.6) |
| `gotas` | `List<Proyectil^>` tipo GOTA (vy 3.6..5.2 — más rápidas que las estrellas) |
| `naveEstacionada` | Decorado |
| `atrapadas` | Contador del objetivo (15 gana) |
| `enfriamientoEstrella / enfriamientoGota` | Ritmo de aparición de cada cosa |

## `actualizar`, bloque por bloque

**1. Lluvias separadas:** dos contadores independientes; estrella cada 45–75 ticks,
gota cada `Math::Max(40, 85 - atrapadas * 3)` ticks → **la dificultad escala con TU
progreso**: cuantas más estrellas llevas, más ácido cae. Empezar es amable, terminar
exige.

**2. Atrapar (aquí brilla el UML):**
```cpp
if (e->zona().IntersectsWith(jugador->zona())) {
    jugador->recolectarItem(e);          // → e->aplicarEfecto(jugador) → +10 pts + sonido
    puntajeLocal += e->getValor();       // para el cartel de resultados
    atrapadas++;
    Particulas::explosion(..., dorado, 12);   // mini-festejo
    if (atrapadas >= 15) ganar();
}
```
Es la cadena completa del diagrama: `recolectarItem → aplicarEfecto → sumarPuntaje`.

**3. Estrella que llega al suelo:** se elimina **sin castigo**. Decisión de diseño:
castigar el fallo Y el ácido a la vez sería doble penalización; mejor que perder una
estrella solo "duela" en el tiempo.

**4. Las gotas:** si te tocan (y no eres invulnerable) → `perderVida` + salpicadura
verde; si tocan el suelo → salpicadura pequeña y fuera.

## El dibujo

Fondo de Venus (cielo mostaza + nubes que cruzan en bucle), cohete, estrellas (con su
halo pulsante y doble estrella, doc 17), gotas (elipse + triángulo + brillo, doc 13),
astronauta y partículas. La atmósfera amarilla densa hace que las estrellas doradas
**resalten** y las gotas verdes se camuflen un poco — a propósito: lo valioso se ve,
lo peligroso exige atención.

## `progreso()`

`"Estrellas: 7/15"` — el formato "x/total" comunica el objetivo sin explicaciones.
