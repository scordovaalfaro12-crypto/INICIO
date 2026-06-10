# 📖 Guía del código: cómo se dibuja cada cosa

Esta guía explica **bloque por bloque** cómo funcionan los diseños del juego.
Todo se dibuja con GDI+ sobre el formulario: círculos (`FillEllipse`), rectángulos (`FillRectangle`),
polígonos (`FillPolygon`), arcos (`DrawArc`) y líneas (`DrawLine`). **No hay ni una imagen.**

---

## 0. El motor: cómo "cobra vida" el dibujo

En `MyForm.h` hay un **Timer** que dispara cada 16 ms (~60 veces por segundo):

```cpp
this->temporizador->Interval = 16;
this->temporizador->Tick += gcnew EventHandler(this, &MyForm::alTick);
```

Cada tick hace dos cosas:

```cpp
juego->actualizar();   // 1. mueve TODO (posiciones, contadores, animaciones)
this->Invalidate();    // 2. le dice a Windows "vuelve a pintar la ventana"
```

`Invalidate()` provoca el evento `Paint`, donde se dibuja **todo el cuadro desde cero**:

```cpp
e->Graphics->SmoothingMode = SmoothingMode::AntiAlias;  // bordes suaves
juego->dibujar(e->Graphics);                            // pinta la pantalla actual
```

Como el formulario tiene `OptimizedDoubleBuffer`, cada cuadro se pinta primero en una
imagen oculta y luego se muestra de golpe → **no parpadea**.

> 📌 **Idea clave:** un juego no "mueve" dibujos. Borra la pantalla y la vuelve a pintar
> 60 veces por segundo con las posiciones nuevas. La variable `tick` (que sube +1 cada
> cuadro) es el "reloj" con el que se animan todas las cosas.

### Coordenadas
- `(0,0)` es la **esquina superior izquierda**; X crece a la derecha, Y crece **hacia abajo**.
- La ventana mide 1000×700 (`Config::ANCHO`, `Config::ALTO`) y el suelo está en `y = 560` (`Config::SUELO`).

### Los 3 trucos que se repiten en todo el juego
1. **Oscilar con seno:** `x = centro + sin(tick * velocidad) * amplitud` → algo que va y
   viene suavemente (planetas flotando, cohete del menú, nave de Saturno).
2. **Transparencia:** `Color::FromArgb(alfa, r, g, b)` con alfa 0–255 → halos, sombras,
   paneles translúcidos, partículas que se desvanecen.
3. **Bucle infinito con módulo:** `x = (tick * velocidad) % anchoTotal` → cosas que cruzan
   la pantalla y reaparecen por el otro lado (nubes de Venus, viento de Neptuno, polvo de Saturno).

---

## 1. El fondo estrellado (`Fondo.cpp`)

**Bloque 1 — crear las estrellas (una sola vez):**
```cpp
ex[i]    = Azar::entreF(0, ANCHO);     // posición x al azar
ey[i]    = Azar::entreF(0, ALTO);      // posición y al azar
etam[i]  = Azar::entreF(1.0f, 2.8f);   // tamaño del puntito
efase[i] = Azar::entreF(0, 6.2832f);   // "reloj propio" de cada estrella
```
Se guardan 130 estrellas en arreglos. La **fase** hace que cada una parpadee en un
momento distinto (si no, las 130 parpadearían a la vez y se vería falso).

**Bloque 2 — el degradado del espacio:**
```cpp
LinearGradientBrush cielo(rectanguloPantalla,
    Color::FromArgb(6, 6, 18),     // casi negro arriba
    Color::FromArgb(24, 14, 48),   // morado oscuro abajo
    90.0f);                        // 90° = degradado vertical
g->FillRectangle(%cielo, 0, 0, ANCHO, ALTO);
```

**Bloque 3 — el parpadeo:**
```cpp
float brillo = 0.45f + 0.55f * Math::Abs(Math::Sin(tick * 0.04 + efase[i]));
int alfa = (int)(255 * brillo);
SolidBrush pincel(Color::FromArgb(alfa, 255, 255, 255));
g->FillEllipse(%pincel, ex[i], ey[i], etam[i], etam[i]);
```
El seno oscila entre −1 y 1; con `Abs` queda 0→1, y se convierte en un brillo del 45% al
100%. Cada estrella es solo un **circulito blanco con alfa variable**.

