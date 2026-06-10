# 22 · SistemaSolar — el mapa de misiones y el dibujo de los planetas

**Archivos:** `SistemaSolar.h`, `SistemaSolar.cpp`

## ¿Para qué sirve?

Dos trabajos:
1. **La pantalla del mapa**: el Sol, los 8 planetas clicables, las insignias ✓, el
   candado de Neptuno, el panel del jugador y el panel de información.
2. **El método estático `dibujarPlaneta(g, indice, cx, cy, radio, tick)`**: dibuja
   cualquier planeta a cualquier tamaño. Lo reusan el menú (decoración), `Superficie`
   (cielos) y el minijuego de Saturno (el gigante del fondo).

## Atributos

| Atributo | Para qué sirve |
|---|---|
| `px[8], py[8]` | Posición de cada planeta en el mapa (alturas alternadas para que no queden en fila aburrida) |
| `radios[8]` | Tamaños relativos reales: Júpiter 46, Saturno 36, Mercurio 16… |
| `marcado` | Planeta resaltado (por ratón o flechas); −1 = ninguno |

`nombre(i)` y `nombreMision(i)` devuelven los textos ("MERCURIO", "Lluvia de fuego").

## La interacción

```cpp
int SistemaSolar::planetaEn(int mx, int my) {
    for (int i = 0; i < 8; i++) {
        float dx = mx - px[i], dy = my - py[i];
        if (Math::Sqrt(dx*dx + dy*dy) <= radios[i] + 14.0f) return i;
    }
    return -1;
}
```
Distancia del cursor al centro ≤ radio + 14 → estás sobre el planeta. El `+14` hace
clicables también a los chiquitos (Mercurio sería frustrante con sus 16 px pelados).
`moverRaton` usa esto para el hover (y devuelve true para el "tic" sonoro);
`marcarConTeclado` permite elegir con ← → y el `% 8` da la vuelta circular.

**El candado:**
```cpp
bool desbloqueado(int indice, completados) {
    if (indice != 7) return true;            // todos libres menos Neptuno
    for (int i = 0; i < 7; i++) if (!completados[i]) return false;
    return true;                             // el jefe exige los 7 anteriores
}
```

## El dibujo del mapa

- **El Sol**: medio fuera de pantalla (cx = −55): 3 halos translúcidos + 12 rayos
  (líneas) que **giran** porque su ángulo incluye `tick * 0.3`.
- **Las órbitas**: para cada planeta se calcula su distancia y ángulo al Sol
  (`Atan2(dy, dx)`), y se dibuja un `DrawArc` tenue de ±16° alrededor — un trocito de
  su órbita real pasando por el planeta.
- **Flotación**: `py[i] + sin(tick * 0.03 + i * 1.3) * 4` — cada planeta con su fase.
- **Bloqueado**: velo circular oscuro + candado (rectángulo + arco + ojo de cerradura).
- **Completado**: círculo verde + palomita hecha con DOS líneas.
- **Marcado**: anillo cian pulsante (`alfa = 150 + 90*sin(tick * 0.15)`) + panel
  inferior con el nombre de la misión y la instrucción.

## `dibujarPlaneta` — la fábrica de planetas

Estructura común: halo suave → círculo base → **`SetClip` circular** → detalles →
sombra → `ResetClip` → extras fuera del clip.

```cpp
GraphicsPath recorte;
recorte.AddEllipse(cx - r, cy - r, r*2, r*2);
g->SetClip(%recorte);          // todo lo siguiente solo pinta DENTRO del círculo
```

Detalles por planeta (todo escalado por `r`, por eso funciona a cualquier tamaño):
- **Mercurio**: 3 cráteres. **Venus**: 2 arcos gruesos de nube.
- **Tierra**: continentes con `FillClosedCurve` (le das 4–5 puntos y GDI+ los une con
  curvas suaves = costas orgánicas) + nubes + **la Luna orbitando** (fuera del clip):
```cpp
double a = tick * 0.02;
lunaX = cx + cos(a) * r * 1.7;   lunaY = cy + sin(a) * r * 0.6;
```
  radios distintos (1.7 vs 0.6) = órbita elíptica vista en perspectiva.
- **Marte**: manchas + casquete polar. **Júpiter**: 4 bandas (rects clipped) + mancha roja.
- **Saturno**: bandas + **anillos en dos mitades**: el arco trasero (180°→360°) se pinta
  ANTES del planeta y el delantero (0°→180°) DESPUÉS, ambos con
  `Translate + Rotate(−18°)`; así el planeta queda "dentro" del anillo. Un segundo arco
  fino más oscuro hace de surco.
- **Urano**: bruma + anillo fino casi vertical (`Rotate(75°)`).
- **Neptuno**: tormenta oscura + arco de viento.

**La sombra** (en todos): dentro del clip, un círculo negro translúcido del MISMO tamaño
desplazado abajo-derecha → la "media luna" nocturna que da volumen 3D instantáneo.
