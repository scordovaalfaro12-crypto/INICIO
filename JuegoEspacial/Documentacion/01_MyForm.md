# 01 · MyForm — la ventana única del juego

**Archivos:** `MyForm.h`, `MyForm.cpp`

## ¿Para qué sirve?

Es el **formulario de Windows Forms** donde ocurre absolutamente todo. El requisito del
proyecto era "una sola ventana, sin abrir otras": por eso `MyForm` no tiene botones, ni
paneles, ni controles — solo un lienzo negro de 1000×700 donde la clase `Juego` pinta
cada pantalla. La ventana es de tamaño fijo (`FixedSingle`, sin botón maximizar) para que
las coordenadas del dibujo siempre coincidan.

## ¿Quién la usa?

`MyForm.cpp` contiene el `main()` del programa:
```cpp
[STAThread]
void main() {
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false);
    Application::Run(gcnew JuegoEspacial::MyForm());   // abre la ventana y arranca el bucle
}
```
`Application::Run` queda en bucle procesando mensajes de Windows hasta que cierras la ventana.

## Atributos

| Atributo | Tipo | Para qué sirve |
|---|---|---|
| `temporizador` | `Timer^` | Dispara `alTick` cada 16 ms (~60 cuadros por segundo) |
| `components` | `IContainer^` | Contenedor estándar del diseñador (lo trae la plantilla) |
| `juego` | `Juego^` | El cerebro del juego; el formulario solo le reenvía eventos |

## El constructor, bloque por bloque

```cpp
InitializeComponent();                       // crea el timer y configura la ventana
this->SetStyle(ControlStyles::AllPaintingInWmPaint
    | ControlStyles::UserPaint
    | ControlStyles::OptimizedDoubleBuffer, true);
this->UpdateStyles();
juego = gcnew Juego();
```
- `AllPaintingInWmPaint + UserPaint`: "yo pinto todo a mano, Windows no borres nada por tu cuenta".
- `OptimizedDoubleBuffer`: cada cuadro se pinta primero en una imagen oculta en memoria
  y se muestra de golpe → **sin parpadeo**. Sin esto verías un molesto "flickering".

Dentro de `InitializeComponent` (estilo del diseñador de VS):
```cpp
this->temporizador->Interval = 16;                 // 16 ms ≈ 60 fps
this->ClientSize = System::Drawing::Size(1000, 700);
this->FormBorderStyle = ...FixedSingle;            // no se puede redimensionar
this->MaximizeBox = false;
this->KeyPreview = true;                           // el form recibe TODAS las teclas
```

## Los 6 eventos (el "cartero" del juego)

`MyForm` no decide nada: recoge los eventos de Windows y se los pasa a `Juego`.

| Evento | Método | Qué hace |
|---|---|---|
| `Tick` del timer | `alTick` | `juego->actualizar()` + `Invalidate()` (pide repintar) |
| `Paint` | `alPintar` | Activa el antialiasing y llama `juego->dibujar(e->Graphics)` |
| `KeyDown` | `alTeclaAbajo` | `juego->teclaAbajo(e->KeyCode)` |
| `KeyUp` | `alTeclaArriba` | `juego->teclaArriba(e->KeyCode)` |
| `MouseMove` | `alMoverRaton` | `juego->ratonMovido(x, y)` (para el hover del menú) |
| `MouseDown` | `alClic` | `juego->clic(x, y)` |

En `alPintar` se activa la calidad de dibujo:
```cpp
e->Graphics->SmoothingMode = SmoothingMode::AntiAlias;        // figuras sin dientes
e->Graphics->TextRenderingHint = TextRenderingHint::AntiAlias; // textos suaves
```

## El truco de las flechas (`ProcessDialogKey`)

Windows Forms usa ↑ ↓ ← → para navegar entre controles y normalmente **no** llegan a
`KeyDown`. Como las necesitamos para jugar, se interceptan:
```cpp
virtual bool ProcessDialogKey(Keys keyData) override {
    Keys codigo = keyData & Keys::KeyCode;
    if (codigo == Keys::Up || codigo == Keys::Down ||
        codigo == Keys::Left || codigo == Keys::Right)
        return false;                  // "no la proceses tú, déjala pasar al KeyDown"
    return Form::ProcessDialogKey(keyData);
}
```

## ¿Por qué `Invalidate()` y no dibujar directo?

Podríamos dibujar con `CreateGraphics()` en el tick (como el ejemplo de clase
figurasMovimiento), pero eso pinta directo en pantalla: parpadea y se borra cuando la
ventana se tapa. El camino correcto es: el tick **invalida**, y todo el dibujo vive en
`Paint`, donde el doble búfer hace su magia.
