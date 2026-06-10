# 08 · Particula y Particulas — llamas, polvo y explosiones

**Archivos:** `Particula.h`, `Particula.cpp`

## ¿Para qué sirve?

Una **partícula** es un circulito con velocidad y vida que se va apagando. Sumando muchas
salen los efectos "jugosos": la llama del cohete, el polvo del aterrizaje, las
explosiones, las chispas de los rayos y los fuegos artificiales del final.

## La clase `Particula`

| Atributo | Para qué sirve |
|---|---|
| `x, y` | Posición |
| `vx, vy` | Velocidad (px por tick) |
| `radio` | Tamaño inicial |
| `vida, vidaMax` | Ticks restantes / totales (para calcular el desvanecido) |
| `color` | Color base |
| `conGravedad` | Si `true`, cae acelerando (`vy += 0.15` por tick) |

**Movimiento:**
```cpp
void Particula::mover() {
    x += vx;  y += vy;
    if (conGravedad) vy += 0.15f;   // las chispas de Júpiter caen como de verdad
    vida--;
}
```

**Dibujo (lo importante):**
```cpp
int alfa = (int)(255.0f * vida / vidaMax);          // se va volviendo transparente
float r = radio * (0.4f + 0.6f * vida / vidaMax);   // y también se encoge
SolidBrush pincel(Color::FromArgb(alfa, color));
g->FillEllipse(%pincel, x - r, y - r, r * 2, r * 2);
```
La fracción `vida/vidaMax` va de 1.0 → 0.0; con ella el alfa baja de 255 a 0 y el radio
del 100% al 40%. **Desvanecerse + encogerse** es lo que hace que un círculo simple se
sienta como fuego o humo.

## La clase `Particulas` (las recetas)

Funciones estáticas que operan sobre una `List<Particula^>^` (cada minijuego tiene la suya):

- `actualizar(lista)` — mueve todas y borra las muertas. Recorre **al revés**
  (`for i = Count-1 → 0`) porque borrar mientras avanzas hacia adelante saltaría elementos.
- `dibujar(g, lista)` — pinta todas.
- `explosion(lista, x, y, color, cantidad)` — el clásico estallido radial:
```cpp
float angulo = Azar::entreF(0, 6.2832f);          // dirección aleatoria (2π)
float velocidad = Azar::entreF(1.0f, 4.5f);
vx = Math::Cos(angulo) * velocidad;               // descomponer en x e y
vy = Math::Sin(angulo) * velocidad;
```
  Muchos puntos saliendo del mismo lugar en direcciones aleatorias = explosión.
- `polvo(lista, x, y, color, cantidad)` — partículas anchas, lentas y que suben un
  poquito (vy entre −1.4 y −0.2): la nube al aterrizar el cohete.
- `llama(lista, x, y, fuerza)` — UNA partícula por llamada (se llama cada tick desde el
  cohete), naranja o amarilla al 50%, que viaja **hacia abajo**: el chorro del motor.

## Quién las usa

| Efecto | Dónde |
|---|---|
| Chorro del motor + polvo al tocar suelo | `Aterrizaje` |
| Estelas de meteoritos y explosiones al impactar | `MinijuegoMercurio`, `Neptuno` |
| Destello al atrapar estrellas/artefactos | Venus, Marte, Saturno |
| Chispas que caen del rayo (con gravedad) | `MinijuegoJupiter` |
| Estela del motor de la nave | `MinijuegoSaturno` |
| Fuegos artificiales | `Juego::actualizar` en estado FINAL |
