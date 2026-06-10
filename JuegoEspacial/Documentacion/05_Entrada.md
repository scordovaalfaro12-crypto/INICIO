# 05 · Entrada — el estado del teclado y el ratón

**Archivo:** `Entrada.h` (solo encabezado)

## ¿Para qué sirve?

Es un "buzón" sencillo con el estado **actual** de los controles. Los minijuegos no
escuchan eventos de Windows: en cada tick miran este objeto y ya.

```cpp
public ref class Entrada {
public:
    bool izquierda, derecha, arriba, abajo;  // teclas de movimiento mantenidas
    bool disparo;                            // ESPACIO mantenido
    int ratonX, ratonY;                      // última posición del cursor
};
```

## ¿Por qué existe? (el problema que resuelve)

Los eventos `KeyDown` de Windows llegan "a su ritmo" (con auto-repetición del teclado,
pausas, etc.). Un juego necesita otra cosa: saber **si la tecla está presionada en este
instante** para mover al personaje suave y constante. La solución clásica:

- `KeyDown` → `Juego::teclaAbajo` pone el bool en `true`.
- `KeyUp` → `Juego::teclaArriba` lo pone en `false`.
- El minijuego, en su `actualizar(entrada)`, hace:

```cpp
if (entrada->izquierda) jugador->caminar(-6.0f, 25.0f, 975.0f);
if (entrada->derecha)   jugador->caminar( 6.0f, 25.0f, 975.0f);
```

Resultado: mientras mantengas la tecla, se mueve 6 px **por tick** (≈360 px por
segundo), sin tirones.

## Detalles finos

- Soporta **dos teclas a la vez**: en Marte puedes volar en diagonal porque `izquierda`
  y `arriba` pueden ser `true` al mismo tiempo (se suman los desplazamientos).
- `disparo` es mantenido, pero cada minijuego le pone un **enfriamiento**
  (`enfriamientoDisparo = 16..20` ticks) para que mantener ESPACIO no dispare 60 láseres
  por segundo.
- Las teclas "de un solo toque" (ENTER, ESC, R) **no** pasan por aquí: las maneja
  directamente `Juego::teclaAbajo` según la pantalla, porque son órdenes puntuales y no
  estados.
