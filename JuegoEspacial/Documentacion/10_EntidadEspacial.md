# 10 · EntidadEspacial — la clase madre abstracta (UML)

**Archivos:** `EntidadEspacial.h`, `EntidadEspacial.cpp`
**UML:** atributos `x: float`, `y: float` · métodos `mover()`, `dibujar()`

## ¿Para qué sirve?

Es la **raíz de la jerarquía**: todo lo que existe en el mundo del juego (enemigos,
ítems, el astronauta, la nave, los proyectiles) ES una `EntidadEspacial`. Define el
contrato mínimo: *tienes una posición, sabes moverte y sabes dibujarte*.

```cpp
public ref class EntidadEspacial abstract {
protected:
    float x;
    float y;
public:
    EntidadEspacial(float px, float py);
    virtual void mover() = 0;                 // método abstracto (puro)
    virtual void dibujar(Graphics^ g) = 0;    // método abstracto (puro)
    float getX();  float getY();
    void setPosicion(float px, float py);
};
```

## Conceptos de POO que demuestra (para defender el proyecto)

1. **Clase abstracta** (`abstract`): no se puede hacer `gcnew EntidadEspacial(...)`.
   Solo existe para ser heredada. Igual que en el UML, donde aparece con (A).

2. **Métodos virtuales puros** (`= 0`): obligan a cada hija a implementar SU forma de
   moverse y dibujarse. Un `Asteroide` se mueve con velocidad y giro; una `Estrella`
   cae girando; el `JefeFinal` oscila de lado a lado. Mismo verbo, conductas distintas.

3. **Encapsulamiento**: `x` e `y` son `protected` (el cuadrito rojo del UML = privado
   hacia afuera): nadie de afuera las toca directamente; se leen con `getX()/getY()` y
   se cambian con `setPosicion()`. Las hijas sí pueden usarlas directo (son protected).

4. **Polimorfismo**: gracias a esta base, un minijuego puede tener una
   `List<Asteroide^>` y recorrerla llamando `mover()` y `dibujar(g)` sin preguntar el
   tipo concreto. El compilador llama a la versión correcta sola.

## El árbol completo de herencia

```
EntidadEspacial (abstracta)
├── Enemigo (abstracta)  → Asteroide, BasuraEspacial, JefeFinal
├── Item (abstracta)     → Estrella, Artefacto
├── Astronauta
├── Nave
└── Proyectil
```

## ¿Por qué `float` y no `int`?

Las posiciones avanzan en pasos pequeños y suaves (ej. 2.9 px por tick, o
`sin(tick)*4`). Con `int` se perderían los decimales y el movimiento se vería a
saltitos. GDI+ acepta floats directamente en `FillEllipse`, `DrawLine`, etc.

## ¿Por qué `dibujar` recibe `Graphics^`?

El objeto `Graphics` ("el lápiz de la ventana") **solo existe durante el evento Paint**.
Por eso nadie lo guarda: viaja como parámetro desde
`MyForm::alPintar → Juego::dibujar → minijuego->dibujar → cadaEntidad->dibujar(g)`.