**Bloque 4 — la estrella fugaz:** cada cierto tiempo al azar (`Azar::entre(0,300)==7`) se
crea una posición y velocidad; mientras viva (40 ticks) se dibuja una **línea** desde su
posición hacia atrás (`x - vx*5`) que se va apagando bajando el alfa. Una línea que se
mueve + estela = fugaz.

---

## 2. El astronauta (`Astronauta::dibujarModelo`)

El personaje completo se dibuja **desde los pies** `(px, py)` hacia arriba, y todo se
multiplica por `u` (la escala) para poder dibujarlo grande en el menú y pequeño en el juego.

**Bloque 1 — la animación de caminar:**
```cpp
float balanceo = Math::Sin(fase) * 9.0f * u;   // -9..+9 px
float lado = mirandoIzquierda ? -1.0f : 1.0f;  // espeja los detalles
```
`fase` sube solo cuando camina (`fase += 0.32` por tick). El seno convierte ese contador
en un vaivén: una pierna adelante (+balanceo) y la otra atrás (−balanceo).

**Bloque 2 — orden de pintado (¡importante!):** se pinta de **atrás hacia adelante**,
como capas: mochila → pierna/brazo de atrás → torso → brazo de adelante → casco →
detalles. Lo último que pintas queda encima.

**Bloque 3 — la mochila:**
```cpp
float mochilaX = px - lado * 22.0f * u;   // siempre a la espalda
Estilos::rellenarRedondeado(g, gris, mochilaX, py - 68*u, 14*u, 32*u, 4*u);
```
Si el personaje mira a la izquierda, `lado` invierte el signo y la mochila se pasa al
otro costado automáticamente.

**Bloque 4 — piernas con líneas gruesas:**
```cpp
Pen pierna(trajeOscuro, 7.0f * u);
pierna.StartCap = LineCap::Round;          // puntas redondeadas
g->DrawLine(%pierna, px-5*u, caderaY, px-5*u + balanceo, pieY);
g->DrawLine(%pierna, px+5*u, caderaY, px+5*u - balanceo, pieY);
```
Una pierna no es un rectángulo: es una **línea de 7 px con puntas redondas** desde la
cadera hasta el pie. El pie de una usa `+balanceo` y el de la otra `−balanceo` → paso
alternado. Encima va una elipse blanca como bota.

**Bloque 5 — torso y panel de control:**
```cpp
Estilos::rellenarRedondeado(g, traje, px-14*u, py-66*u, 28*u, 36*u, 8*u);  // cuerpo
g->FillRectangle(%panel, px-8*u, py-60*u, 16*u, 11*u);                     // panel oscuro
// 3 circulitos: rojo, amarillo, verde (los "LEDs")
```
El color `traje` es el que elige el jugador en PERSONAJES (azul/naranja/verde/rojo).

**Bloque 6 — casco y visor:**
```cpp
g->FillEllipse(%casco, px-13.5*u, py-94*u, 27*u, 27*u);          // esfera blanca
float visorX = px - 9*u + lado * 2.5f * u;                       // se corre hacia donde mira
g->FillEllipse(%visor, visorX, py-89.5*u, 18*u, 16*u);           // cristal oscuro
g->FillEllipse(%reflejo, visorX+3*u, py-87*u, 5.5*u, 4*u);       // brillo blanquito
```
El visor desplazado 2.5 px hacia el lado que mira da la sensación de "perfil". El
**reflejo** (elipse blanca semitransparente) es lo que hace que parezca cristal. El color
del visor cambia por personaje: azul, azul, **verde** (Gustavo), **ámbar** (Mustafa).

**Bloque 7 — detalles por estilo:** un `if` por personaje: Bryan franja roja + estrella
dorada; Lenin antena con foco y llave (rect + círculo); Gustavo matraz (triángulo) y
hombreras; Mustafa linterna (rectangulito amarillo) y mochila extra grande.

---

## 3. El cohete (`Nave::dibujarCuerpo`)

Se dibuja una sola vez "apuntando hacia arriba" con la base en `(0,0)`, y luego:

