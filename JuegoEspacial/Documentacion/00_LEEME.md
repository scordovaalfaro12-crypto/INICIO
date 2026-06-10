# 📚 Documentación de Odisea Espacial — Índice

Aquí hay **un documento por cada clase del juego**. Si los lees en orden, primero
entiendes el "esqueleto" (ventana → juego → pantallas) y después cada pieza.

## ¿Cómo está armado el juego? (la arquitectura en 1 minuto)

```
MyForm (la ventana única)
  └── Juego (el "cerebro": decide qué pantalla está activa)
        ├── MenuJuego        → menú principal, personajes, controles, créditos
        ├── SistemaSolar     → el mapa con el Sol y los 8 planetas
        ├── Aterrizaje       → cinemática de aterrizar/despegar
        └── Minijuego (8 hijos: uno por planeta)
              usan → Astronauta, Nave, Asteroide, BasuraEspacial,
                     JefeFinal, Estrella, Artefacto, Proyectil
              dibujan con → Superficie, Fondo, Estilos, Particula
              suenan con → Sonido
```

El **flujo de cada cuadro** (60 veces por segundo):
1. El `Timer` de `MyForm` dispara `alTick`.
2. `alTick` llama a `juego->actualizar()` (mover todo) y a `Invalidate()` (repintar).
3. Windows dispara el evento `Paint` → `juego->dibujar(g)` pinta la pantalla activa.

El **flujo del jugador**:
```
MENU → (Iniciar) → SISTEMA → (clic en planeta) → ATERRIZAJE → MINIJUEGO
   ↑                  ↑                                  │ (ganar/perder)
   └── ESC            └────────── DESPEGUE ←──── RESULTADO
                                     │ (si ya están los 8)
                                   FINAL (fuegos artificiales)
```

## Orden de lectura sugerido

| # | Archivo | Qué explica |
|---|---|---|
| 01 | `01_MyForm.md` | La ventana, el Timer, el doble búfer y los eventos |
| 02 | `02_Juego.md` | La máquina de estados que lo coordina TODO |
| 03 | `03_Config.md` | Las constantes globales (tamaño, suelo) |
| 04 | `04_Azar.md` | Números aleatorios |
| 05 | `05_Entrada.md` | El estado del teclado/ratón |
| 06 | `06_Sonido.md` | Los pitidos en otro hilo |
| 07 | `07_Estilos.md` | Fuentes y utilidades de dibujo (rectángulos redondeados, textos) |
| 08 | `08_Particula.md` | Llamas, polvo y explosiones |
| 09 | `09_Fondo.md` | El cielo estrellado animado |
| 10 | `10_EntidadEspacial.md` | La clase madre abstracta del UML |
| 11 | `11_Enemigo.md` | Base abstracta de los enemigos (UML) |
| 12 | `12_Item.md` | Base abstracta de los ítems (UML) |
| 13 | `13_Proyectil.md` | Láser, rayo del jefe y gota ácida |
| 14 | `14_Asteroide.md` | La roca giratoria |
| 15 | `15_BasuraEspacial.md` | Panel solar, satélite y tuerca |
| 16 | `16_JefeFinal.md` | El platillo alienígena |
| 17 | `17_Estrella.md` | El ítem estrella |
| 18 | `18_Artefacto.md` | El ítem reliquia |
| 19 | `19_Astronauta.md` | El personaje jugable y sus 4 diseños |
| 20 | `20_Nave.md` | El cohete (integridad, reparar, HUD) |
| 21 | `21_Superficie.md` | El escenario de cada planeta |
| 22 | `22_SistemaSolar.md` | El mapa y el dibujo de los 8 planetas |
| 23 | `23_MenuJuego.md` | Las 4 pantallas de menú y el ratón |
| 24 | `24_Aterrizaje.md` | La cinemática (línea de tiempo por ticks) |
| 25 | `25_Minijuego.md` | La base común de los 8 minijuegos |
| 26–33 | `26..33_Minijuego*.md` | Cada minijuego: reglas, dificultad y dibujo |

## ¿Dónde está cada clase del UML?

| Clase del UML | Archivos | Documento |
|---|---|---|
| Entidad base abstracta (x, y, mover, dibujar) | `EntidadEspacial.h/.cpp` | 10 |
| **Juego** (nivel, estado, iniciar, actualizar, finalizar) | `Juego.h/.cpp` | 02 |
| **Enemigo** (vida, atacar, mover) *(abstracta)* | `Enemigo.h/.cpp` | 11 |
| **Item** (valor, aplicarEfecto) *(abstracta)* | `Item.h/.cpp` | 12 |
| **Astronauta** (nombre, puntaje, recolectarItem, disparar) | `Astronauta.h/.cpp` | 19 |
| **Nave** (integridad, velocidad, recibirDanio, reparar, mostrarEstado) | `Nave.h/.cpp` | 20 |
| **Asteroide / BasuraEspacial / JefeFinal** (heredan de Enemigo) | 14 / 15 / 16 |
| **Estrella / Artefacto** (heredan de Item) | 17 / 18 |

> 💡 Consejo: abre el documento de una clase y el archivo `.cpp` a la vez,
> y ve comparando bloque por bloque. Así se aprende de verdad.
