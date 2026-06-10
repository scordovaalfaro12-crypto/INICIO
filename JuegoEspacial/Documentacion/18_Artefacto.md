# 18 · Artefacto — la reliquia alienígena (UML)

**Archivos:** `Artefacto.h`, `Artefacto.cpp`
**UML:** método `aplicarEfecto()` · **Hereda de:** `Item` (valor = 50)

## ¿Para qué sirve?

El ítem "premium" del juego (vale 50 puntos, 5 veces una estrella). Es el objetivo del
minijuego de **Marte**: recoger 6 artefactos volando con el jetpack. Solo existe UNO a
la vez en pantalla: al recogerlo, el minijuego crea el siguiente en otra posición
(`nuevoArtefacto()`), lejos del jugador para obligarlo a cruzar el peligro.

## Atributos

| Atributo | Para qué sirve |
|---|---|
| `fase` | Su reloj personal. Arranca en un valor aleatorio (0..2π) para que dos artefactos seguidos no floten "iguales" |

## `mover()` — no se traslada, FLOTA

```cpp
void Artefacto::mover() { fase += 0.06f; }
float Artefacto::desplaceY() { return Math::Sin(fase) * 5.0f; }
```
El artefacto se queda en su sitio; lo que cambia es un desplazamiento vertical de ±5 px
con seno. Igual que en `BasuraEspacial`, ese `desplaceY()` se aplica **tanto al dibujo
como a `zona()`**, para que la colisión acompañe al balanceo.

## `aplicarEfecto`

```cpp
astronauta->sumarPuntaje(valor);   // +50
Sonido::item();
```

## El dibujo, figura por figura (cómo se construye "lo misterioso")

```cpp
float yv = y + desplaceY();                          // posición flotante

// 1. AURA: círculo cian translúcido cuyo alfa "respira"
int alfa = 50 + (int)(30.0 * Math::Sin(fase * 2.0));
g->FillEllipse(%aura, x - 24, yv - 24, 48, 48);

// 2. PIRÁMIDE dorada: un triángulo con borde más oscuro
piramide = { (x-14, yv+11), (x+14, yv+11), (x, yv-13) };
g->FillPolygon(%oro, piramide);  g->DrawPolygon(%filo, piramide);
g->DrawLine(%detalle, x-7, yv+5, x+7, yv+5);         // una "grieta" decorativa

// 3. GEMA que LATE en el centro
float pulso = 1.0f + 0.25f * Math::Sin(fase * 3.0);  // 0.75x .. 1.25x
g->FillEllipse(%gema, x - 4*pulso, yv + 1 - 4*pulso, 8*pulso, 8*pulso);

// 4. ANILLO orbital: elipse aplastada solo contorno
g->DrawEllipse(%anillo, x - 19, yv - 3, 38, 11);
```

Fíjate que las **tres animaciones usan la misma `fase` a distintas velocidades**:
- flotación → `sin(fase)` (lenta)
- aura → `sin(fase * 2)` (media)
- gema → `sin(fase * 3)` (rápida)

Capas de movimiento a ritmos distintos = sensación de objeto "vivo" con un solo reloj.

## Bonus: la flecha guía (vive en MinijuegoMarte, pero la provoca esta clase)

Si el artefacto está a más de 320 px del jugador, el minijuego dibuja un triangulito
cian a 70 px del astronauta **apuntando hacia el artefacto** (otra vez el vector
normalizado `dx/d, dy/d`). Sin esa flecha, encontrar el artefacto en medio de la
tormenta de arena era frustrante; con ella, el minijuego se siente guiado.
