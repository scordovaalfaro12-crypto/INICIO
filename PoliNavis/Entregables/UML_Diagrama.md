# Diagrama de Clases UML — PoliNavis: Viaje Interestelar

Aplicación de los 4 principios de POO:
- **Abstracción:** `Entidad`, `Planeta` y `Obstaculo` son clases **abstractas** con métodos virtuales puros.
- **Encapsulamiento:** los atributos son `protected/private`; se acceden por `get/set`.
- **Herencia:** toda figura del juego hereda de `Entidad`.
- **Polimorfismo:** `Juego` recorre listas de `Planeta^` y `Obstaculo^` llamando `mover()`/`dibujar()` sin conocer el tipo concreto.

---

## 1. Código PlantUML
> Pégalo en https://www.plantuml.com/plantuml para ver el diagrama renderizado.

```plantuml
@startuml PoliNavis
skinparam classAttributeIconSize 0
skinparam linetype ortho

abstract class Entidad {
  # x, y : float
  # dx, dy : float
  + {abstract} mover() : void
  + {abstract} dibujar(g) : void
  + area() : RectangleF
  + getX() / getY() : float
  + setPos() / setVel() : void
}

class Nave {
  - vidas : int
  - energia : int
  - invulnerable : int
  - angulo : float
  + acelerar(ax, ay) : void
  + reiniciarEn(px, py) : void
  + perderVida() : void
  + gastarEnergia() / recargarEnergia()
  + getVidas() / getEnergia() : int
}

class Sol {
  - radio : float
  - giro : float
}

abstract class Planeta {
  # solX, solY : float
  # radioOrbita, anguloOrbita, velOrbita : float
  # rotacion, velRotacion : float
  # radio : float
  # nombre, dato : String
  + {abstract} dibujarSuperficie(g, r, rot)
  + dibujarExtra(g, r)
  + acelerar() : void
  + getNombre() / getDato() : String
}

class Mercurio
class Venus
class Tierra
class Marte
class Jupiter
class Saturno
class Urano
class Neptuno

abstract class Obstaculo {
  # radio : float
  # giro, velGiro : float
  + fueraDePantalla() : bool
}

class Cometa
class Asteroide
class FiguraKhaos {
  - lados : int
  - caos : int
  + mover() : void  '' rebote + rumbo aleatorio
}

class Estacion {
  - tipo : int
  - fase : float
}

class Juego <<controladora>> {
  - estado : String
  - nivel, colisiones, ticksNivel : int
  + actualizar() : void
  + dibujar(g) : void
  + teclaAbajo() / teclaArriba()
  + clic() / ratonMovido()
  - iniciarNivel(n) : void
  - revisarColisiones() : void
  - spawnObstaculoNivel1() : void
}

class Menu {
  - opcion : int
  + dibujarPrincipal() / dibujarInstrucciones() / dibujarCreadores()
  + clicPrincipal() : AccionMenu
}

class MyForm {
  - temporizador : Timer
  - juego : Juego
  + alTick() / alPintar()
  + alTeclaAbajo() / alClic()
}

class Entrada
class Config
class Azar
class Sonido
class Estilos
class Particula
class Fondo
class Recursos

Entidad <|-- Nave
Entidad <|-- Sol
Entidad <|-- Planeta
Entidad <|-- Obstaculo
Entidad <|-- Estacion

Planeta <|-- Mercurio
Planeta <|-- Venus
Planeta <|-- Tierra
Planeta <|-- Marte
Planeta <|-- Jupiter
Planeta <|-- Saturno
Planeta <|-- Urano
Planeta <|-- Neptuno

Obstaculo <|-- Cometa
Obstaculo <|-- Asteroide
Obstaculo <|-- FiguraKhaos

Juego "1" *-- "1" Nave
Juego "1" *-- "1" Sol
Juego "1" *-- "0..8" Planeta
Juego "1" *-- "0..*" Obstaculo
Juego "1" *-- "1" Estacion
Juego "1" *-- "1" Menu
MyForm "1" --> "1" Juego : controla
MyForm ..> Recursos
Juego ..> Entrada
Juego ..> Sonido
Juego ..> Estilos
Juego ..> Fondo
@enduml
```

---

## 2. Versión en texto (por si no puedes renderizar)

```
                         ┌───────────────────────────┐
                         │      Entidad (abstracta)   │
                         │  # x, y, dx, dy            │
                         │  + mover()  *abstracto*    │
                         │  + dibujar(g) *abstracto*  │
                         │  + area()                  │
                         └────────────┬──────────────┘
        ┌──────────────┬──────────────┼───────────────┬───────────────┐
        ▼              ▼              ▼               ▼               ▼
     ┌──────┐      ┌──────┐   ┌───────────────┐  ┌──────────────┐  ┌──────────┐
     │ Nave │      │ Sol  │   │   Planeta     │  │  Obstaculo   │  │ Estacion │
     └──────┘      └──────┘   │ (abstracta)   │  │ (abstracta)  │  └──────────┘
                              │ órbita+rotación│  │ trayectoria │
                              └──────┬────────┘  └──────┬───────┘
              ┌──────┬──────┬────────┼──────┬──────┐    ├── Cometa
              ▼      ▼      ▼        ▼      ▼      ▼    ├── Asteroide
          Mercurio Venus Tierra  Júpiter Saturno ...   └── FiguraKhaos (Nivel 2)
                          (detalle) (anillos)

   ┌─────────────────────────── Juego (CLASE CONTROLADORA) ───────────────────────────┐
   │  estado, nivel, colisiones, ticksNivel                                            │
   │  posee: 1 Nave, 1 Sol, 0..8 Planeta, 0..* Obstaculo, 1 Estacion, 1 Menu           │
   │  + actualizar()  + dibujar()  + revisarColisiones()  + iniciarNivel()             │
   └───────────────────────────────────────────────────────────────────────────────────┘
                                        ▲
                                        │ controla
                                   ┌─────────┐
                                   │ MyForm  │  (ventana única + Timer 60 fps)
                                   └─────────┘

   Clases de apoyo: Entrada, Config, Azar, Sonido, Estilos, Particula, Fondo, Recursos.
```

---

## 3. Dónde se ve cada principio en el código

| Principio | Evidencia en el proyecto |
|-----------|--------------------------|
| **Abstracción** | `Entidad`, `Planeta`, `Obstaculo` con `= 0` (virtual puro) |
| **Encapsulamiento** | `x,y` son `protected`; se usan `getX()/setPos()`; nadie los toca directo desde fuera |
| **Herencia** | `Nave`, `Sol`, `Planeta`, `Obstaculo`, `Estacion` heredan de `Entidad`; los 8 planetas de `Planeta` |
| **Polimorfismo** | `for each (Planeta^ p in planetas) p->mover();` y `o->dibujar(g)` llaman a la versión concreta correcta |
| **Clase controladora** | `Juego` integra y coordina todas las clases (rúbrica: 3 pts) |
