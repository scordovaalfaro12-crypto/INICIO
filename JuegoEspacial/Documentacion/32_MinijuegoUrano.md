# 32 · MinijuegoUrano — "Cristales de memoria" (Simón dice)

**Archivos:** `MinijuegoUrano.h/.cpp` · **Objetivo:** repetir la secuencia hasta la ronda 6 · **Errores:** 3 y pierdes

## La idea

Cambio total de género: nada de esquivar — **memoria**. En una caverna helada hay 4
cristales (cian, magenta, ámbar, verde) que se encienden en secuencia, cada uno con su
nota musical (Do-Mi-Sol-Do). Tú la repites haciendo **CLIC**. Cada ronda añade un paso.
Es el único minijuego 100% de ratón (por eso existe `Minijuego::clic` como virtual).

## Atributos

| Atributo | Para qué sirve |
|---|---|
| `secuencia` | `List<int>` con los cristales (0..3) a repetir; crece 1 por ronda |
| `faseSimon` | 0 = MOSTRANDO (el juego enseña) · 1 = ESPERANDO (tu turno) · 2 = PAUSA |
| `temporizador` | Ticks para el siguiente paso de la fase actual |
| `mostrando` | Qué índice de la secuencia se está enseñando |
| `paso` | Qué índice espera de ti |
| `errores` | 3 = derrota |
| `iluminado / iluminadoTicks` | Cristal encendido ahora y por cuántos ticks (26) |
| `cx[], cy[]` | Centros de los 4 cristales (rombo: arriba, derecha, abajo, izquierda) |

## La máquina de estados (en `actualizar`)

```cpp
if (faseSimon == 2) {                 // PAUSA dramática antes de mostrar
    if (--temporizador <= 0) { faseSimon = 0; mostrando = 0; }
}
else if (faseSimon == 0) {            // MOSTRANDO
    if (--temporizador <= 0) {
        if (mostrando < secuencia->Count) {
            encender(secuencia[mostrando]);    // luz + nota (26 ticks)
            mostrando++;
            temporizador = 38;                 // espacio entre notas
        }
        else { faseSimon = 1; paso = 0; }      // tu turno
    }
}
// faseSimon == 1: no hace nada aquí; manda el clic
```
Como la nota dura 26 ticks y el intervalo es 38, hay 12 ticks de silencio entre notas:
sin ese hueco, dos notas iguales seguidas sonarían como una sola larga.

## El clic (toda la lógica del juego está aquí)

```cpp
void MinijuegoUrano::clic(int mx, int my) {
    if (faseSimon != 1) return;            // si no es tu turno, se ignora
    int indice = cristalEn(mx, my);
    if (indice == -1) return;
    encender(indice);                      // tu clic también suena y brilla

    if (indice == secuencia[paso]) {       // ¿acertaste el paso?
        paso++;
        if (paso == secuencia->Count) {    // ¿completaste la ronda?
            sumar(15);
            if (secuencia->Count >= 6) { sumar(40); ganar(); return; }
            secuencia->Add(Azar::entre(0, 4));   // un paso más...
            faseSimon = 2;  temporizador = 55;   // ...y se vuelve a mostrar
        }
    }
    else {                                 // fallo
        Sonido::error();
        errores++;
        jugador->perderVida();             // los corazones del HUD = tus errores
        if (errores >= 3) { perder(); return; }
        faseSimon = 2;  temporizador = 70; // repite la MISMA secuencia
    }
}
```
Detalle amable: al fallar no se alarga la secuencia ni se reinicia el juego — se te
vuelve a mostrar la misma. El castigo es el error, no la humillación.

## El área de clic con forma de rombo

```cpp
if (Math::Abs(dx) / 52.0f + Math::Abs(dy) / 68.0f <= 1.0f) return i;
```
La ecuación `|dx|/a + |dy|/b ≤ 1` define **exactamente un rombo** (la "norma diamante").
La zona de clic tiene la MISMA forma que el dibujo del cristal — clics justos.

## El dibujo

- Caverna helada (estalactitas + cristales en el suelo, doc 21) + nave + astronauta
  quieto observando.
- Cada cristal: rombo de 4 puntos; **apagado** = su color oscurecido al 40%;
  **encendido** = color pleno + borde blanco + halo elíptico + facetas internas (4
  líneas blancas) más intensas. La diferencia encendido/apagado es enorme a propósito:
  el juego ES leer esas luces.
- Pedestal central con un núcleo que **late** (radio `10 + 2.5*sin(ticks*0.15)`) y
  cambia de color: ámbar = "OBSERVA...", verde = "¡REPITE!". El cartel superior dice lo
  mismo en texto: doble canal (color + texto) para que nunca dudes de quién es el turno.
