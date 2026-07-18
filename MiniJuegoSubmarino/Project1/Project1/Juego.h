#pragma once
// ============================================================
//  Juego.h
//  Clase controladora: carga las imagenes, administra todas
//  las entidades, las colisiones, las vidas y los mensajes.
// ============================================================
#include "Entidad.h"
#include "Submarino.h"
#include "Enemigo.h"
#include "Pez.h"
#include "Tiburon.h"
#include "Torpedo.h"
#include "Tesoro.h"
#include "Explosion.h"

namespace Project1 {

	using namespace System;
	using namespace System::Collections::Generic;
	using namespace System::Drawing;
	using namespace System::Windows::Forms;

	public enum class EstadoJuego : int
	{
		Jugando = 0,
		GameOver = 1,
		MisionCompleta = 2
	};

	public ref class Juego
	{
	private:
		int anchoVentana;
		int altoVentana;

		// ----- Imagenes (hojas de sprites) -----
		Bitmap^ imgSubmarinoH;
		Bitmap^ imgSubmarinoV;
		Bitmap^ imgPez;
		Bitmap^ imgTiburon;
		Bitmap^ imgTorpedoH;
		Bitmap^ imgTorpedoV;
		Bitmap^ imgTesoro;
		Bitmap^ imgExplosion;
		Bitmap^ imgCapsulas;

		// ----- Entidades -----
		Submarino^ submarino;
		List<Enemigo^>^ enemigos;
		List<Torpedo^>^ torpedos;
		List<Tesoro^>^ tesoros;
		List<Explosion^>^ explosiones;

		Random^ azar;
		EstadoJuego estado;
		int tesorosRescatados;

		// mensaje temporal en pantalla ("Tesoro rescatado")
		String^ mensaje;
		int mensajeContador;

		// animacion de las capsulas de oxigeno del marcador
		int frameCapsula;
		int contadorCapsula;

		// pequeno intervalo entre disparos
		int enfriamientoDisparo;

		// ----- Metodos internos -----
		String^ BuscarCarpetaImagenes();
		Bitmap^ CargarImagen(String^ carpeta, String^ nombre);
		void CargarImagenes();
		void CrearTesoros();
		void RevisarColisiones();
		void EliminarInactivos();
		void MostrarMensaje(String^ texto);
		void DibujarFondo(Graphics^ g);
		void DibujarCapsulas(Graphics^ g);
		void DibujarTextos(Graphics^ g);

	public:
		Juego(int anchoVentana, int altoVentana);

		void Reiniciar();
		void Actualizar();               // un ciclo del juego (timer)
		void Dibujar(Graphics^ g);

		void TeclaPresionada(Keys tecla);
		void TeclaSoltada(Keys tecla);

		void GenerarPeces();             // tecla P: 3 a 5 peces
		void GenerarTiburones();         // tecla T: 4 a 6 tiburones
		void DispararTorpedo();          // tecla ESPACIO
	};
}