```cpp
void Nave::dibujar(Graphics^ g) {              // parado en el suelo
    g->TranslateTransform(x, y);               // mueve el origen a (x,y)
    g->RotateTransform(inclinacion);
    dibujarCuerpo(g);
    g->ResetTransform();
}
void Nave::dibujarHorizontal(Graphics^ g) {    // volando (Saturno)
    g->TranslateTransform(x, y);
    g->RotateTransform(90.0f + inclinacion);   // ¡el MISMO dibujo girado 90°!
    dibujarCuerpo(g);
    g->ResetTransform();
}
```
> 📌 `TranslateTransform` + `RotateTransform` mueven y giran **el papel**, no el dibujo.
> Así un solo cohete sirve vertical y horizontal.

**Bloque 1 — patas de aterrizaje:** dos líneas inclinadas + dos rectangulitos como pies.

**Bloque 2 — la tobera:** un trapecio con `FillPolygon` (4 puntos, más ancho abajo).

**Bloque 3 — la llama (el truco del fuego):**
```cpp
float flicker = Math::Sin(faseLlama)*4 + Azar::entreF(0, 3);   // tiembla
// triángulo naranja grande, y encima uno amarillo más corto
fuego[2] = PointF(0, 26 + flicker);   // la punta cambia de largo cada cuadro
```
Dos triángulos (naranja + amarillo) cuya punta cambia de longitud cada cuadro con seno
+ azar = fuego vivo. En el juego además se sueltan **partículas** que caen y se apagan.

**Bloque 4 — el cuerpo sin costuras:**
```cpp
g->FillEllipse(%casco, -16, -112, 32, 44);    // nariz redondeada
g->FillRectangle(%casco, -16, -92, 32, 70);   // tubo del fuselaje
```
Una elipse y un rectángulo **del mismo color** se superponen → el ojo los ve como una
sola cápsula. Encima, `FillPie(..., 180, 180)` pinta **solo la mitad superior** de la
elipse de rojo: esa es la punta.

**Bloque 5 — volumen barato:** un rectángulo negro con alfa 45 pegado al borde derecho
(`FillRectangle(sombra, 7, -90, 9, 66)`) simula la sombra del cilindro. Las aletas son
2 triángulos rojos, la ventana son 3 círculos (aro gris → vidrio azul → reflejo) y los
remaches 4 puntitos grises.

---

## 4. Los planetas (`SistemaSolar::dibujarPlaneta`)

**Bloque 1 — el recorte circular (la clave):**
```cpp
GraphicsPath recorte;
recorte.AddEllipse(cx-r, cy-r, r*2, r*2);
g->SetClip(%recorte);        // desde aquí SOLO se pinta dentro del círculo
... bandas, continentes, manchas ...
g->ResetClip();              // vuelve a la normalidad
```
Gracias al *clip* puedes pintar rectángulos enormes (las bandas de Júpiter) y solo se ve
lo que cae **dentro** del planeta. Sin esto las bandas se saldrían del círculo.

**Bloque 2 — cada planeta, su receta:**
- **Mercurio:** círculo gris + 3 elipses más oscuras (cráteres).
- **Venus:** crema + 2 arcos gruesos semitransparentes (nubes).
- **Tierra:** azul + continentes con `FillClosedCurve` (le das 4-5 puntos y GDI+ los une
  con curvas suaves = forma orgánica) + 2 elipses blancas (nubes).
- **Marte:** rojo + manchas + elipse blanca arriba (casquete polar).
- **Júpiter:** rectángulos horizontales alternando claro/oscuro + elipse roja (la mancha).
- **Urano:** turquesa + anillo fino girado 75°.
- **Neptuno:** azul + tormenta oscura + arco blanco (viento).

**Bloque 3 — los anillos de Saturno (en 2 mitades):**
```cpp
// ANTES del planeta: la mitad de ATRÁS del anillo (arco 180°..360°)
g->TranslateTransform(cx, cy); g->RotateTransform(-18);
g->DrawArc(%anilloTras, -r*1.9f, -r*0.62f, r*3.8f, r*1.24f, 180, 180);
// ... se dibuja el planeta encima ...
// DESPUÉS: la mitad de ADELANTE (arco 0°..180°)
```
Un anillo es una **elipse aplastada girada −18°** dibujada con un lápiz gordo. Se parte
en dos arcos para que el planeta quede "metido dentro" del anillo.

**Bloque 4 — la sombra (efecto 3D):** dentro del clip se pinta **otro círculo negro con
alfa 80 desplazado** hacia abajo-derecha. La media luna que se asoma dentro del planeta
es la zona "de noche". Es el truco de volumen más barato que existe.

