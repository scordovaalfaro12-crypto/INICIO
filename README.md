# 🚀 Odisea Espacial

Juego de exploración espacial hecho en **C++/CLI (Proyecto vacío de CLR .NET Framework)** con **Windows Forms (MyForm)**.
Todo el arte está dibujado **100% con figuras geométricas de GDI+** (círculos, rectángulos, polígonos, arcos…): **no se usa ni un solo sprite ni imagen**.

Todo el juego transcurre **en una sola ventana** de 1000×700: los menús, el mapa, las cinemáticas y los 8 minijuegos son pantallas dibujadas dentro del mismo `MyForm`.

---

## 🎮 ¿Qué incluye?

- **Menú principal interactivo** (ratón + teclado, con sonido al cambiar de opción):
  - **INICIAR** → va al mapa del sistema solar.
  - **PERSONAJES** → 4 astronautas con diseños distintos y su descripción; haz clic para elegir el tuyo.
  - **CONTROLES** → teclas dibujadas con su explicación.
  - **CRÉDITOS** → el equipo y el rol de cada uno.
  - **SALIR** → cierra el juego.
- **Mapa del sistema solar** con el Sol y los **8 planetas clicables** (cada uno dibujado con sus detalles: anillos de Saturno, mancha roja de Júpiter, continentes de la Tierra con su Luna orbitando…).
- **Cinemática de aterrizaje** en cada planeta: el cohete desciende frenando con la llama encendida, levanta polvo al tocar el suelo, se abre la escotilla, baja la escalera y el astronauta sale caminando. Al terminar la misión ocurre el **despegue** inverso.
- **Un minijuego diferente en cada planeta**:

| Planeta | Misión | Mecánica |
|---|---|---|
| Mercurio | Lluvia de fuego | Esquiva meteoritos 30 s (las sombras en el suelo te avisan) |
| Venus | Atrapa estrellas | Recoge 15 estrellas, evita la lluvia ácida |
| Tierra | Limpieza orbital | Dispara a la basura espacial estilo *invaders* |
| Marte | Caza de artefactos | Vuela con jetpack y recoge 6 reliquias entre rocas |
| Júpiter | Tormenta eléctrica | Sobrevive a los rayos con señales de aviso y viento |
| Saturno | Carrera de anillos | Pilota la nave por los huecos; el casco se daña y se repara |
| Urano | Cristales de memoria | "Simón dice" con 4 cristales musicales (solo ratón) |
| Neptuno | **EL JEFE FINAL** | Jefe con rayos y asteroides (se desbloquea al completar los otros 7) |

- **Sonido** en todo el juego con tonos generados (`Console::Beep` en un hilo aparte): menús, disparos, daño, ítems, victoria, derrota, despegue y las 4 notas musicales de Urano.
- **Puntaje acumulado**, vidas con corazones, barra de vida del jefe, estado de la nave, misiones completadas con insignia ✓ y pantalla final de celebración con fuegos artificiales.

---

## 🧩 Cumplimiento del UML

| Clase del UML | Archivo | Atributos / Métodos |
|---|---|---|
| *Clase abstracta base* | `EntidadEspacial.h/.cpp` | `x`, `y` · `mover()`, `dibujar()` |
| **Juego** | `Juego.h/.cpp` | `nivel`, `estado` · `iniciar()`, `actualizar()`, `finalizar()` |
| *Enemigo (abstracta)* | `Enemigo.h/.cpp` | `vida` · `atacar()`, `mover()` |
| *Item (abstracta)* | `Item.h/.cpp` | `valor` · `aplicarEfecto()` |
| **Astronauta** | `Astronauta.h/.cpp` | `nombre`, `puntaje` · `recolectarItem()`, `disparar()` |
| **Nave** | `Nave.h/.cpp` | `integridad`, `velocidad` · `recibirDanio(cantidad)`, `reparar(cantidad)`, `mostrarEstado()` |
| **Asteroide** | `Asteroide.h/.cpp` | `atacar()`, `mover()` |
| **BasuraEspacial** | `BasuraEspacial.h/.cpp` | `atacar()`, `mover()` |
| **JefeFinal** | `JefeFinal.h/.cpp` | `atacar()`, `dispararRayo()`, `lanzarAsteroide()` |
| **Estrella** | `Estrella.h/.cpp` | `aplicarEfecto()` |
| **Artefacto** | `Artefacto.h/.cpp` | `aplicarEfecto()` |

Las jerarquías de herencia son las del diagrama: `Asteroide`, `BasuraEspacial` y `JefeFinal` heredan de `Enemigo`; `Estrella` y `Artefacto` heredan de `Item`; `Enemigo`, `Item`, `Astronauta` y `Nave` heredan de la entidad base. El `Astronauta` posee la relación con la `Nave` y los ítems aplican su efecto sobre él (`recolectarItem` → `aplicarEfecto`).

Clases de apoyo (pantallas y motor): `MyForm`, `MenuJuego`, `SistemaSolar`, `Superficie`, `Aterrizaje`, `Minijuego` (base abstracta) + `MinijuegoMercurio…Neptuno`, `Proyectil`, `Particula`, `Fondo`, `Sonido`, `Estilos`, `Entrada`, `Azar`, `Config`.

---

## 🛠️ Cómo abrir y compilar

1. Abre **`JuegoEspacial/JuegoEspacial.sln`** con **Visual Studio 2022** (carga de trabajo *Desarrollo de escritorio con C++* + *Compatibilidad con C++/CLI*).
2. Elige la configuración **Debug | x64**.
3. Compila y ejecuta con **F5**.

> Si tu Visual Studio trae otra versión del toolset, acepta el cuadro de **"Volver a destinar proyectos"** (Retarget) y listo.
> Proyecto basado en la plantilla *"Proyecto vacío de CLR (.NET Framework 4.7.2)"*, igual que los ejemplos de clase.

## ⌨️ Controles

- **Menús**: ratón (hover + clic) o `↑` `↓` y `ENTER`.
- **Caminar**: `A` / `D` o `←` / `→`.
- **Volar (Marte)**: `W` `A` `S` `D` o flechas. **Nave (Saturno)**: `↑` / `↓`.
- **Disparar**: `ESPACIO`.
- **Urano**: solo el ratón (clic en los cristales).
- **ESC**: volver / abandonar misión. **R**: reintentar una misión fallida.

## 👨‍🚀 Créditos

| Integrante | Rol |
|---|---|
| **Bryan Cordova** | Programación y lógica del juego (Developer) |
| **Lenin Palomino** | Diseño visual con figuras geométricas (Diseñador) |
| **Gustavo Arimana** | Jugabilidad y pruebas (QA Tester) |
| **Mustafa Altuntas** | Sonido, UML y documentación (Productor) |
