# 15 · BasuraEspacial — la chatarra orbital (UML)

**Archivos:** `BasuraEspacial.h`, `BasuraEspacial.cpp`
**UML:** métodos `atacar()`, `mover()` · **Hereda de:** `Enemigo` (vida = 1: un láser la destruye)

## ¿Para qué sirve?

Son las 15 piezas de chatarra del minijuego de la Tierra (estilo *Space Invaders*).
Hay **tres diseños** según `tipoPieza` (0, 1, 2), repartidos con `(fila + columna) % 3`
para que la formación se vea variada.

## Atributos

| Atributo | Para qué sirve |
|---|---|
| `tipoPieza` | 0 = panel solar roto · 1 = satélite viejo · 2 = tuerca gigante |
| `angulo, velocidadGiro` | Cada pieza rota lento sobre sí misma (±0.6..2.0°/tick) |
| `fase` | Reloj del balanceo vertical (sube/baja ±4 px con seno): parece flotar sin gravedad |

## La división del trabajo (importante para entender el diseño)

`mover()` **no** desplaza la pieza: solo gira y balancea (`angulo += velocidadGiro;
fase += 0.05`). El movimiento de "marcha lateral + bajada" lo controla el
**MinijuegoTierra**, porque las 15 piezas se mueven EN FORMACIÓN (todas a la vez, como
los invaders clásicos). La pieza es dueña de su animación individual; el minijuego es
dueño de la coreografía del grupo.

## El dibujo con transformaciones (la novedad de esta clase)

```cpp
g->TranslateTransform(x, yv);     // 1. mueve el origen al centro de la pieza
g->RotateTransform(angulo);       // 2. gira el "papel"
... aquí se dibuja TODO con coordenadas locales alrededor de (0,0) ...
g->ResetTransform();              // 3. devuelve el papel a su sitio (¡siempre!)
```
Dibujar "alrededor de (0,0)" y dejar que la transformación coloque y gire es mucho más
fácil que calcular a mano dónde queda cada esquina rotada.

### Receta del panel solar (tipo 0)
Marco gris (rect 34×22) → celda azul (rect 30×18) → 3 líneas celestes (la cuadrícula)
→ **un triángulo del color del fondo** en una esquina = mordisco roto. Ese triángulo
"faltante" es lo que cuenta la historia de que es basura.

### Receta del satélite (tipo 1)
Cuerpo gris → 2 alitas doradas (paneles) → antena (línea + circulito ROJO encima, su
luz de aviso) → `DrawArc` de 180° abajo = la antena parabólica.

### Receta de la tuerca (tipo 2)
Hexágono calculado con el mismo truco del polígono (6 vértices cada 60°) + círculo
oscuro al centro (el agujero).

## El balanceo sincronizado con la colisión

```cpp
float BasuraEspacial::desplaceY() { return Math::Sin(fase) * 4.0f; }
```
Este desplazamiento se usa **tanto en `dibujar` como en `zona()`**: la hitbox sube y
baja junto con el dibujo. Si no, podrías "atravesar" una pieza que se ve más arriba de
donde realmente colisiona.

## `atacar()`

Como en el asteroide: enciende `destello = 14`. Ocurre cuando una pieza toca al
astronauta (el minijuego le quita la vida al jugador y elimina la pieza con explosión).
