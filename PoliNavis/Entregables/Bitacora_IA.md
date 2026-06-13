# Bitácora de Uso de IA — Anexo 1
## PC1: Game Jam — "PoliNavis: Viaje Interestelar"
**Curso:** Algoritmos de Programación (1ACC0265)
**Equipo:** Bryan Cordova · Lenin Palomino · Gustavo Arimana · Mustafa Altuntas · Biary Tineo
**Herramienta de IA utilizada:** Claude (asistente de programación), usado dentro de Visual Studio / chat.

> Esta bitácora documenta **cómo** y **para qué** usamos la IA, qué prompts dimos, qué
> resultó y **cómo verificamos/modificamos** cada respuesta. Cada integrante puede
> sustentar oralmente los registros que le corresponden.

---

## 1. Resumen por actividad

| N° | Fecha | Integrante | Propósito / Actividad | Herramienta | Resultado | Validación / Modificación |
|----|-------|-----------|------------------------|-------------|-----------|----------------------------|
| 1 | 13/06 | Lenin | Diseñar la jerarquía de clases (POO) a partir de la rúbrica | Claude | Propuesta de clase base abstracta `Entidad` + herencias | Revisamos que cubriera abstracción, herencia y polimorfismo; ajustamos nombres al español |
| 2 | 13/06 | Bryan | Definir la **clase controladora** `Juego` que integra todo | Claude | Esqueleto con máquina de estados (MENU, NIVEL1, NIVEL2, mensajes) | Comprobamos que centraliza Nave, Planetas, Obstáculos y Estación |
| 3 | 13/06 | Gustavo | Dibujar la **nave** solo con polígonos/líneas (sin sprites) | Claude | Función `Nave::dibujar` con triángulos, rombo y cabina | Probamos en pantalla; giramos la nave hacia su dirección con `Atan2` |
| 4 | 13/06 | Lenin | **Planetas** con rotación (eje) y traslación (órbita) | Claude | Clase `Planeta` (órbita por seno/coseno) + 8 subclases | Verificamos que Tierra y Saturno fueran distintos; añadimos anillos |
| 5 | 13/06 | Lenin | Hacer **visible** la rotación de cada planeta | Claude | Helper `marca()` que mueve manchas por el disco según el ángulo | Ajustamos para que solo se vean las manchas de la cara frontal |
| 6 | 13/06 | Mustafa | **Cometas y asteroides** con trayectoria y velocidad aleatorias | Claude | `Cometa` (cola) y `Asteroide` (polígono irregular) + spawner | Confirmamos que van más rápido que la nave (rúbrica) |
| 7 | 13/06 | Bryan | Lógica de **colisión → regresar al Punto A** y contador | Claude | `revisarColisiones()`: resta vida, +1 colisión, reinicia nave | Probamos límites; añadimos 2 s de invulnerabilidad tras reiniciar |
| 8 | 13/06 | Gustavo | **Panel de control** (tiempo, colisiones, energía, vidas) | Claude | HUD superior con barra de energía y corazones | Verificamos que el tiempo avance en mm:ss en tiempo real |
| 9 | 13/06 | Gustavo | **Leyenda de planetas** (componente educativo) | Claude | Panel lateral con nombres + dato al hacer clic | Revisamos que los datos de rotación/traslación fueran correctos |
| 10 | 13/06 | Mustafa | **Nivel 2** caótico (galaxia del Khaos) con figuras geométricas | Claude | `FiguraKhaos`: polígonos de N lados, rebote y rumbo aleatorio | Cambiamos la paleta a neón para diferenciarlo del Nivel 1 |
| 11 | 13/06 | Biary | **Menú** con Niveles, Instrucciones, Creadores y Salir | Claude | Clase `Menu` + pantallas con texto | Redactamos nosotros el contenido educativo de Instrucciones |
| 12 | 13/06 | Biary | **Estación espacial** (Punto B) y **portal** (Lugar 2) | Claude | `Estacion` con anillo, paneles y luces pulsantes | Ajustamos tamaños y el área de llegada |
| 13 | 13/06 | Bryan | **Mensajes** de éxito y derrota | Claude | "NIVEL 1 COMPLETO…" y "MISIÓN CUMPLIDA…" con el tiempo | Verificamos los textos exactos que pide la rúbrica |
| 14 | 13/06 | Mustafa | **Sonido** sin archivos (tonos) en hilo aparte | Claude | `Sonido` con `Console::Beep` en ThreadPool | Comprobamos que el juego no se congela al sonar |
| 15 | 13/06 | Gustavo | Permitir **imagen de fondo** decorativa (permitido por rúbrica) | Claude | `Recursos` carga espacio.png/menu.png con respaldo de estrellas | Confirmamos que si falta la imagen, no se cae el programa |
| 16 | 13/06 | Lenin | Corregir **acentos** que salían como "Ã©" | Claude | Agregar `/utf-8` al proyecto en las 4 configuraciones | Recompilamos y verificamos "CRÉDITOS", "MISIÓN", etc. |
| 17 | 14/06 | Bryan | Diagrama **UML** de clases | Claude | UML en PlantUML + versión en texto | Lo contrastamos con el código real clase por clase |
| 18 | 14/06 | Todos | **Pruebas integrales** y ajuste de dificultad | Claude | Velocidades de spawn y tamaños equilibrados | Jugamos ambos niveles de inicio a fin sin errores |

---

## 2. Detalle de prompts representativos

**Prompt (actividad 1 — jerarquía POO):**
> "Según esta rúbrica, necesitamos un juego en C++/CLI donde una nave va del Punto A al
> Punto B esquivando cometas y asteroides, con planetas que rotan y orbitan. Propón una
> jerarquía de clases con una clase base abstracta y herencia, en español."

*Resultado:* la IA propuso `Entidad` (abstracta, con `x,y,dx,dy`, `mover()` y `dibujar()`
virtuales puros) y de ahí `Nave`, `Sol`, `Planeta`, `Obstaculo` y `Estacion`.
*Qué hicimos nosotros:* validamos que `Planeta` y `Obstaculo` también fueran abstractas
(polimorfismo) y renombramos métodos al español para entenderlos en la defensa.

**Prompt (actividad 5 — rotación visible):**
> "La rotación de los planetas no se nota. ¿Cómo hago que se vea girar usando solo
> figuras, sin sprites?"

*Resultado:* el helper `marca()` ubica manchas según una "longitud" y un ángulo `rot`;
al aumentar `rot` las manchas cruzan el disco y solo se ven las del frente
(`cos(ang) > 0`). *Qué hicimos:* probamos valores hasta que el giro se viera fluido.

**Prompt (actividad 16 — acentos):**
> "Los textos salen como CRÃ‰DITOS en vez de CRÉDITOS. ¿Por qué y cómo se arregla?"

*Resultado:* el compilador leía el archivo como ANSI; la solución fue añadir la opción
`/utf-8`. *Qué hicimos:* lo aplicamos en Propiedades → C/C++ → Línea de comandos en las
4 configuraciones y recompilamos.

---

## 3. Reflexión del equipo

- La IA **aceleró** el diseño y el dibujo con figuras, pero **las decisiones de diseño,
  la dificultad, los textos educativos y las pruebas las hicimos nosotros**.
- Entendemos cada clase: qué hereda, qué método es virtual y por qué, cómo la clase
  controladora `Juego` integra todo, y cómo funcionan la traslación (seno/coseno) y la
  rotación (desplazamiento de manchas).
- Usamos la IA de forma **reflexiva**: cuando una respuesta no encajaba con la rúbrica
  (por ejemplo, usar sprites de planetas), la descartamos y dibujamos todo con figuras.
