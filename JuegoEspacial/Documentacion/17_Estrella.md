# 17 · Estrella — el ítem clásico (UML)

**Archivos:** `Estrella.h`, `Estrella.cpp`
**UML:** método `aplicarEfecto()` · **Hereda de:** `Item` (valor = 10)

## ¿Para qué sirve?

El recolectable estrella dorada. Aparece en:
- **Venus**: caen del cielo y hay que atrapar 15.
- **Saturno**: flotan entre los anillos y además **reparan la nave** (+15 de casco).
- Decoración: icono del botón CRÉDITOS, el resultado de misión, las tarjetas… para eso
  existe el método estático `dibujarForma`, usable sin crear el objeto.

## Atributos

| Atributo | Para qué sirve |
|---|---|
| `vy` | Velocidad de caída (en Venus 2.2..3.6; en Saturno 0 porque ahí "scrollea" hacia la izquierda y la mueve el minijuego) |
| `tam` | Radio de la estrella |
| `angulo` | Rotación (gira +1.8° por tick en `mover()`) |

## `aplicarEfecto` (el método del UML)

```cpp
void Estrella::aplicarEfecto(Astronauta^ astronauta) {
    astronauta->sumarPuntaje(valor);   // +10 puntos
    Sonido::item();                    // ¡brillito!
}
```
Lo invoca `Astronauta::recolectarItem(item)` — ver doc 12 (Item) para el patrón completo.

## La matemática de la estrella de 5 puntas (la joya de esta clase)

```cpp
static void Estrella::dibujarForma(g, cx, cy, radio, anguloGrados, color) {
    array<PointF>^ puntos = gcnew array<PointF>(10);
    for (int i = 0; i < 10; i++) {
        double a = (anguloGrados - 90.0 + i * 36.0) * Math::PI / 180.0;
        float r = (i % 2 == 0) ? radio : radio * 0.45f;   // alterna punta / valle
        puntos[i] = PointF(cx + Math::Cos(a)*r, cy + Math::Sin(a)*r);
    }
    g->FillPolygon(%pincel, puntos);
}
```
Cómo leerla:
- Una estrella de 5 puntas tiene **10 vértices**: 5 puntas (radio completo) y 5 valles
  (radio chico, aquí el 45%).
- Se reparten cada 36° (360 / 10).
- El `- 90` rota todo para que la primera punta quede **hacia arriba** (en GDI+ el
  ángulo 0 apunta a la derecha).
- Pasar `anguloGrados` distinto cada cuadro (p. ej. `tick * 1.8f`) = estrella girando.

## El dibujo completo (`dibujar()`)

```cpp
int alfa = 60 + (int)(35.0 * Math::Sin(angulo * 0.1));            // halo "respirando"
g->FillEllipse(%halo, x - tam*1.6f, y - tam*1.6f, ...);           // resplandor
dibujarForma(g, x, y, tam,        angulo, Color(255, 210, 70));   // estrella dorada
dibujarForma(g, x, y, tam*0.55f,  angulo, Color(255, 240, 160));  // núcleo más claro
```
Tres capas: halo pulsante (reusa `angulo` como reloj) + estrella + estrella interior
clara. La doble estrella da el efecto de "oro brillante" sin degradados.

## La zona de colisión

`RectangleF(x - tam, y - tam, tam*2, tam*2)` — el cuadrado que la envuelve. Es
ligeramente generoso (incluye los valles), lo cual está bien: atrapar estrellas debe
sentirse fácil y justo, frustrar al jugador con hitboxes mezquinas no aporta.
