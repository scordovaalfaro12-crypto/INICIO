#pragma once
using namespace System;
using namespace System::Drawing;

// Dardo tranquilizante disparado por el guardabosques.
// Viaja en linea recta en la direccion hacia la que miraba el guardabosques.
// Si impacta a un dinosaurio lo captura; si sale de la zona de juego,
// desaparece y el guardabosques pierde 1 vida.
// Sprite: _dardo.png (hoja 4x4, frames de 256x256).
class Dardo
{
private:
	int x, y;
	int ancho, alto;
	int velocidad;
	int direccion;      // 0 abajo, 1 izquierda, 2 derecha, 3 arriba
	int frame;
	int contadorAnim;
	bool activo;

	static const int FRAME_ANCHO = 256;
	static const int FRAME_ALTO = 256;
	static const int TOTAL_FRAMES = 4;

public:
	Dardo(int centroX, int centroY, int direccion);
	~Dardo();

	void mover();
	bool estaFuera(Rectangle zona);
	void dibujar(Graphics^ g, Bitmap^ bmp);
	Rectangle getRectangulo();
	void desactivar();
	bool estaActivo();
};
