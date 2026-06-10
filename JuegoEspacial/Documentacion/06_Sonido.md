# 06 · Sonido — los efectos con tonos

**Archivos:** `Sonido.h`, `Sonido.cpp`

## ¿Para qué sirve?

Genera todos los efectos del juego con `Console::Beep(frecuencia, duración)`: tonos puros
del altavoz, sin archivos de audio (igual que todo lo visual es sin imágenes). Cada
efecto es una **secuencia de tonos** con personalidad propia.

## El problema y su solución (hilos)

`Console::Beep` es **bloqueante**: congela al hilo que lo llama hasta que el tono
termina. Si sonara en el hilo de la ventana, el juego se detendría 100 ms en cada pitido.
Solución: tocar en un hilo del **ThreadPool**:

```cpp
void Sonido::lanzar(array<int>^ datos) {
    if (!activado) return;
    ThreadPool::QueueUserWorkItem(gcnew WaitCallback(&Sonido::trabajador), datos);
}

void Sonido::trabajador(Object^ estado) {
    if (sonando) return;          // evita que los pitidos se amontonen
    sonando = true;
    array<int>^ datos = (array<int>^)estado;
    for (int i = 0; i + 1 < datos->Length; i += 2)
        Console::Beep(datos[i], datos[i + 1]);   // {frec, dur, frec, dur, ...}
    sonando = false;
}
```

La banderita `sonando` descarta sonidos nuevos mientras uno suena: sin ella, mover rápido
el ratón por el menú encolaría 30 beeps en fila.

## El "diseño musical" de cada efecto

| Método | Secuencia (Hz, ms) | Por qué suena así |
|---|---|---|
| `mover()` | 700·35 | Un "tic" corto y neutro al cambiar de opción |
| `seleccionar()` | 880·55 → 1175·75 | Dos notas **subiendo** = confirmación positiva |
| `error()` | 220·130 | Grave y largo = "no se puede" (candado de Neptuno) |
| `disparo()` | 1000·28 | Agudísimo y muy corto = "pew" |
| `explosion()` | 180·45 → 130·55 | Dos graves bajando = golpe |
| `item()` | 1320·45 → 1760·55 | Muy agudo subiendo = brillito de recompensa |
| `danio()` | 200·90 | Grave seco = ay |
| `victoria()` | 659→784→880→1047 | Arpegio ascendente (Mi-Sol-La-Do): fanfarria |
| `derrota()` | 392→330→262 | Tres notas **bajando**: tristeza |
| `notaSimon(i)` | 523/659/784/1047 ·220 | Do-Mi-Sol-Do agudo: el acorde de los 4 cristales de Urano |
| `anillo()` | 940·40 | "Ding" al cruzar un anillo en Saturno |
| `despegueNave()` | 150→220→320 | Graves subiendo = motor acelerando |
| `aterrizajeNave()` | 320→220→150 | Lo mismo al revés = motor frenando |

> 🎵 Regla de oro usada: **subir tono = bueno, bajar tono = malo, grave = golpe,
> agudo = premio.** Con eso el jugador "entiende" el sonido sin pensarlo.

## Otros detalles

- `activado` es un interruptor global por si quieren silenciar el juego (ponerlo en
  `false` en una sola línea).
- Las frecuencias de `notaSimon` son notas musicales reales (Do5=523, Mi5=659, Sol5=784,
  Do6=1047), por eso la secuencia de Urano suena melódica y es fácil de memorizar.
