# 🚀 PoliNavis: Viaje Interestelar

Juego educativo en **C++/CLI (Windows Forms)** para la **PC1 – Game Jam** del curso
Algoritmos de Programación. Una nave debe viajar del **Punto A** al **Punto B**
esquivando cometas y asteroides, mientras el jugador aprende los nombres de los planetas
y sus movimientos de **rotación** y **traslación**.

Todo se dibuja con **rectángulos, líneas y polígonos** (GDI+). **No se usan sprites**: lo
único que puede ser imagen es el **fondo decorativo** (permitido por la rúbrica).

**Equipo:** Bryan Cordova · Lenin Palomino · Gustavo Arimana · Mustafa Altuntas · Biary Tineo

---

## 🎮 Contenido

- **Menú principal** (una sola ventana): NIVEL 1, NIVEL 2, INSTRUCCIONES, CREADORES, SALIR.
- **Nivel 1 – Sistema Solar:** Sol central + **8 planetas** (incluidos **Tierra** y
  **Saturno**) con **traslación** (órbita) y **rotación** (giro de su superficie). La nave
  va del Punto A (abajo-izquierda) a la **Estación Espacial** (Punto B). Cometas y
  asteroides aparecen a intervalos con **trayectorias y velocidades aleatorias**, siempre
  más rápidos que la nave. Al chocar, la nave **regresa al Punto A**, pierde una vida y
  sube el contador de colisiones. Clic en un planeta → **gira/orbita más rápido**.
- **Nivel 2 – Galaxia Nebulosa del Khaos:** un agujero negro lleva la nave a una galaxia
  de **figuras geométricas caóticas** (polígonos de distinta cantidad de lados, paleta
  neón totalmente distinta) que se mueven de forma **impredecible**. La nave va del
  Lugar 1 al **portal de salida** (Lugar 2).
- **Panel de control:** tiempo (mm:ss), colisiones, **energía** y **vidas**, en tiempo real.
- **Leyenda de planetas:** panel lateral con los nombres + dato educativo (componente
  educativo principal).
- **Mensajes:** "NIVEL 1 COMPLETO – Recargando combustible…" y "MISIÓN CUMPLIDA – Has
  escapado de la Galaxia Nebulosa del Khaos!", además de derrota.

## ⌨️ Controles

- **Mover la nave:** Flechas o `W A S D`.
- **Acelerar un planeta (Nivel 1):** clic sobre el planeta.
- **Volver al menú / pausar:** `ESC`. **Reintentar (tras derrota):** `R`.

## 🛠️ Cómo compilar

1. Abre **`PoliNavis/PoliNavis.sln`** con **Visual Studio 2022** (carga *Desarrollo de
   escritorio con C++* + *Compatibilidad con C++/CLI*).
2. Configuración **Debug | x64**.
3. **F5** para compilar y ejecutar.

> El proyecto ya incluye `/utf-8` (para que los acentos se vean bien) y la carpeta
> `img/` con los fondos. Si faltara `img/`, el juego dibuja un cielo de estrellas como
> respaldo y **no se cae**.

## ✅ Cumplimiento de la rúbrica (20 pts)

| Criterio | Dónde está |
|---|---|
| Diagrama de clases (POO) | `Entregables/UML_Diagrama.md` — abstracción/encapsulamiento/herencia/polimorfismo |
| Nave, planetas, cometas, asteroides con figuras + rotación/traslación | `Nave`, `Planeta`+8 subclases, `Cometa`, `Asteroide` |
| Ambos niveles + menú con instrucciones, acceso independiente | `Juego` (estados), `Menu` |
| Colisiones (regreso al origen) + panel de control + leyenda | `Juego::revisarColisiones`, `dibujarPanel`, `dibujarLeyenda` |
| Clase **controladora** que integra todo | `Juego` |
| Mensajes de éxito / derrota | `Juego::dibujarMensaje` |
| Creatividad y experiencia de usuario | fondos, sonido, animaciones |
| Entregables completos | `Entregables/` (fuente, UML, bitácora, plan) |

## 📂 Estructura

```
PoliNavis/
├── PoliNavis.sln
├── PoliNavis/                 (código fuente .h/.cpp + img/)
│   ├── Entidad.*              (clase base abstracta)
│   ├── Nave.*  Sol.*  Estacion.*
│   ├── Planeta.*  Planetas.*  (8 planetas)
│   ├── Obstaculo.*  Cometa.*  Asteroide.*  FiguraKhaos.*
│   ├── Juego.*                (CLASE CONTROLADORA)
│   ├── Menu.*  MyForm.*       (ventana única)
│   ├── (motor) Config Azar Entrada Sonido Estilos Particula Fondo Recursos
│   └── img/  espacio.png  menu.png  nebulosa.png
└── Entregables/
    ├── PC1_FUENTE.TXT              (código fuente .h/.cpp en texto)
    ├── UML_Diagrama.png            (diagrama de clases — imagen)
    ├── UML_Diagrama.md             (fuente PlantUML + explicación)
    ├── Bitacora_IA.docx            (bitácora de IA — Word, Anexo 1)
    └── Plan_de_Actividades.docx    (plan de actividades — Word, Anexo 2)
```
