# 07 · Estilos — fuentes y utilidades de dibujo

**Archivos:** `Estilos.h`, `Estilos.cpp`

## ¿Para qué sirve?

Es la "caja de herramientas" visual: las fuentes compartidas y las operaciones de dibujo
que se repiten en todas las pantallas (rectángulos redondeados, textos centrados, textos
con sombra). Evita duplicar 10 líneas cada vez que quieres un panel bonito.

## Las fuentes (creadas UNA vez)

```cpp
static Font^ titulo  = gcnew Font(L"Segoe UI", 42.0f, FontStyle::Bold);
static Font^ grande  = gcnew Font(L"Segoe UI", 24.0f, FontStyle::Bold);
...
static Font^ hud     = gcnew Font(L"Consolas", 11.0f, FontStyle::Bold);
```
¿Por qué estáticas? Crear una `Font` cuesta un recurso del sistema (GDI). Si se crearan
dentro de `Paint` serían 60 fuentes nuevas por segundo → fuga de recursos. Aquí se crean
una vez y se comparten siempre. `Consolas` (monoespaciada) se usa en el HUD porque los
números no "bailan" al cambiar.

## Los formatos de texto

```cpp
static StringFormat^ centrado     // centrado horizontal y vertical
static StringFormat^ izquierda    // pegado a la izquierda, centrado vertical
static StringFormat^ centroArriba // centrado horizontal, pegado arriba (textos largos)
```
`DrawString(texto, fuente, pincel, PointF(x,y), centrado)` dibuja el texto con (x,y) como
**centro exacto** — comodísimo para botones y títulos.

## El rectángulo redondeado (no existe en GDI+, lo construimos)

```cpp
void Estilos::agregarRectRedondeado(GraphicsPath^ ruta, x, y, ancho, alto, radio) {
    float d = radio * 2;
    ruta->AddArc(x, y, d, d, 180, 90);                       // esquina sup-izq
    ruta->AddArc(x + ancho - d, y, d, d, 270, 90);           // sup-der
    ruta->AddArc(x + ancho - d, y + alto - d, d, d, 0, 90);  // inf-der
    ruta->AddArc(x, y + alto - d, d, d, 90, 90);             // inf-izq
    ruta->CloseFigure();                                     // une todo
}
```
Cuatro arcos de 90° (uno por esquina) unidos en un `GraphicsPath`. Los ángulos de GDI+
empiezan en las 3 en punto (0°) y crecen en sentido horario; por eso la esquina superior
izquierda es el arco que arranca en 180°. Con la ruta lista:

- `rellenarRedondeado(...)` → `g->FillPath` (paneles, botones, tarjetas)
- `bordeRedondeado(...)` → `g->DrawPath` (los contornos cian/dorados)

## Textos con personalidad

```cpp
void Estilos::textoSombra(g, texto, fuente, color, cx, cy) {
    SolidBrush sombra(Color::FromArgb(170, 0, 0, 0));
    g->DrawString(texto, fuente, %sombra, PointF(cx + 3, cy + 3), centrado);  // sombra
    SolidBrush pincel(color);
    g->DrawString(texto, fuente, %pincel, PointF(cx, cy), centrado);          // texto
}
```
La "sombra" es el mismo texto en negro translúcido desplazado 3px. Hace legibles los
títulos sobre cualquier fondo (lo usan "ODISEA ESPACIAL", "SISTEMA SOLAR", etc.).

## Aclarar y oscurecer colores

```cpp
Color Estilos::oscurecer(Color c, float f) {     // f=0.65 → 65% del brillo
    return Color::FromArgb(c.A, c.R * f, c.G * f, c.B * f);
}
```
Multiplicar los canales RGB acerca el color al negro; `aclarar` hace lo contrario hacia
el blanco. Así, del **único** color del traje del astronauta salen gratis el color de
sus piernas/brazos (oscurecido 0.65) y bordes. Cambias un color y toda la paleta
derivada acompaña.

## El truco de C++/CLI: pinceles "de pila"

En todo el proyecto verás:
```cpp
SolidBrush pincel(color);        // SIN gcnew
g->FillEllipse(%pincel, ...);    // % obtiene el "handle"
```
Al declararse sin `gcnew`, el pincel se **destruye solo al salir del bloque** (semántica
de pila de C++/CLI). Como pintamos 60 veces por segundo, esto evita acumular miles de
objetos GDI esperando al recolector de basura.
