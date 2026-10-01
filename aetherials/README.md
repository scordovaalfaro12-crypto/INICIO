# Aetherials — línea de Vulcanid

![Línea evolutiva](linea_evolutiva.png)

| Forma | Carpeta | Idea |
|---|---|---|
| 1. Vulcanid | (modelo original) | Lagarto de roca con placas abovedadas y grietas de magma |
| 2. **Basaltor** | [`basaltor/`](basaltor/) | El magma se enfría en columnas de basalto con una grieta de lava en el lomo y un mazo en la cola |
| 3. **Obsidrax** | [`obsidrax/`](obsidrax/) | Erupción: la lava se vuelve obsidiana. Melena de cristales, corazón de magma, ojos de lava y orbe en la cola |

Cada carpeta trae renders, GIFs (caminando y girando), archivos de Blender, un GLB, los FBX para
Roblox (con esqueleto, piezas sueltas y animaciones de reposo, rugido y caminata) y un script de
configuración para Roblox Studio.

El código que genera los dos modelos está en [`fuente/`](fuente/) (Python + Blender 5.0, `pip install bpy==5.0.1 pillow`).
