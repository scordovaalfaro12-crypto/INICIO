# 24 · Aterrizaje — la cinemática del cohete

**Archivos:** `Aterrizaje.h`, `Aterrizaje.cpp`

## ¿Para qué sirve?

Reproduce la película de **aterrizar** (bajar → polvo → escotilla → escalera →
astronauta sale caminando) y, con `despegue = true`, la inversa (caminar de vuelta →
subir → ignición → despegue). Le da "cuerpo" al viaje: no te teletransportas al
minijuego, LLEGAS.

## Atributos

| Atributo | Para qué sirve |
|---|---|
| `planeta` | Qué escenario pinta (`Superficie::dibujarFondo`) |
| `despegue` | false = aterrizar, true = despegar |
| `tick` | El reloj de la película (todo se decide comparándolo) |
| `terminado` | `Juego` lo consulta para pasar a la siguiente pantalla |
| `naveX, naveY` | Posición del cohete (240 horizontal; la Y es la animada) |
| `nave` | Un objeto `Nave` propio (visual: llama, patas) |
| `jugador` | EL astronauta real del jugador (con su color elegido) |
| `particulas` | Llama del motor y polvo |
| `sonoDespegue / sonoAterrizaje` | Para disparar cada sonido UNA sola vez |

## La técnica: línea de tiempo por ticks

Todo `actualizar()` es una serie de `if` comparando `tick` contra "hitos". Es la manera
más simple de hacer una cinemática: **el guion está escrito en números**.

### Guion del ATERRIZAJE (60 ticks = 1 segundo)

| Ticks | Qué pasa |
|---|---|
| 0–150 | La nave baja de y=−30 a y=560 **frenando** + llama + partículas |
| 150 | Toca suelo: llama OFF, sonido `aterrizajeNave`, 32 partículas de polvo |
| 190–235 | La escotilla se abre y la escalera se **despliega** progresivamente |
| 235–265 | El astronauta baja por la escalera (su Y interpola 46 px) |
| 265–345 | Camina hacia su puesto (`jugador->caminar(2.9f, ...)` cada tick — piernas animadas gratis) |
| 330+ | Cartel "¡A explorar MARTE!" |
| 400 | `terminado = true` → `Juego` crea el minijuego |

**El descenso suave** (la parte más fina):
```cpp
float p = suavizar(tick / 150.0f);        // 0..1
naveY = -30.0f + (suelo + 30.0f) * p;
naveX = 240.0f + sin(tick * 0.06) * 4;    // se bambolea al bajar

float Aterrizaje::suavizar(float t) { return t * t * (3 - 2*t); }
```
`suavizar` es la curva **smoothstep**: arranca lenta, acelera al medio y FRENA al final.
Sin ella el cohete bajaría a velocidad constante y se "clavaría" en el suelo; con ella
parece que el motor lo frena de verdad. Es la diferencia entre animación robótica y
natural.

**La escalera progresiva:**
```cpp
float p = Math::Min(1.0f, (tick - 190) / 45.0f);   // 0..1 en 45 ticks
float largo = (SUELO - topeY) * p;                 // crece hacia el suelo
// 2 rieles (líneas) + un peldaño cada 12 px:
int cantidad = (int)(largo / 12.0f);
```
Los peldaños "aparecen" conforme la escalera baja porque su cantidad depende del largo.

### Guion del DESPEGUE

| Ticks | Qué pasa |
|---|---|
| 0–70 | El astronauta camina de regreso (`caminar(-3.2f)` mientras esté lejos de la nave) |
| 70–100 | Sube por la escalera (Y interpola hacia la escotilla) y desaparece |
| 100 | `Sonido::despegueNave()` (una vez, con la bandera `sonoDespegue`) |
| 100–160 | Ignición: llama ON y **la nave vibra** (`naveX = 240 + Azar::entreF(-2, 2)`) |
| 160+ | Sube acelerando: `naveY -= (tick - 160) * 0.14f` (cada vez más rápido) |
| naveY < −160 | `terminado = true` |

Detalle físico: al subir, la velocidad ES el tiempo transcurrido × 0.14 → aceleración
constante, como un cohete de verdad (al revés del aterrizaje, que desacelera).

## `saltar()`

`terminado = true` y ya. `Juego` la llama con clic/ENTER/ESC: el jugador nunca está
obligado a ver la cinemática otra vez (regla de oro de los juegos).

## El dibujo

`Superficie::dibujarFondo` → escotilla (elipse oscura cuyo alfa crece con la apertura)
→ escalera → `nave->dibujar(g)` → partículas → el astronauta (solo cuando está "fuera":
`tick >= 235` al aterrizar, `tick <= 100` al despegar) → rótulos con `textoSombra` →
pista "CLIC o ENTER para saltar".
