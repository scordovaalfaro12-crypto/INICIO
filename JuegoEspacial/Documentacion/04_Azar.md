# 04 · Azar — los números aleatorios

**Archivo:** `Azar.h` (solo encabezado)

## ¿Para qué sirve?

Centraliza el azar del juego. En .NET, crear muchos `Random` seguidos es un error típico
(se inicializan con el reloj y pueden dar la MISMA secuencia); por eso aquí hay **un solo
generador compartido** y funciones cómodas alrededor.

```cpp
public ref class Azar abstract sealed {
public:
    static Random^ generador = gcnew Random();

    static int entre(int minimo, int maximoExcluido) { return generador->Next(minimo, maximoExcluido); }
    static float real() { return (float)generador->NextDouble(); }          // 0.0 a 0.999...
    static float entreF(float a, float b) { return a + (b - a) * real(); }  // real entre a y b
    static bool moneda() { return generador->Next(2) == 0; }                // cara o cruz
};
```

## Los 4 métodos y cuándo se usan

| Método | Devuelve | Ejemplo real en el juego |
|---|---|---|
| `entre(a, b)` | entero `a..b-1` (¡b excluido!) | `Azar::entre(0, 4)` elige cuál de los 4 cristales se suma a la secuencia de Urano |
| `real()` | 0.0 → 1.0 | base de `entreF` |
| `entreF(a, b)` | decimal entre a y b | `Azar::entreF(3.0f, 5.2f)` = velocidad de caída de un meteorito |
| `moneda()` | true/false al 50% | decidir si un asteroide gira a la izquierda o a la derecha |

## El patrón "azar una vez, quieto para siempre"

Hay dos formas de usar el azar y es importante distinguirlas:

1. **Azar en cada tick** → cosas vivas: la llama del cohete tiembla
   (`Azar::entreF(0,3)` cada cuadro), el zigzag del rayo de Júpiter se redibuja distinto
   cada cuadro (por eso "vibra" como electricidad).
2. **Azar una sola vez al crear** → cosas fijas: las 130 estrellas del fondo, las 14
   piedras del suelo o la forma de cada asteroide (sus 8 radios `forma[i]` se sortean en
   el constructor). Si se sortearan en cada cuadro, ¡la piedra "herviría"!
