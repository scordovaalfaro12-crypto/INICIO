# 21 · Superficie — el escenario de cada planeta

**Archivos:** `Superficie.h`, `Superficie.cpp` (clase estática)

## ¿Para qué sirve?

Pinta el "decorado" donde ocurren los aterrizajes y los minijuegos terrestres: cielo con
degradado, decoración característica del planeta y el suelo con piedras y cráteres.
La llaman `Aterrizaje::dibujar` y casi todos los minijuegos como primera línea:
`Superficie::dibujarFondo(g, planeta, ticks);`

## Las paletas (4 colores definen un planeta)

Cuatro funciones con un `switch` devuelven los colores según el índice (0=Mercurio …
7=Neptuno): `colorCieloArriba`, `colorCieloAbajo` (degradado), `colorSuelo` y
`colorSueloOscuro` (piedras/sombras). Ejemplos con intención:

| Planeta | Cielo | Por qué |
|---|---|---|
| Mercurio | negro → naranja quemado | está pegado al Sol |
| Venus | mostaza denso | atmósfera tóxica y espesa |
| Tierra (plataforma) | azul espacial | estás EN ÓRBITA, no en el suelo |
| Marte | salmón → rosado | el famoso cielo marciano |
| Júpiter | marrones cálidos | no hay suelo: solo nubes |
| Urano | azul petróleo → turquesa | caverna de hielo |
| Neptuno | índigo tormentoso | viento eterno |

## `dibujarFondo` — los 4 pasos

```cpp
1. LinearGradientBrush cielo(...arriba, ...abajo, 90°);   // el cielo
2. if (planeta oscuro) Fondo::dibujarEstrellas(g, tick, 130);  // estrellas tenues
3. dibujarDecoracion(g, planeta, tick);                   // lo que hace ÚNICO al planeta
4. dibujarSuelo(g, planeta, tick);                        // piso + piedras + cráteres
```
Las estrellas solo se pintan donde el cielo es oscuro (0, 2, 5, 6, 7); en Venus o
Júpiter, con cielo claro, se verían mal.

## `dibujarDecoracion` — la firma de cada planeta

- **Mercurio**: el Sol gigante = 3 círculos translúcidos cada vez más grandes (halo) +
  2 círculos sólidos (cuerpo y núcleo).
- **Venus**: 5 nubes (pares de elipses) que **cruzan en bucle** con el truco del módulo:
```cpp
float nx = ((tick * 4 + i * 2300) % 12400) / 10.0f - 120.0f;
```
  (cada nube va desplazada `i * 2300` para que no vayan en fila).
- **Tierra**: ¡el planeta Tierra asomando tras el horizonte! Se reusa
  `SistemaSolar::dibujarPlaneta(g, 2, 760, SUELO+690, 780, tick)`: un círculo de radio
  780 con su centro MUY por debajo de la pantalla → solo asoma el arco superior, como
  un amanecer. Encima, un `DrawArc` cian translúcido = atmósfera.
- **Marte**: 2 lunas (Fobos y Deimos) + una cordillera que es **un solo polígono** de 7
  puntos picudos.
- **Júpiter**: bandas horizontales translúcidas + la gran mancha roja (elipse).
- **Saturno (luna rocosa)**: el planeta Saturno enorme en el cielo (otra vez reusando
  `dibujarPlaneta` con radio 95).
- **Urano**: 9 estalactitas = triángulos colgando del borde superior, con alturas
  variadas por `abs(sin(i * 2.7)) * 70` (pseudoaleatorio pero ESTABLE entre cuadros).
- **Neptuno**: 6 ráfagas de viento = línea larga + línea corta que cruzan con módulo.

## `dibujarSuelo` — tres variantes

**Normal** (Mercurio, Venus, Marte, Urano, Neptuno):
rectángulo desde `y = SUELO` + línea clarita (horizonte) + **14 piedras** (polígonos de
4 puntos en posiciones sorteadas UNA vez con `asegurarDetalles()`) + 3 cráteres
(elipses solo contorno). En Urano las piedras se cambian por **cristales**
(triángulos celestes).

**Tierra — plataforma metálica:**
```cpp
for (int i = 0; i <= 10; i++)                       // líneas de paneles en diagonal
    g->DrawLine(%lineaPanel, i*100, SUELO, i*100 - 40, ALTO);
bool encendida = ((tick / 30) + i) % 2 == 0;        // balizas alternando
```
Las balizas naranjas parpadean en cadena: el `+ i` desfasa cada una.

**Júpiter — cubierta de nubes:** en vez de rectángulo, una fila de 12 elipses gordas
solapadas (borde esponjoso) + el relleno debajo + elipses de sombra. No hay piedras:
¡no hay suelo sólido en un gigante gaseoso!

## Ideas de diseño que conviene contar

1. **Reusar `dibujarPlaneta` para los cielos** (Tierra en el horizonte, Saturno gigante)
   da coherencia: el planeta del mapa y el del cielo son EL MISMO dibujo.
2. **El azar estable**: piedras y estalactitas se calculan una vez (o con funciones
   deterministas de `i`); si se sortearan por cuadro, el suelo "herviría".
3. Los minijuegos quedan limpios: su `dibujar()` empieza con UNA línea de fondo y se
   concentran en sus propios objetos.
