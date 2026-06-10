# 12 · Item — base abstracta de los recolectables (UML)

**Archivos:** `Item.h`, `Item.cpp`
**UML:** atributo `valor: int` · método `aplicarEfecto()`
**Hereda de:** `EntidadEspacial` · **La heredan:** `Estrella`, `Artefacto`

## ¿Para qué sirve?

Modela todo lo que el astronauta puede **recoger**. Cada ítem tiene un valor en puntos y
un efecto que se aplica sobre quien lo recoge.

```cpp
ref class Astronauta;     // declaración adelantada (ver abajo)

public ref class Item abstract : public EntidadEspacial {
protected:
    int valor;
public:
    Item(float px, float py, int pvalor);
    virtual void aplicarEfecto(Astronauta^ astronauta) = 0;
    virtual RectangleF zona() = 0;
    int getValor();
};
```

## La relación con Astronauta (flechas del UML)

En el diagrama, el `Astronauta` tiene `recolectarItem()` y los ítems tienen
`aplicarEfecto()`. Así se conectan en el código:

```cpp
// En Astronauta.cpp:
void Astronauta::recolectarItem(Item^ item) {
    if (item != nullptr) item->aplicarEfecto(this);   // "aplícate sobre MÍ"
}

// En Estrella.cpp (una de las hijas):
void Estrella::aplicarEfecto(Astronauta^ astronauta) {
    astronauta->sumarPuntaje(valor);
    Sonido::item();
}
```

Fíjate en la elegancia: el minijuego solo hace `jugador->recolectarItem(estrella)`.
No sabe ni le importa qué hace el ítem — **el ítem decide su propio efecto**
(polimorfismo). Si mañana inventan un ítem "VidaExtra", solo crean otra hija con su
`aplicarEfecto` y ningún minijuego cambia.

## El truco del encabezado: `ref class Astronauta;`

`Item.h` necesita mencionar a `Astronauta`, y `Astronauta.h` necesita mencionar a `Item`
(en `recolectarItem`). Si cada uno hiciera `#include` del otro habría un **ciclo
infinito de inclusión**. La solución estándar:

- En el `.h` solo se **declara** que la clase existe: `ref class Astronauta;`
  (suficiente para usar `Astronauta^` como tipo de parámetro).
- En el `.cpp` sí se hace el `#include "Astronauta.h"` completo, porque ahí se llaman
  sus métodos.

## ¿Quién usa cada cosa?

| Miembro | Uso real |
|---|---|
| `valor` | Estrella = 10 pts, Artefacto = 50 pts. El minijuego también lo suma a su contador local (`puntajeLocal += item->getValor()`) para el cartel de resultados |
| `zona()` | Colisión con `jugador->zona()` (Venus, Marte) o con `nave->zona()` (Saturno) |
| `aplicarEfecto()` | Venus (estrellas), Marte (artefactos), Saturno (estrellas que además reparan la nave) |
