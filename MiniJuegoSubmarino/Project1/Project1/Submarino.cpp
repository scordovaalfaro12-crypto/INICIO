#include "Submarino.h"
// ============================================================
//  Submarino.cpp
//  El submarino tiene 8 vidas (capsulas de oxigeno) y una
//  velocidad menor a la de los enemigos.
// ============================================================

namespace Project1 {

	// Tamanios con los que se dibuja el submarino
	static const int SUB_ANCHO_H = 100;
	static const int SUB_ALTO_H = 80;
	static const int SUB_ANCHO_V = 78;
	static const int SUB_ALTO_V = 104;

	Submarino::Submarino(Bitmap^ hojaHorizontal, Bitmap^ hojaVertical,
		float x, float y)
		: Entidad(hojaHorizontal, 3, 2, x, y,
			SUB_ANCHO_H, SUB_ALTO_H,
			8,      /* vida: 8 capsulas de oxigeno */
			6.0f    /* velocidad menor que los enemigos */)
	{
		this->hojaHorizontal = hojaHorizontal;
		this->hojaVertical = hojaVertical;
		this->direccion = Direccion::Derecha;
		this->moverIzquierda = false;
		this->moverDerecha = false;
		this->moverArriba = false;
		this->moverAbajo = false;
		ActualizarSprite();
	}

	void Submarino::ActualizarSprite()
	{
		float cx = CentroX;
		float cy = CentroY;

		if (direccion == Direccion::Izquierda || direccion == Direccion::Derecha)
		{
			hoja = hojaHorizontal;
			ancho = SUB_ANCHO_H;
			alto = SUB_ALTO_H;
			fila = (direccion == Direccion::Izquierda) ? 0 : 1;
		}
		else
		{
			hoja = hojaVertical;
			ancho = SUB_ANCHO_V;
			alto = SUB_ALTO_V;
			fila = (direccion == Direccion::Arriba) ? 0 : 1;
		}

		if (hoja != nullptr)
		{
			frameAncho = hoja->Width / columnas;
			frameAlto = hoja->Height / filas;
		}

		// conserva el centro al cambiar de orientacion
		x = cx - ancho / 2.0f;
		y = cy - alto / 2.0f;
	}

	void Submarino::EmpezarMovimiento(Direccion d)
	{
		switch (d)
		{
		case Direccion::Izquierda: moverIzquierda = true; break;
		case Direccion::Derecha:   moverDerecha = true; break;
		case Direccion::Arriba:    moverArriba = true; break;
		case Direccion::Abajo:     moverAbajo = true; break;
		}
		if (direccion != d)
		{
			direccion = d;
			ActualizarSprite();
		}
	}

	void Submarino::PararMovimiento(Direccion d)
	{
		switch (d)
		{
		case Direccion::Izquierda: moverIzquierda = false; break;
		case Direccion::Derecha:   moverDerecha = false; break;
		case Direccion::Arriba:    moverArriba = false; break;
		case Direccion::Abajo:     moverAbajo = false; break;
		}
	}

	void Submarino::Mover(int anchoVentana, int altoVentana)
	{
		if (!activo) return;

		if (moverIzquierda) x -= velocidad;
		if (moverDerecha)   x += velocidad;
		if (moverArriba)    y -= velocidad;
		if (moverAbajo)     y += velocidad;

		// el submarino no puede salir de la ventana
		if (x < 0) x = 0;
		if (y < 0) y = 0;
		if (x > anchoVentana - ancho) x = (float)(anchoVentana - ancho);
		if (y > altoVentana - alto)   y = (float)(altoVentana - alto);

		Animar();
	}

	Direccion Submarino::Sentido::get()
	{
		return direccion;
	}
}
