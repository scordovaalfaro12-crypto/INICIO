# 25 · Minijuego — la clase base de las 8 misiones

**Archivos:** `Minijuego.h`, `Minijuego.cpp` (abstracta)

## ¿Para qué sirve?

Define el **molde** que cumplen los 8 minijuegos. `Juego` trabaja contra este molde sin
saber cuál misión está activa (polimorfismo, igual que el UML hace con EntidadEspacial).

```cpp
public ref class Minijuego abstract {
public:
    virtual void iniciar() = 0;                      // colocar todo en su sitio
    virtual void actualizar(Entrada^ entrada) = 0;   // 1 tick de juego
    virtual void dibujar(Graphics^ g) = 0;           // pintar la escena
    virtual void clic(int mx, int my);               // solo lo usa Urano
    virtual String^ progreso() = 0;                  // texto del objetivo p/ el HUD
    virtual void dibujarHUD(Graphics^ g);            // barra superior (redefinible)
    bool jugando();  bool ganado();  bool perdido();
};
```

## Atributos compartidos

| Atributo | Para qué sirve |
|---|---|
| `jugador` | El astronauta (referencia al MISMO objeto de toda la partida) |
| `estadoInterno` | 0 = jugando, 1 = ganado, 2 = perdido |
| `ticks` | Reloj local del minijuego (animaciones y dificultad creciente) |
| `titulo` / `instrucciones` | Los textos del cartel de inicio (cada hijo los pasa en su constructor) |
| `puntajeLocal` | Los puntos ganados EN ESTE intento (para el cartel de resultado) |
| `planeta` | Índice 0..7 (para el nombre en el HUD y el fondo) |
| `usaVidas` | Si el HUD muestra corazones (Saturno lo apaga: ahí la "vida" es el casco de la nave) |
| `particulas` | Lista de efectos del minijuego |

## Los métodos "de fábrica" (protected, los usan los hijos)

```cpp
void Minijuego::ganar() {
    if (estadoInterno != 0) return;     // solo se puede terminar UNA vez
    estadoInterno = 1;
    Sonido::victoria();
}
void Minijuego::sumar(int cantidad) {   // puntos al intento Y al jugador global
    puntajeLocal += cantidad;
    jugador->sumarPuntaje(cantidad);
}
void Minijuego::tictac() {              // primera línea de todo actualizar()
    ticks++;
    Particulas::actualizar(particulas);
}
```
El guardia `if (estadoInterno != 0) return;` evita bugs del tipo "gané y perdí en el
mismo tick" (por ejemplo, cruzar el último anillo y chocar a la vez).

## El HUD común (`dibujarHUD`)

Barra superior translúcida con línea cian + 3 zonas:
- Izquierda: `"MERCURIO · Lluvia de fuego"` (nombre del planeta + misión, fuente Consolas).
- Centro: el texto de `progreso()` — **cada hijo lo redefine**: "Sobrevive: 18 s",
  "Estrellas: 7/15", "Vida del jefe: 14/24"...
- Derecha: puntos del intento + los **corazones**.

**Los corazones** (puro dibujo geométrico):
```cpp
Color color = (i < vidas) ? rojo : gris;
g->FillEllipse(... dos círculos arriba ...);     // los dos lóbulos
g->FillPolygon(... triángulo abajo ...);         // la punta
```
2 círculos + 1 triángulo del mismo color = un corazón. Llenos = vidas restantes.

**Redefinición del HUD** (otra vez polimorfismo):
- `MinijuegoSaturno::dibujarHUD` llama al de la base **y además** `nave->mostrarEstado(g)`.
- `MinijuegoNeptuno::dibujarHUD` añade `jefe->dibujarBarraVida(g)`.
```cpp
void MinijuegoSaturno::dibujarHUD(Graphics^ g) {
    Minijuego::dibujarHUD(g);     // lo común
    nave->mostrarEstado(g);       // lo suyo
}
```

## El ciclo de vida de un minijuego (cómo lo usa Juego)

```
crearMinijuego(planeta) → iniciar() → [subMini=0: cartel de instrucciones]
→ ENTER → [subMini=1: actualizar(entrada) cada tick]
→ ganar()/perder() → [subMini=2: cartel de resultado, las partículas siguen vivas]
→ ENTER → DESPEGUE   (o R → se crea OTRO minijuego igual y se reintenta)
```
Detalle fino: en el resultado (`subMini = 2`) `Juego` **sigue llamando** a
`actualizar`, pero como `estadoInterno != 0` los hijos salen temprano
(`if (!jugando()) return;`) tras el `tictac()` → la explosión del jefe sigue
animándose detrás del cartel. Bonito y barato.
