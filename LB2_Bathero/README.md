# LB2 – Bathero atrapa villanos con su bumerán (Práctica de Laboratorio Nro. 2)

Curso: **ALGORITMOS (1ACC0265)** · Pregunta 2 (10 puntos) · POO en **C++/CLR (Windows Forms)**.

Simulación donde **Bathero** debe atrapar a dos tipos de villanos lanzando su
**bumerán**, cumpliendo todos los requisitos del enunciado y la rúbrica.

---

## Cómo abrir y ejecutar (Visual Studio 2019 o superior)

1. Abrir **`LB2_Bathero.sln`** con Visual Studio.
2. Seleccionar la configuración **Debug | x86** (o x64) en la barra superior.
3. Pulsar **Ctrl + F5** (Iniciar sin depurar) o **F5**.
   - Al compilar, un *post-build* copia automáticamente la carpeta `Recursos`
     (sprites + fondo) junto al ejecutable, por lo que las imágenes se cargan
     solas. Si por algún motivo no se vieran, el programa busca los recursos en
     varias rutas alternativas (incluida `LB2_Bathero\Recursos`).

> Requiere el componente **"Compatibilidad con C++/CLI"** y **".NET Framework 4.7.2"**
> en el instalador de Visual Studio (Herramientas → Obtener herramientas y características).
>
> El proyecto usa el conjunto de herramientas **v142 (VS 2019)**. Si abres con
> **VS 2022** y aparece un aviso de *retargeting*, haz clic derecho en el proyecto →
> **"Volver a seleccionar como destino los proyectos"** (Retarget) y acepta; o cambia
> *Propiedades → General → Conjunto de herramientas de plataforma* a la versión instalada.

---

## Controles

| Tecla | Acción |
|-------|--------|
| **Flechas** ← ↑ → ↓ | Mover a Bathero (solo por la mitad **izquierda**) |
| **E** | Aparecen **3 a 5 Enemigo1** (movimiento **horizontal**, velocidad aleatoria) en la mitad **derecha** |
| **F** | Aparecen **2 a 4 Enemigo2** (movimiento **vertical**, velocidad aleatoria) en la mitad **derecha** |
| **Barra espaciadora** | Bathero lanza el **bumerán**; si toca un enemigo lo atrapa (desaparece) y el bumerán **regresa** a su posición inicial |
| **X** | Termina la simulación y muestra el **reporte** de enemigos atrapados |

---

## Estructura del proyecto (Programación Orientada a Objetos)

Cada clase está separada en su **`.h` (declaración)** y **`.cpp` (implementación)**:

```
LB2_Bathero/
├── LB2_Bathero.sln
└── LB2_Bathero/
    ├── MyForm.h / MyForm.cpp        → Formulario + PROGRAMA PRINCIPAL (main)
    ├── Controlador.h / .cpp         → Objeto CONTROLADOR (buffer, animación, teclas, colisiones, reporte)
    ├── Sprite.h / .cpp              → Clase BASE (posición, recorte de spritesheet, colisión)
    ├── Bathero.h / .cpp             → Hereda de Sprite (héroe, movimiento con flechas)
    ├── Enemigo.h / .cpp             → Clase base de los villanos (hereda de Sprite)
    ├── Enemigo1.h / .cpp            → Hereda de Enemigo (movimiento horizontal)
    ├── Enemigo2.h / .cpp            → Hereda de Enemigo (movimiento vertical)
    ├── Bumeran.h / .cpp             → Hereda de Sprite (proyectil que gira y regresa)
    ├── Util.h / .cpp                → Carga de imágenes/recursos
    └── Recursos/
        ├── Bathero.png   (hoja 3×4)
        ├── Enemigos.png  (hoja 6×4: Enemigo1 cols 0–2, Enemigo2 cols 3–5)
        ├── bumeran.png
        └── fondo.png
```

La **herencia** y el **polimorfismo** se aplican con el método virtual
`Actualizar()`, redefinido por cada subclase, y con la lista polimórfica
`List<Enemigo^>` que almacena ambos tipos de villanos.

---

## Cómo cada requisito cumple la RÚBRICA

| Rúbrica | Dónde se cumple |
|---------|-----------------|
| **Definición e implementación de la(s) clase(s)** (2) | `Sprite`, `Bathero`, `Enemigo`, `Enemigo1`, `Enemigo2`, `Bumeran` con sus atributos y métodos. |
| **Desarrollo del objeto Controlador** (2) | `Controlador.h`: administra el **buffer** (`Bitmap` fuera de pantalla), la **animación** (`Actualizar`/`Dibujar`) y el **uso de las teclas**. |
| **Sprites en movimiento** (4) | Bathero (flechas, 4 direcciones), Enemigo1 (horizontal), Enemigo2 (vertical) y el **bumerán** con sus **colisiones** que atrapan enemigos. |
| **Reporte final** (1) | Al pulsar **X**, `Reporte()` muestra en una ventana la **cantidad de enemigos atrapados** (total y por tipo). |
| **Desarrollo del Programa Principal** (1) | `MyForm.cpp`: el objeto se **crea** (`gcnew`) y se **libera** correctamente (`Liberar()` en el destructor). |
| **Buffer / fondo** | Doble buffer manual (sin parpadeo) + `fondo.png` como imagen de fondo. |

---

> Antes de enviar al aula virtual, comprimir la solución como
> `CODESTUDIANTE_LB2.zip` (eliminando las carpetas de compilación `Debug/`,
> `Release/`, `x64/`, `.vs/`, que ya están en el `.gitignore`).
