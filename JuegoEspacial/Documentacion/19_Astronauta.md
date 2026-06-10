# 19 · Astronauta — el personaje jugable (UML)

**Archivos:** `Astronauta.h`, `Astronauta.cpp`
**UML:** atributos `nombre: string`, `puntaje: int` · métodos `recolectarItem()`, `disparar()`
**Hereda de:** `EntidadEspacial`

## ¿Para qué sirve?

Es TU personaje. Un único objeto `Astronauta` vive durante toda la partida (lo crea
`Juego` al arrancar): camina, vuela, dispara, recoge ítems, pierde vidas y **acumula el
puntaje global**. Además presta su dibujo a los menús con el método estático
`dibujarModelo` (las 4 tarjetas de PERSONAJES son el mismo dibujo con otros colores).

## Atributos

| Atributo | Para qué sirve |
|---|---|
| `nombre`, `puntaje` | Los del UML: quién eres y cuántos puntos llevas |
| `colorTraje` | Azul/naranja/verde/rojo según el personaje elegido |
| `estilo` | 0..3: decide los accesorios del dibujo (ver abajo) |
| `escala` | Tamaño del dibujo (1.55 en las tarjetas, 0.9–0.95 en los minijuegos) |
| `fase` | Reloj de la animación de caminar |
| `caminando` | Bandera de "este tick me moví" (activa la animación) |
| `mirandoIzquierda` | Hacia dónde mira (espeja visor y mochila) |
| `jetpack` | Si dibuja la llama del propulsor (Marte) |
| `vidas` | Vidas del minijuego actual (cada `iniciar()` la pone en 3) |
| `invulnerable` | Ticks de inmunidad tras un golpe (90 = 1.5 s) |

## Los métodos del UML

```cpp
void Astronauta::recolectarItem(Item^ item) {
    if (item != nullptr) item->aplicarEfecto(this);   // el ítem decide su efecto
}
Proyectil^ Astronauta::disparar() {
    Sonido::disparo();
    return gcnew Proyectil(x, y - 52.0f * escala, 0.0f, -9.5f, Proyectil::LASER);
}
```
El láser nace a la altura del pecho (`y - 52*escala`, recuerda que `y` son los PIES) y
sube a 9.5 px/tick. Quien guarda el proyectil devuelto es el minijuego.

## Movimiento y animación (el sistema de la "bandera")

```cpp
void Astronauta::caminar(float dx, float minX, float maxX) {
    x += dx;  // y se limita entre minX y maxX
    caminando = true;
    mirandoIzquierda = dx < 0;
}
void Astronauta::mover() {                    // se llama 1 vez por tick
    if (caminando) fase += 0.32f;             // SOLO anima si hubo movimiento
    caminando = false;                        // se "consume" la bandera
    if (invulnerable > 0) invulnerable--;
}
```
`caminar`/`volar` se llaman solo cuando hay tecla presionada; encienden la bandera.
`mover()` avanza la fase únicamente si la bandera está encendida y la apaga. Resultado:
**las piernas se mueven solo cuando el personaje se desplaza**, sin código extra en los
minijuegos. `volar` es igual pero en 2D y con límites en los 4 lados (Marte).

## Vidas e invulnerabilidad

```cpp
void Astronauta::perderVida() { vidas--; invulnerable = 90; Sonido::danio(); }
```
Y en `dibujar()`:
```cpp
if (invulnerable > 0 && (invulnerable / 6) % 2 == 0) return;   // ¡parpadeo!
```
Mientras es invulnerable, el dibujo se salta 6 ticks sí / 6 no → el clásico parpadeo de
"me golpearon" de los juegos retro. Y los minijuegos comprueban
`!jugador->esInvulnerable()` antes de aplicar daño, dándote 1.5 s para escapar.

## `dibujarModelo` — el cuerpo, capa por capa (de atrás a adelante)

1. **Mochila** al lado contrario de la mirada (`px - lado*22*u`) + tanque.
2. **Llama del jetpack** (si vuela): 2 triángulos cuya punta tiembla con `sin(fase*2)`.
3. **Piernas**: 2 líneas gruesas (`Pen` de `7*u` con `LineCap::Round`) desde la cadera;
   los pies se separan con `balanceo = sin(fase) * 9 * u` (uno +, otro −) → zancada.
   Botas = elipses blancas en los pies.
4. **Brazo trasero** (línea de `6*u`, balancea al revés que las piernas: como al caminar
   de verdad).
5. **Torso**: rectángulo redondeado del color del traje + borde oscurecido (0.65).
6. **Panel del pecho**: rectángulo oscuro + 3 LEDs (rojo/amarillo/verde) + cinturón.
7. **Brazo delantero + guantes** (círculos blancos en las manos).
8. **Casco**: círculo blanco + "cuello" rectangular gris.
9. **Visor**: elipse oscura desplazada `lado * 2.5*u` hacia donde mira (efecto perfil)
   + reflejo blanco translúcido (el toque que lo hace parecer vidrio).
10. **Accesorios según `estilo`**:

| Estilo | Personaje | Accesorios |
|---|---|---|
| 0 | Bryan (Comandante) | Franja roja en el casco + **estrella dorada** en el pecho (reusa `Estrella::dibujarForma`) |
| 1 | Lenin (Ingeniero) | Antena con foco rojo + **llave inglesa** en el cinturón (rect + círculo) |
| 2 | Gustavo (Científico) | Visor **verde** + matraz (triángulo) + hombreras |
| 3 | Mustafa (Explorador) | Visor **ámbar** + linterna en el casco + mochila extra grande |

> 📌 Todo se multiplica por `u` (la escala) y los detalles laterales por `lado` (±1).
> Por eso el MISMO código dibuja al personaje gigante en el menú, chiquito en el juego,
> mirando a cualquier lado, con cualquier color.
