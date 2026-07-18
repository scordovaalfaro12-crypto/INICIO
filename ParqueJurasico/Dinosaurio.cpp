#include "Dinosaurio.h"

Dinosaurio::Dinosaurio(int x, int y, int ancho, int alto, int velocidad,
	int frameAncho, int frameAlto, int totalFrames)
{
	this->x = x;
	this->y = y;
	this->ancho = ancho;
	this->alto = alto;
	this->velocidad = velocidad;
	this->frameAncho = frameAncho;
	this->frameAlto = frameAlto;
	this->totalFrames = totalFrames;
	dx = 0;
	dy = 0;
	fila = 0;
	frame = 0;
	contadorAnim = 0;
	capturado = false;
}

Dinosaurio::~Dinosaurio()
{
}

bool Dinosaurio::recibirImpacto()
{
	// Por defecto un solo dardo captura al dinosaurio
	return true;
}

void Dinosaurio::dibujar(Graphics^ g, Bitmap^ bmp)
{
	Rectangle destino(x, y, ancho, alto);
	Rectangle origen(frame * frameAncho, fila * frameAlto, frameAncho, frameAlto);
	g->DrawImage(bmp, destino, origen, GraphicsUnit::Pixel);
}

void Dinosaurio::moverCapturado(Rectangle zona)
{
	// En la zona de capturados se desplaza muy lentamente y rebota en sus bordes
	x += dx;
	y += dy;
	if (x <= zona.X)                 { x = zona.X; dx = 1; }
	else if (x + ancho >= zona.Right)  { x = zona.Right - ancho; dx = -1; }
	if (y <= zona.Y)                 { y = zona.Y; dy = 1; }
	else if (y + alto >= zona.Bottom)  { y = zona.Bottom - alto; dy = -1; }
	avanzarAnimacion();
}

void Dinosaurio::capturar(Rectangle zonaCapturados)
{
	capturado = true;
	x = zonaCapturados.X + rand() % (zonaCapturados.Width - ancho);
	y = zonaCapturados.Y + rand() % (zonaCapturados.Height - alto);
	dx = (rand() % 2 == 0) ? 1 : -1;
	dy = (rand() % 2 == 0) ? 1 : -1;
}

bool Dinosaurio::estaCapturado()
{
	return capturado;
}

Rectangle Dinosaurio::getRectangulo()
{
	return Rectangle(x, y, ancho, alto);
}

void Dinosaurio::avanzarAnimacion()
{
	contadorAnim++;
	if (contadorAnim >= 6)
	{
		contadorAnim = 0;
		frame = (frame + 1) % totalFrames;
	}
}
