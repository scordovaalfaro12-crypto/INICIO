#pragma once
using namespace System;
using namespace System::Drawing;

// Guardabosques: personaje controlado por el jugador.
// Se mueve con las flechas direccionales o con W, A, S, D; cuenta con un
// sprite animado de cuatro direcciones, inicia con 3 vidas y dispara dardos
// con la barra espaciadora en la direccion hacia la que mira.
// Sprite: _guardabosque.png (hoja 4x4, frames de 256x256).
class Guardabosques
{
private:
	int x, y;
	int ancho, alto;
	int velocidad;
	int fila;          // 0 abajo, 1 izquierda, 2 derecha, 3 arriba
	int frame;
	int contadorAnim;
	int vidas;
	int dardos;

	static const int FRAME_ANCHO = 256;
	static const int FRAME_ALTO = 256;
	static const int TOTAL_FRAMES = 4;
	static const int VIDAS_MAXIMAS = 3;

public:
	Guardabosques(int x, int y);
	~Guardabosques();

	void mover(bool arriba, bool abajo, bool izquierda, bool derecha, Rectangle zona);
	void dibujar(Graphics^ g, Bitmap^ bmp);
	Rectangle getRectangulo();

	int getDireccion();     // direccion hacia la que mira (para el dardo)
	int getCentroX();
	int getCentroY();

	bool usarDardo();       // false si no hay dardos disponibles
	void agregarDardos(int cantidad);
	void quitarVida();
	void agregarVida();     // no supera el maximo
	int getVidas();
	int getDardos();
};
