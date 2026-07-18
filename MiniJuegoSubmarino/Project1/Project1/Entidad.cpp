#include "Entidad.h"
// ============================================================
//  Entidad.cpp
//  Implementacion de la clase padre Entidad.
// ============================================================

namespace Project1 {

	Entidad::Entidad(Bitmap^ hoja, int columnas, int filas,
		float x, float y, int ancho, int alto,
		int vida, float velocidad)
	{
		this->hoja = hoja;
		this->columnas = columnas;
		this->filas = filas;
		this->x = x;
		this->y = y;
		this->ancho = ancho;
		this->alto = alto;
		this->vida = vida;
		this->velocidad = velocidad;
		this->activo = true;
		this->frameActual = 0;
		this->fila = 0;
		this->contadorAnim = 0;
		this->retardoAnim = 4;

		if (hoja != nullptr)
		{
			this->frameAncho = hoja->Width / columnas;
			this->frameAlto = hoja->Height / filas;
		}
		else
		{
			this->frameAncho = ancho;
			this->frameAlto = alto;
		}
	}

	void Entidad::Dibujar(Graphics^ g)
	{
		if (!activo || hoja == nullptr) return;

		Rectangle destino((int)x, (int)y, ancho, alto);
		Rectangle origen(frameActual * frameAncho, fila * frameAlto,
			frameAncho, frameAlto);
		g->DrawImage(hoja, destino, origen, GraphicsUnit::Pixel);
	}

	void Entidad::Animar()
	{
		contadorAnim++;
		if (contadorAnim >= retardoAnim)
		{
			contadorAnim = 0;
			frameActual = (frameActual + 1) % columnas;
		}
	}

	Rectangle Entidad::ObtenerRectangulo()
	{
		return Rectangle((int)x, (int)y, ancho, alto);
	}

	bool Entidad::ColisionaCon(Entidad^ otra)
	{
		if (otra == nullptr || !activo || !otra->Activo) return false;
		return ObtenerRectangulo().IntersectsWith(otra->ObtenerRectangulo());
	}

	void Entidad::PerderVida()
	{
		if (vida > 0) vida--;
		if (vida <= 0) activo = false;
	}

	bool Entidad::EstaVivo()
	{
		return vida > 0;
	}

	// ----- Propiedades -----
	float Entidad::X::get() { return x; }
	void  Entidad::X::set(float valor) { x = valor; }

	float Entidad::Y::get() { return y; }
	void  Entidad::Y::set(float valor) { y = valor; }

	int Entidad::Ancho::get() { return ancho; }
	int Entidad::Alto::get() { return alto; }

	int  Entidad::Vida::get() { return vida; }
	void Entidad::Vida::set(int valor) { vida = valor; }

	float Entidad::Velocidad::get() { return velocidad; }
	void  Entidad::Velocidad::set(float valor) { velocidad = valor; }

	bool Entidad::Activo::get() { return activo; }
	void Entidad::Activo::set(bool valor) { activo = valor; }

	float Entidad::CentroX::get() { return x + ancho / 2.0f; }
	float Entidad::CentroY::get() { return y + alto / 2.0f; }
}
