# 03 · Config — las constantes globales

**Archivo:** `Config.h` (solo encabezado, no necesita .cpp)

## ¿Para qué sirve?

Es el único lugar donde viven los "números mágicos" del juego. Si mañana quieren una
ventana más grande o un suelo más alto, se cambia **aquí una sola vez** y todo el juego
se adapta, porque ninguna clase escribe `1000` o `560` a mano: todas usan `Config::`.

```cpp
public ref class Config abstract sealed {
public:
    literal int ANCHO = 1000;          // ancho del área de juego en píxeles
    literal int ALTO = 700;            // alto del área de juego
    literal float SUELO = 560.0f;      // altura (y) del suelo en los planetas
    literal int TICKS_SEGUNDO = 60;    // el temporizador corre a ~60 ticks por segundo
};
```

## Detalles del lenguaje (C++/CLI)

- `abstract sealed` = clase **estática**: no se puede crear (`gcnew Config()` da error)
  ni heredar. Solo sirve de contenedor de constantes. Lo mismo usan `Azar`, `Sonido`,
  `Estilos`, `Fondo` y `Superficie`.
- `literal` = constante de **tiempo de compilación** (el equivalente a `const` de C# en
  .NET). El compilador reemplaza `Config::ANCHO` por `1000` directamente.

## ¿Quién la usa y para qué?

| Constante | Usos típicos |
|---|---|
| `ANCHO`, `ALTO` | El tamaño de la ventana en `MyForm`; los degradados del cielo; saber cuándo un proyectil "se fue" de la pantalla (`Proyectil::fuera()`); centrar textos (`ANCHO/2`) |
| `SUELO` | Donde pisan el astronauta y el cohete; donde empieza el rectángulo del suelo en `Superficie`; donde explotan los meteoritos de Mercurio |
| `TICKS_SEGUNDO` | Convertir segundos a ticks: "sobrevive 30 s" = `30 * TICKS_SEGUNDO` = 1800 ticks |

> 💡 Ejemplo real: en `MinijuegoMercurio` el contador es
> `restante = 30 * Config::TICKS_SEGUNDO;` y el HUD lo convierte de vuelta con
> `(restante + 59) / 60` (el `+59` redondea hacia arriba para que nunca diga "0 s"
> mientras quede tiempo).
