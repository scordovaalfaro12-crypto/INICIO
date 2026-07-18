# Parque Jurásico: Operación Contención 2042

Juego en **C++/CLI con Windows Forms (MyForm)** desarrollado con **Programación
Orientada a Objetos**: encapsulamiento, herencia y **polimorfismo diferenciado
por subclase** (cada especie de dinosaurio tiene su propio algoritmo de
desplazamiento en `mover()`).

## Cómo compilar y ejecutar

1. Requiere **Visual Studio 2022** con la carga de trabajo *Desarrollo para el
   escritorio con C++* y el componente **Compatibilidad con C++/CLI**
   (Instalador de Visual Studio → Componentes individuales → "C++/CLI support").
2. Abrir `ParqueJurasico.sln`.
3. Compilar y ejecutar con **F5** (Debug x64 o x86).
   - Si Visual Studio pide *retarget* del toolset (v143 → el instalado), aceptar.
4. La carpeta `sprites/` debe estar junto al proyecto (ya incluida). El programa
   la busca también junto al ejecutable si se ejecuta el .exe suelto.

## Controles

| Tecla | Acción |
|---|---|
| Flechas o W, A, S, D | Mover al guardabosques |
| Barra espaciadora | Disparar dardo (en la dirección hacia la que mira) |
| P | Liberar a los pteranodones (una sola vez, entre 3 y 5) |

## Estructura de clases (cada una en su .h + .cpp)

| Clase | Archivos | Rol |
|---|---|---|
| `Dinosaurio` | Dinosaurio.h/.cpp | Clase **base abstracta**: `mover()` y `getTipo()` virtuales puros, `recibirImpacto()` virtual |
| `Velocirraptor` | Velocirraptor.h/.cpp | Hereda de Dinosaurio. Movimiento **horizontal o vertical** aleatorio, velocidad 2–4, rebota en bordes |
| `Pteranodonte` | Pteranodonte.h/.cpp | Hereda de Dinosaurio. Movimiento **diagonal**, velocidad 5–6 (mayor que el raptor), rebota en bordes |
| `DinosaurioAlfa` | DinosaurioAlfa.h/.cpp | Hereda de Dinosaurio. El más veloz (8), horizontal aleatorio, **2 impactos** para capturarlo, sobreescribe `recibirImpacto()` |
| `Guardabosques` | Guardabosques.h/.cpp | Jugador: sprite animado 4 direcciones, 3 vidas, inventario de dardos |
| `Dardo` | Dardo.h/.cpp | Proyectil en 4 direcciones; si sale de la zona de juego → −1 vida |
| `Caja` | Caja.h/.cpp | Cuadrado rojo; al recogerla +1 dardo |
| `Helicoptero` | Helicoptero.h/.cpp | Apoyo aéreo; al tocarlo +1 vida y +3 dardos (colisión pedida en la rúbrica) |
| `Controlador` | Controlador.h/.cpp | Administra el **buffer** (BufferedGraphics), el **ciclo de animación**, las **teclas**, las colisiones, los estados y **RESULTADOS.TXT** |
| `MyForm` | MyForm.h/.cpp | Formulario: Timer, carga de sprites, delega todo al Controlador |

## Cumplimiento del enunciado / rúbrica

- **Interfaz**: HUD permanente con tiempo (s), capturados, dardos y vidas.
  Pantalla dividida: zona de juego (izquierda) y zona de capturados (derecha),
  donde los atrapados se desplazan muy lentamente.
- **Guardabosques**: flechas y WASD, sprite 4 direcciones, 3 vidas, dispara con
  espacio hacia donde mira.
- **Velocirraptores**: 6–9 al iniciar, posición y velocidad aleatorias,
  desplazamiento horizontal o vertical con rebote.
- **Pteranodones**: tecla P, 3–5, más veloces, trayectoria diagonal con rebote.
- **Cajas**: entre N y 2N (N = dinosaurios generados), cuadrados rojos creados
  al iniciar la animación; al colisionar desaparecen y +1 dardo.
- **Disparos**: consumen 1 dardo; sin dardos → *"No dispone de dardos
  tranquilizantes."*; impacto → captura y pasa a la zona de capturados; fallo
  (sale de la ventana de juego) → el dardo desaparece y −1 vida.
- **Dinosaurio Alfa**: aparece al capturar el 70% del total generado, el más
  veloz, horizontal aleatorio con rebote, requiere 2 impactos, −1 vida por
  colisión. Victoria → **"PARQUE ASEGURADO"**; 0 vidas → **"LOS DINOSAURIOS
  ESCAPARON"**.
- **RESULTADOS.TXT** (modo texto con `std::ofstream`): tiempo total, capturados
  por especie, dardos recogidos/disparados, aciertos/fallos y vidas restantes.
- **Buffer** (`BufferedGraphics`) para eliminar el parpadeo.
- **Fondo temático** del parque (selva dibujada con GDI+).
- **Liberación de memoria**: todos los `new` se liberan en los destructores.

## Decisiones tomadas (puntos que el enunciado deja abiertos)

- **Helicóptero**: la rúbrica pide la colisión Guardabosques–helicóptero con
  "vida y dardos"; se implementó como apoyo: +1 vida (máx. 3) y +3 dardos, y el
  helicóptero se retira ~12 s antes de volver a patrullar.
- **Dardos iniciales**: 0 (se consiguen con las cajas y el helicóptero).
- **Tecla P**: genera los pteranodones una sola vez (con sus cajas adicionales
  para mantener la proporción N–2N).
- Si alguna dirección de sprite se ve invertida en tu monitor, basta ajustar el
  número de fila en `actualizarFila()` de la clase correspondiente.
