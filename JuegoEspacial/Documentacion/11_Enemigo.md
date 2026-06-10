# 11 · Enemigo — base abstracta de los enemigos (UML)

**Archivos:** `Enemigo.h`, `Enemigo.cpp`
**UML:** atributo `vida: int` · métodos `atacar()`, `mover()`
**Hereda de:** `EntidadEspacial` · **La heredan:** `Asteroide`, `BasuraEspacial`, `JefeFinal`

## ¿Para qué sirve?

Reúne lo que **todo enemigo comparte**: tiene vida, puede recibir golpes, puede ser
destruido, sabe atacar y tiene una zona de colisión. Lo que cambia entre enemigos
(cómo se mueven, cómo se ven, cómo atacan) queda como métodos abstractos.

```cpp
public ref class Enemigo abstract : public EntidadEspacial {
protected:
    int vida;
    int destello;     // ticks de resplandor al atacar o recibir un golpe
public:
    Enemigo(float px, float py, int pvida);
    virtual void atacar() = 0;            // cada enemigo ataca a su manera
    virtual RectangleF zona() = 0;        // su rectángulo de colisión
    void recibirGolpe(int danio);
    int getVida();
    bool destruido();
};
```

## Atributos

| Atributo | Para qué sirve |
|---|---|
| `vida` | Puntos de vida (del UML). Asteroide: 2 · Basura: 1 · Jefe: 24 |
| `destello` | Contador visual: cuando ataca o lo golpean se pone en 12–22 y, mientras sea > 0, el enemigo se dibuja con un aro/aura blanca que se va apagando. Es el "feedback" de que el golpe conectó |

## Métodos comunes (implementados aquí, heredados por todos)

```cpp
void Enemigo::recibirGolpe(int danio) {
    vida -= danio;
    destello = 12;        // ¡enciéndete para que el jugador vea el impacto!
}
bool Enemigo::destruido() { return vida <= 0; }
```
Así, el minijuego de la Tierra hace `pieza->recibirGolpe(1); if (pieza->destruido()) ...`
sin importar qué tipo de chatarra es. Y en Neptuno, el MISMO código sirve para el jefe.

## Métodos abstractos (cada hijo los rellena)

- `atacar()` — en Asteroide/Basura es "chocar" (encienden su destello al impactar);
  en el JefeFinal es la **carga de energía** que avisa que viene un rayo o asteroide.
- `zona()` — devuelve el `RectangleF` que se usa con `IntersectsWith` para las
  colisiones. Cada enemigo lo ajusta a su forma (el asteroide usa el 80% de su radio
  para que el roce visual coincida con el roce jugable).
- `mover()` y `dibujar()` siguen pendientes desde `EntidadEspacial`.

## Cómo se usan las colisiones (patrón repetido en los minijuegos)

```cpp
if (!jugador->esInvulnerable() && enemigo->zona().IntersectsWith(jugador->zona())) {
    enemigo->atacar();          // destello del enemigo
    jugador->perderVida();      // corazón menos + parpadeo + sonido
}
```
`RectangleF::IntersectsWith` la trae .NET: devuelve true si dos rectángulos se tocan.
Usar rectángulos (en vez de formas exactas) es el estándar en juegos 2D: simple y rápido.
