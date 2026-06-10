# 02 · Juego — el cerebro (clase del UML)

**Archivos:** `Juego.h`, `Juego.cpp`
**UML:** atributos `nivel`, `estado` · métodos `iniciar()`, `actualizar()`, `finalizar()`

## ¿Para qué sirve?

Es la **máquina de estados** del juego: decide en qué pantalla estás, reparte los eventos
de teclado/ratón a quien corresponda y dibuja la pantalla activa. Ninguna otra clase sabe
"en qué momento del juego estamos"; todas obedecen a `Juego`.

## El atributo `estado` (el corazón de la clase)

Es un `String^` (tal como pide el UML) con uno de estos valores:

| estado | Pantalla |
|---|---|
| `"MENU"` | Menú principal |
| `"PERSONAJES"` / `"CONTROLES"` / `"CREDITOS"` | Subpantallas del menú |
| `"SISTEMA"` | Mapa del sistema solar |
| `"ATERRIZAJE"` / `"DESPEGUE"` | Cinemática del cohete |
| `"MINIJUEGO"` | Jugando una misión |
| `"FINAL"` | Galaxia completada (fuegos artificiales) |

Todos los métodos grandes (`actualizar`, `dibujar`, `teclaAbajo`, `clic`) empiezan con
una cadena de `if (estado == L"...")`: cada pantalla tiene su comportamiento.

## Atributos

| Atributo | Para qué sirve |
|---|---|
| `nivel` | Misiones completadas (0..8) — atributo del UML |
| `estado` | Pantalla actual — atributo del UML |
| `tick` / `tickEstado` | Reloj global / reloj desde el último cambio de pantalla (para animaciones de entrada y parpadeos) |
| `menu` | Objeto `MenuJuego` con las 4 pantallas de menú |
| `sistema` | El mapa `SistemaSolar` |
| `transicion` | El `Aterrizaje` en curso (aterrizar o despegar) |
| `minijuego` | El minijuego activo (puntero a la clase base `Minijuego`) |
| `jugador` | El `Astronauta` (vive TODA la partida, acumula el puntaje) |
| `entrada` | Teclas presionadas ahora mismo (se la pasa al minijuego) |
| `completados` | `array<bool>(8)`: qué planetas ya tienen su misión superada |
| `fiesta` | Partículas de los fuegos artificiales del FINAL |
| `personajeSel` | Índice del astronauta elegido (0..3) |
| `planetaActual` | En qué planeta estás aterrizando/jugando |
| `subMini` | Sub-estado del minijuego: 0 = cartel de instrucciones, 1 = jugando, 2 = resultado |
| `ganadoUltimo` / `bonusDado` | Para pintar el cartel de resultado correcto |

## Los métodos del UML

```cpp
void Juego::iniciar()   { cambiarEstado(L"SISTEMA"); Sonido::seleccionar(); }
void Juego::finalizar() { fiesta->Clear(); cambiarEstado(L"FINAL"); Sonido::victoria(); }
```
- `iniciar()` se ejecuta al pulsar INICIAR: te lleva al mapa.
- `finalizar()` se ejecuta cuando el **despegue de la 8ª misión** termina: pantalla final.
- `actualizar()` es el tick: ver siguiente sección.

## `actualizar()` — qué pasa 60 veces por segundo

```cpp
if (estado == L"ATERRIZAJE") {
    transicion->actualizar();
    if (transicion->haTerminado()) {        // al tocar suelo y salir el astronauta...
        minijuego = crearMinijuego(planetaActual);  // ...se crea el minijuego
        minijuego->iniciar();
        subMini = 0;                        // arranca mostrando las instrucciones
        cambiarEstado(L"MINIJUEGO");
    }
}
```
- En `DESPEGUE`: cuando termina, decide entre `finalizar()` (si `nivel >= 8`) o volver a `SISTEMA`.
- En `MINIJUEGO` con `subMini == 1`: llama `minijuego->actualizar(entrada)` y, si el
  minijuego pasó a ganado/perdido, salta a `subMini = 2` y llama `terminarMinijuego()`:

```cpp
if (ganadoUltimo && !completados[planetaActual]) {
    completados[planetaActual] = true;
    nivel++;                                // sube el nivel del UML
    jugador->sumarPuntaje(100);             // bonus por primera vez
}
```
- En `FINAL`: cada 18 ticks lanza una explosión de partículas de color al azar (fuegos
  artificiales) y de vez en cuando un tono musical.

## `crearMinijuego()` — polimorfismo en acción

```cpp
switch (indice) {
case 0: return gcnew MinijuegoMercurio(jugador);
case 1: return gcnew MinijuegoVenus(jugador);
...
default: return gcnew MinijuegoNeptuno(jugador);
}
```
Devuelve un `Minijuego^` (la clase base). Después `Juego` solo llama
`minijuego->actualizar(...)` y `minijuego->dibujar(...)` **sin saber cuál es**:
cada hijo hace lo suyo. Eso es el polimorfismo del UML funcionando.

## Entrada de teclado (`teclaAbajo`)

Primero actualiza las teclas "mantenidas" que usan los minijuegos:
```cpp
if (tecla == Keys::Left || tecla == Keys::A) entrada->izquierda = true;
if (tecla == Keys::Space) entrada->disparo = true;
```
Y luego, según `estado`, las teclas "de un toque": en MENU las flechas mueven la opción
(con `Sonido::mover()`), ENTER ejecuta; en SISTEMA eligen planeta; en ATERRIZAJE
cualquier tecla salta la cinemática; en MINIJUEGO ESC abandona (creando el DESPEGUE),
R reintenta tras perder, ENTER pasa del cartel de instrucciones al juego, etc.
`teclaArriba` solo vuelve a poner en `false` las teclas mantenidas.

## El ratón

- `ratonMovido` → pregunta a `menu` o `sistema` si el cursor cambió de botón/planeta;
  si sí, suena `Sonido::mover()` (el "tic" al pasar por opciones).
- `clic` → en MENU ejecuta la `AccionMenu` devuelta; en SISTEMA llama
  `entrarPlaneta(indice)`; en URANO se lo pasa al minijuego (`minijuego->clic(x,y)`).

`entrarPlaneta` valida el candado:
```cpp
if (!sistema->desbloqueado(indice, completados)) { Sonido::error(); return; }
planetaActual = indice;
transicion = gcnew Aterrizaje(indice, false, jugador);   // false = aterrizar
cambiarEstado(L"ATERRIZAJE");
```

## Los carteles que dibuja Juego

- `dibujarIntro` — panel con el nombre de la misión, las instrucciones del minijuego y
  "ENTER para comenzar" parpadeando (alfa con seno de `tickEstado`).
- `dibujarResultado` — "¡MISIÓN CUMPLIDA!" con una estrella dorada girando, o "MISIÓN
  FALLIDA" con una X roja; muestra puntos de la misión, bonus y total.
- `dibujarFinal` — título dorado, puntaje final, los 4 astronautas marchando (la fase de
  caminata es `tick * 0.15f` para que muevan las piernas sin moverse del sitio) y los
  fuegos artificiales.