**Bloque 5 — la Luna orbitando:**
```cpp
double a = tick * 0.02;                       // ángulo que crece con el tiempo
float lunaX = cx + Math::Cos(a) * r * 1.7f;   // órbita ancha
float lunaY = cy + Math::Sin(a) * r * 0.6f;   // y aplastada (elíptica)
```
Coseno para X y seno para Y = movimiento circular. Con radios distintos la órbita se ve
en perspectiva.

---

## 5. El suelo de cada planeta (`Superficie.cpp`)

**Bloque 1 — paletas:** 3 funciones devuelven colores según el planeta:
`colorCieloArriba/Abajo` (degradado) y `colorSuelo/SueloOscuro`. Cambiar el "clima" de un
planeta = cambiar 4 colores.

**Bloque 2 — decoración propia:** un `if` por planeta: el **sol gigante** de Mercurio son
3 círculos concéntricos con alfa bajando (halo) + 2 círculos sólidos; las **nubes de
Venus** se mueven con el truco del módulo:
```cpp
float nx = ((tick * 4 + i * 2300) % 12400) / 10.0f - 120.0f;
```
(cuando una nube sale por la derecha, el módulo la regresa por la izquierda);
las **montañas de Marte** son UN polígono de 7 puntos; las **estalactitas de Urano**
son triángulos colgando del borde superior.

**Bloque 3 — el suelo:** un rectángulo desde `y=560` hasta abajo + una línea clarita en
el borde (horizonte) + 14 piedras (polígonos de 4 puntos en posiciones aleatorias que se
generan **una sola vez**) + 3 elipses sin relleno (cráteres). En la Tierra el suelo es
una plataforma metálica: líneas inclinadas como paneles y balizas naranjas que parpadean
alternando con `((tick/30) + i) % 2`.

---

## 6. Las figuras especiales

**Estrella de 5 puntas (`Estrella::dibujarForma`):**
```cpp
for (int i = 0; i < 10; i++) {
    double a = (angulo - 90 + i * 36) * Math::PI / 180;     // cada 36°
    float r = (i % 2 == 0) ? radio : radio * 0.45f;          // alterna largo/corto
    puntos[i] = PointF(cx + Math::Cos(a)*r, cy + Math::Sin(a)*r);
}
g->FillPolygon(%pincel, puntos);
```
10 puntos en círculo alternando radio grande/chico = estrella. Sumarle `angulo` la hace girar.

**Corazones del HUD:** 2 círculos + 1 triángulo del mismo color. Llenos si te quedan
vidas, grises si no.

**El jefe final:** 10 triángulos (púas) colocados con coseno/seno alrededor del platillo,
girando con `giro += 1.2` por tick; el **ojo** te sigue: se calcula el vector hacia el
jugador, se normaliza (`dx/largo, dy/largo`) y la pupila se desplaza 5 px en esa dirección.

**Los cristales de Urano:** un rombo de 4 puntos + 4 líneas blancas internas (facetas).
"Encendido" = se vuelve a pintar con el color al 100% + un halo elíptico con alfa detrás.

---

## 7. El menú y el ratón

Cada botón es un `RectangleF`. Al mover el ratón:
```cpp
if (rectBoton(i).Contains(mx, my)) { opcion = i; return true; }  // true → suena el beep
```
y al hacer clic se devuelve la **acción** (`AccionMenu::Iniciar`, etc.) que `Juego`
ejecuta. El resaltado del botón marcado es: relleno más claro + borde cian cuyo alfa
"late" con `Math::Sin(tick * 0.12)` + una flecha (triángulo) al costado.

## 8. El sonido

`Console::Beep(frecuencia, milisegundos)` genera el tono, pero **congela** al que lo
llama; por eso `Sonido.cpp` lo ejecuta en otro hilo:
```cpp
ThreadPool::QueueUserWorkItem(gcnew WaitCallback(&Sonido::trabajador), datos);
```
Cada efecto es una secuencia `{frecuencia, duración, ...}`: subir de tono = "premio"
(880→1175 al seleccionar), bajar = "derrota" (392→330→262). Las 4 notas de Urano son
Do-Mi-Sol-Do (523, 659, 784, 1047 Hz).
