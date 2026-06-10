# 33 · MinijuegoNeptuno — "El Jefe Final"

**Archivos:** `MinijuegoNeptuno.h/.cpp` · **Objetivo:** bajarle 24 de vida al jefe · **Vidas:** 3
**Se desbloquea** al completar los otros 7 planetas (el candado del mapa).

## La idea

El examen final: combina todo lo aprendido — moverte y esquivar (Mercurio/Júpiter),
disparar (Tierra) — contra un enemigo que **responde**: el `JefeFinal` (doc 16) con sus
rayos dirigidos, sus asteroides y su fase enfurecida.

## Atributos

| Atributo | Para qué sirve |
|---|---|
| `jefe` | El `JefeFinal` (vida 24) |
| `rayos` | `List<Proyectil^>` tipo RAYO (los del jefe) |
| `asteroides` | `List<Asteroide^>` (los que lanza el jefe) |
| `laseres` | `List<Proyectil^>` (los tuyos) |
| `enfriamientoAtaque` | Ticks entre ataques del jefe (115 normal, 70 enfurecido) |
| `avisoAtaque` | La cuenta regresiva entre la "carga" y el disparo real (24 ticks) |
| `tipoAtaque` | 0 = rayo, 1 = asteroide |

## El ciclo de ataque del jefe (cargar → avisar → disparar)

```cpp
if (avisoAtaque <= 0) {
    if (--enfriamientoAtaque <= 0) {
        jefe->atacar();                                  // ① carga (destello + línea)
        Sonido::tono(260, 70);                           //   zumbido grave de carga
        tipoAtaque = (Azar::entre(0, 3) == 2) ? 1 : 0;   //   2/3 rayo, 1/3 asteroide
        avisoAtaque = 24;                                // ② ventana para escapar
    }
} else if (--avisoAtaque == 0) {                         // ③ ¡fuego!
    if (tipoAtaque == 0)
        rayos->Add(jefe->dispararRayo(jugador->getX(), SUELO - 30));
    else {
        asteroides->Add(jefe->lanzarAsteroide());
        if (jefe->enfurecido()) asteroides->Add(jefe->lanzarAsteroide());  // ¡dobles!
    }
    enfriamientoAtaque = jefe->enfurecido() ? 70 : 115;
}
```
La secuencia carga→aviso→disparo usa los **tres métodos del UML del jefe** y es la
gramática del combate: el rayo apunta a donde estás **al disparar**, así que moverte
durante el aviso lo esquiva. Quien se queda quieto, aprende a no hacerlo.

## Tus láseres (tres blancos posibles)

Cada láser sube y se comprueba contra:
1. **El jefe**: `jefe->recibirGolpe(1)` + 10 pts + chispas magenta. Si `destruido()`:
   **tres explosiones simultáneas de colores** + 150 pts + `ganar()`.
2. **Los asteroides**: también puedes reventarlos (2 golpes, +5 pts) — decisión táctica:
   ¿gasto disparos en defenderme o ataco al jefe?
3. Nada: `fuera()` → se borra.

## Lo que te cae a ti

- **Rayos**: viajan en diagonal hacia tu posición; al tocar el suelo salpican partículas
  magenta. Te tocan → `perderVida` (con el parpadeo de invulnerabilidad de siempre).
- **Asteroides**: caen con deriva; explotan contra el suelo o contra ti.

Con 0 vidas → `perder()` (y la R te deja reintentar el combate completo).

## El HUD doble

```cpp
void MinijuegoNeptuno::dibujarHUD(Graphics^ g) {
    Minijuego::dibujarHUD(g);       // corazones + progreso ("Vida del jefe: 14/24")
    jefe->dibujarBarraVida(g);      // la barra grande del jefe (morada → roja)
}
```
Dos barras de vida en pantalla = lectura instantánea de cómo va el duelo.

## ¿Y al ganar?

`Juego::terminarMinijuego` marca Neptuno como completado (`nivel = 8`), tras el cartel
viene el **DESPEGUE**, y al terminar éste `Juego` detecta `nivel >= 8` y llama
`finalizar()` → la pantalla FINAL con fuegos artificiales, el puntaje total y los 4
astronautas del equipo marchando. Fin de la odisea. 🚀
