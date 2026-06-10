# 20 · Nave — el cohete (UML)

**Archivos:** `Nave.h`, `Nave.cpp`
**UML:** atributos `integridad: int`, `velocidad: float` · métodos `recibirDanio(cantidad)`, `reparar(cantidad)`, `mostrarEstado()`
**Hereda de:** `EntidadEspacial` — su (x, y) es el **centro de la base** (donde apoyan las patas)

## ¿Para qué sirve?

Es el cohete en TODAS sus apariciones:
- **Cinemática**: aterriza y despega (la dibuja `Aterrizaje`).
- **Estacionada**: decorado en el suelo de casi todos los minijuegos.
- **Jugable**: en Saturno la pilotas; ahí importan `integridad` y `velocidad`.

## Atributos

| Atributo | Para qué sirve |
|---|---|
| `integridad` | "Vida" del casco, 0..100 (UML). Saturno: cada choque −20 |
| `velocidad` | Se muestra en el HUD; en Saturno sube con cada anillo (sensación de ir más rápido) |
| `inclinacion` | Grados de inclinación visual (al subir/bajar en Saturno se ladea `dy * 3.2`) |
| `llama` | Si el motor está encendido (descenso, despegue, vuelo) |
| `patas` | Si se dibujan las patas (en vuelo se "repliegan": `false`) |
| `faseLlama` | Reloj del temblor del fuego |
| `parpadeoDanio` | Ticks de flash rojo tras un golpe |

## Métodos del UML

```cpp
void Nave::recibirDanio(int cantidad) {
    integridad -= cantidad;
    if (integridad < 0) integridad = 0;
    parpadeoDanio = 20;             // flash rojo
    Sonido::danio();
}
void Nave::reparar(int cantidad) {
    integridad += cantidad;
    if (integridad > 100) integridad = 100;   // tope (clamp)
}
```

**`mostrarEstado(g)`** — el panel HUD de Saturno:
- Rectángulo redondeado translúcido con borde cian.
- Texto "NAVE Casco:" + **barra de integridad** cuyo color avisa solo:
```cpp
Color colorBarra = verde;
if (integridad <= 60) colorBarra = amarillo;
if (integridad <= 30) colorBarra = rojo;
g->FillRectangle(%barra, bx, by, bancho * integridad / 100.0f, balto);
```
- Línea con `String::Format(L"Velocidad: {0:F0} km/s ({1}%)", velocidad, integridad)`.

## El truco maestro: UN dibujo, DOS orientaciones

```cpp
void Nave::dibujar(Graphics^ g) {            // de pie (suelo)
    g->TranslateTransform(x, y);
    g->RotateTransform(inclinacion);
    dibujarCuerpo(g);                        // siempre se dibuja "hacia arriba"
    g->ResetTransform();
}
void Nave::dibujarHorizontal(Graphics^ g) {  // volando a la derecha (Saturno)
    g->TranslateTransform(x, y);
    g->RotateTransform(90.0f + inclinacion); // ¡solo cambia el ángulo!
    dibujarCuerpo(g);
    g->ResetTransform();
}
```
`dibujarCuerpo` trabaja en coordenadas locales con la base en (0,0). Las
transformaciones colocan y giran ese dibujo donde haga falta. Cero duplicación.

## `dibujarCuerpo` — pieza por pieza

1. **Patas** (si `patas`): 2 líneas inclinadas + 2 pies rectangulares.
2. **Tobera**: trapecio gris oscuro (`FillPolygon` de 4 puntos).
3. **Llama** (si `llama`): naranja + amarilla, con la punta variando:
   `flicker = sin(faseLlama)*4 + Azar::entreF(0,3)` → seno (ritmo) + azar (chisporroteo).
4. **Fuselaje sin costura**: elipse (nariz, 32×44) + rectángulo (32×70) del MISMO blanco
   → el ojo los funde en una cápsula.
5. **Punta roja**: `FillPie(..., 180, 180)` pinta solo la mitad SUPERIOR de la elipse.
6. **Sombra lateral**: rectángulo negro alfa 45 pegado al borde derecho = volumen de
   cilindro al precio de un rectángulo.
7. **Aletas**: 2 triángulos rojos. **Franja** roja decorativa.
8. **Ventana**: 3 círculos (aro gris → vidrio azul → reflejo blanco).
9. **Remaches**: 4 puntitos grises (los detalles pequeños venden el dibujo).
10. **Flash de daño**: si `parpadeoDanio` está activo, un velo rojo intermitente
    (`(parpadeoDanio / 4) % 2 == 0`) cubre la nave.

## `zona()` — la hitbox en vuelo

```cpp
return RectangleF(x + 10, y - 15, 94, 30);
```
Cuando vuela girada 90°, la nave ocupa de la cola (x) a la punta (x+112). La hitbox es
un poco MÁS CHICA que el dibujo (94×30): el rozón visual no castiga — regla de oro para
que un juego se sienta justo.
