# 23 · MenuJuego — las 4 pantallas del menú

**Archivos:** `MenuJuego.h`, `MenuJuego.cpp`

## ¿Para qué sirve?

Dibuja y gestiona el menú principal, PERSONAJES, CONTROLES y CRÉDITOS, todo dentro de la
misma ventana. No cambia pantallas por su cuenta: cuando el usuario hace algo, devuelve
una **`AccionMenu`** y `Juego` decide.

```cpp
public enum class AccionMenu {
    Ninguna, Iniciar, Personajes, Controles, Creditos, Salir, Volver, ElegirPersonaje
};
```
Este enum es el "idioma" entre el menú y el cerebro: el menú dice QUÉ pasó, el cerebro
decide QUÉ HACER. Separación de responsabilidades de manual.

> Nota: la clase se llama `MenuJuego` y no `Menu` porque Windows Forms ya tiene una
> clase `Menu` y chocarían los nombres.

## Atributos

| Atributo | Para qué sirve |
|---|---|
| `opcion` | Botón resaltado del menú principal (0..4), movible con ratón Y flechas |
| `tarjeta` | Tarjeta resaltada en PERSONAJES |
| `personajeClicado` | Cuál tarjeta se clicó (lo lee `Juego` tras recibir `ElegirPersonaje`) |
| `volverMarcado` | Hover del botón VOLVER |
| `animacion` | 0→1, sube 0.045 por cuadro: la entrada animada de cada pantalla |
| `nombres / titulos / descripciones` (static) | Los datos de los 4 personajes (también los usa `Juego`) |
| `colorPersonaje(i)` (static) | Azul / naranja / verde / rojo |

## La animación de entrada (sutil pero elegante)

`Juego` llama `reiniciarAnimacion()` (pone `animacion = 0`) al entrar a cualquier
pantalla, y cada `dibujar*` la avanza. Cómo se usa:
```cpp
float retraso = (1.0f - animacion) * (60.0f + indice * 50.0f);
bx += retraso;     // cada botón entra deslizándose, escalonado
```
Botones del menú: entran desde la derecha en cascada. Tarjetas: suben. Créditos: cada
fila entra desde un lado alterno. Cuando `animacion` llega a 1, todo queda en su sitio.

## La pantalla principal

- Fondo estrellado + planeta anillado + Neptuno chico (decoración con
  `SistemaSolar::dibujarPlaneta`).
- **El cohete paseando**: posición con dos senos desfasados
  (`x = 130 + sin(tick*0.011)*70`, `y = 470 + cos(tick*0.014)*45`) → describe óvalos
  suaves que nunca se repiten igual; con `Rotate(90 + sin*6)` se ladea como flotando.
- Título con `textoSombra` + subtítulo + lema.
- **5 botones** (`rectBoton(i)` calcula su rect):
  - Normal: relleno azul muy oscuro translúcido + borde gris-azul.
  - Marcado: relleno azul brillante, **borde cian cuyo alfa late**
    (`160 + 60*sin(tick*0.12)`), desplazado 8 px y con flecha (triángulo) al costado.
  - Icono geométrico por botón: ▶ verde (INICIAR), casco mini (PERSONAJES), tecla "W"
    (CONTROLES), estrella girando (CRÉDITOS), puerta con pomo (SALIR).

## El manejo del ratón (patrón hover + clic)

```cpp
bool MenuJuego::ratonPrincipal(int mx, int my) {
    for (int i = 0; i < 5; i++)
        if (rectBoton(i).Contains((float)mx, (float)my)) {
            if (opcion != i) { opcion = i; return true; }   // true = "cambió" → beep
            return false;                                   // ya estaba ahí: silencio
        }
    return false;
}
```
Devolver true **solo cuando cambia** es lo que hace que el sonido suene una vez por
opción y no 60 veces por segundo mientras el cursor está encima.

`clicPrincipal` busca el rect bajo el clic y devuelve la acción; `moverSeleccion(±1)`
hace lo mismo con teclado usando `(opcion + d + 5) % 5` (suma 5 para que el −1 nunca dé
índice negativo).

## PERSONAJES

4 tarjetas (`rectTarjeta(i)`): marco redondeado, el astronauta a escala 1.55 con
`Astronauta::dibujarModelo`, nombre, título con el color del traje y la descripción con
**ajuste de línea automático** (DrawString sobre un `RectangleF` + formato
`centroArriba`: GDI+ parte las líneas solo). La marcada se eleva 8 px con borde cian;
la elegida lleva borde dorado + cinta "SELECCIONADO". El astronauta de la tarjeta
marcada **marcha en el sitio** (`fase = tick * 0.18`).

## CONTROLES

Panel grande + 6 filas: etiqueta cian, las teclas dibujadas y la explicación.
`dibujarTecla` es la mini-joya: DOS rectángulos redondeados (uno gris oscuro 3 px más
abajo + el claro encima) = tecla con relieve 3D. `dibujarRaton` arma un ratoncito con
un rect redondeado, 2 líneas (botones) y la ruedita.

## CRÉDITOS

4 filas con borde del color de cada integrante (usando
`Color::FromArgb(130, colorPersonaje(i))` para heredar el color con transparencia),
casco mini, nombre, rol y estrella girando (`(tick*2 + i*45) % 360` — desfasadas).
