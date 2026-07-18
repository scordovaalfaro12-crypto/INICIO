#include "Juego.h"
// ============================================================
//  Juego.cpp
//  Logica completa del mini juego del submarino.
// ============================================================

namespace Project1 {

	using namespace System::IO;
	using namespace System::Drawing::Drawing2D;

	Juego::Juego(int anchoVentana, int altoVentana)
	{
		this->anchoVentana = anchoVentana;
		this->altoVentana = altoVentana;
		this->azar = gcnew Random();

		CargarImagenes();
		Reiniciar();
	}

	// ------------------------------------------------------------
	//  Carga de imagenes
	// ------------------------------------------------------------
	String^ Juego::BuscarCarpetaImagenes()
	{
		// busca la carpeta IMAGENES subiendo desde la carpeta del .exe
		String^ dir = Application::StartupPath;
		for (int i = 0; i < 8; i++)
		{
			String^ ruta = Path::Combine(dir, "IMAGENES");
			if (Directory::Exists(ruta)) return ruta;

			DirectoryInfo^ padre = Directory::GetParent(dir);
			if (padre == nullptr) break;
			dir = padre->FullName;
		}
		return "IMAGENES";
	}

	Bitmap^ Juego::CargarImagen(String^ carpeta, String^ nombre)
	{
		String^ ruta = Path::Combine(carpeta, nombre);
		if (!File::Exists(ruta))
		{
			MessageBox::Show("No se encontro la imagen: " + ruta,
				"Error de imagenes", MessageBoxButtons::OK,
				MessageBoxIcon::Error);
			return nullptr;
		}
		Bitmap^ bmp = gcnew Bitmap(ruta);
		// el color de la esquina superior izquierda se vuelve transparente
		bmp->MakeTransparent(bmp->GetPixel(0, 0));
		return bmp;
	}

	void Juego::CargarImagenes()
	{
		String^ carpeta = BuscarCarpetaImagenes();
		imgSubmarinoH = CargarImagen(carpeta, "SubmarinoHorizontal.png");
		imgSubmarinoV = CargarImagen(carpeta, "SubmarinoVertical.png");
		imgPez = CargarImagen(carpeta, "PezEnemigo.png");
		imgTiburon = CargarImagen(carpeta, "TiburonEnemigo.png");
		imgTorpedoH = CargarImagen(carpeta, "TorpedoHorizontal.png");
		imgTorpedoV = CargarImagen(carpeta, "TorpedoVertical.png");
		imgTesoro = CargarImagen(carpeta, "Tesoros.png");
		imgExplosion = CargarImagen(carpeta, "Explosion.png");
		imgCapsulas = CargarImagen(carpeta, "Capsulas.png");
	}

	// ------------------------------------------------------------
	//  Preparacion de la partida
	// ------------------------------------------------------------
	void Juego::Reiniciar()
	{
		submarino = gcnew Submarino(imgSubmarinoH, imgSubmarinoV,
			60.0f, altoVentana / 2.0f);
		enemigos = gcnew List<Enemigo^>();
		torpedos = gcnew List<Torpedo^>();
		explosiones = gcnew List<Explosion^>();

		estado = EstadoJuego::Jugando;
		tesorosRescatados = 0;
		mensaje = "";
		mensajeContador = 0;
		frameCapsula = 0;
		contadorCapsula = 0;
		enfriamientoDisparo = 0;

		CrearTesoros();
	}

	void Juego::CrearTesoros()
	{
		// 4 tesoros distribuidos de forma aleatoria en el juego
		tesoros = gcnew List<Tesoro^>();
		for (int i = 0; i < 4; i++)
		{
			bool valido;
			float tx, ty;
			int intentos = 0;
			do
			{
				valido = true;
				tx = (float)azar->Next(180, anchoVentana - 100);
				ty = (float)azar->Next(120, altoVentana - 100);

				// evita que dos cofres queden encimados
				for each (Tesoro^ t in tesoros)
				{
					if (Math::Abs(t->X - tx) < 100 && Math::Abs(t->Y - ty) < 100)
					{
						valido = false;
						break;
					}
				}
				intentos++;
			} while (!valido && intentos < 50);

			tesoros->Add(gcnew Tesoro(imgTesoro, tx, ty));
		}
	}

	// ------------------------------------------------------------
	//  Entrada del usuario
	// ------------------------------------------------------------
	void Juego::TeclaPresionada(Keys tecla)
	{
		if (estado != EstadoJuego::Jugando)
		{
			if (tecla == Keys::R) Reiniciar();
			return;
		}

		switch (tecla)
		{
			// movimiento con flechas o W, A, S, D
		case Keys::Left:
		case Keys::A: submarino->EmpezarMovimiento(Direccion::Izquierda); break;
		case Keys::Right:
		case Keys::D: submarino->EmpezarMovimiento(Direccion::Derecha); break;
		case Keys::Up:
		case Keys::W: submarino->EmpezarMovimiento(Direccion::Arriba); break;
		case Keys::Down:
		case Keys::S: submarino->EmpezarMovimiento(Direccion::Abajo); break;

			// enemigos
		case Keys::P: GenerarPeces(); break;
		case Keys::T: GenerarTiburones(); break;

			// disparo
		case Keys::Space: DispararTorpedo(); break;
		}
	}

	void Juego::TeclaSoltada(Keys tecla)
	{
		switch (tecla)
		{
		case Keys::Left:
		case Keys::A: submarino->PararMovimiento(Direccion::Izquierda); break;
		case Keys::Right:
		case Keys::D: submarino->PararMovimiento(Direccion::Derecha); break;
		case Keys::Up:
		case Keys::W: submarino->PararMovimiento(Direccion::Arriba); break;
		case Keys::Down:
		case Keys::S: submarino->PararMovimiento(Direccion::Abajo); break;
		}
	}

	// ------------------------------------------------------------
	//  Aparicion de enemigos y disparos
	// ------------------------------------------------------------
	void Juego::GenerarPeces()
	{
		// entre 3 y 5 peces, de derecha a izquierda,
		// mas rapidos que el submarino (velocidad 6)
		int cantidad = azar->Next(3, 6);
		for (int i = 0; i < cantidad; i++)
		{
			float px = (float)(anchoVentana + azar->Next(20, 320));
			float py = (float)azar->Next(60, altoVentana - 100);
			float vel = (float)azar->Next(7, 10);           // 7 a 9
			enemigos->Add(gcnew Pez(imgPez, px, py, vel));
		}
	}

	void Juego::GenerarTiburones()
	{
		// entre 4 y 6 tiburones, en ambos sentidos,
		// mas rapidos que los peces y que el submarino
		int cantidad = azar->Next(4, 7);
		for (int i = 0; i < cantidad; i++)
		{
			bool haciaLaDerecha = (azar->Next(0, 2) == 1);
			float py = (float)azar->Next(60, altoVentana - 100);
			float vel = (float)azar->Next(10, 14);          // 10 a 13
			float px = haciaLaDerecha
				? (float)(-120 - azar->Next(0, 300))
				: (float)(anchoVentana + azar->Next(20, 320));
			enemigos->Add(gcnew Tiburon(imgTiburon, px, py, vel, haciaLaDerecha));
		}
	}

	void Juego::DispararTorpedo()
	{
		if (enfriamientoDisparo > 0) return;
		enfriamientoDisparo = 8;

		torpedos->Add(gcnew Torpedo(imgTorpedoH, imgTorpedoV,
			submarino->CentroX, submarino->CentroY,
			submarino->Sentido));
	}

	// ------------------------------------------------------------
	//  Ciclo principal
	// ------------------------------------------------------------
	void Juego::Actualizar()
	{
		// las explosiones y la animacion de capsulas siguen su curso
		for each (Explosion^ e in explosiones)
			e->Mover(anchoVentana, altoVentana);

		contadorCapsula++;
		if (contadorCapsula >= 6)
		{
			contadorCapsula = 0;
			frameCapsula = (frameCapsula + 1) % 4;
		}

		if (mensajeContador > 0) mensajeContador--;

		if (estado != EstadoJuego::Jugando)
		{
			EliminarInactivos();
			return;
		}

		if (enfriamientoDisparo > 0) enfriamientoDisparo--;

		// movimiento de todas las entidades
		submarino->Mover(anchoVentana, altoVentana);

		for each (Enemigo^ e in enemigos)
			e->Mover(anchoVentana, altoVentana);

		for each (Torpedo^ t in torpedos)
			t->Mover(anchoVentana, altoVentana);

		for each (Tesoro^ t in tesoros)
			t->Mover(anchoVentana, altoVentana);

		RevisarColisiones();
		EliminarInactivos();
	}

	void Juego::RevisarColisiones()
	{
		// ----- torpedos contra enemigos -----
		for each (Torpedo^ t in torpedos)
		{
			for each (Enemigo^ e in enemigos)
			{
				if (t->ColisionaCon(e))
				{
					explosiones->Add(gcnew Explosion(imgExplosion,
						e->CentroX, e->CentroY));
					e->Activo = false;
					t->Activo = false;
					break;
				}
			}
		}

		// ----- enemigos contra el submarino -----
		for each (Enemigo^ e in enemigos)
		{
			if (e->ColisionaCon(submarino))
			{
				e->Activo = false;
				explosiones->Add(gcnew Explosion(imgExplosion,
					e->CentroX, e->CentroY));

				submarino->PerderVida();   // pierde 1 capsula de oxigeno
				if (!submarino->EstaVivo())
				{
					estado = EstadoJuego::GameOver;
					return;
				}
			}
		}

		// ----- tesoros contra el submarino -----
		for each (Tesoro^ t in tesoros)
		{
			if (!t->Rescatado && t->ColisionaCon(submarino))
			{
				t->Rescatar();
				tesorosRescatados++;

				if (tesorosRescatados >= 4)
				{
					estado = EstadoJuego::MisionCompleta;
				}
				else
				{
					MostrarMensaje("Tesoro rescatado");
				}
			}
		}
	}

	void Juego::EliminarInactivos()
	{
		for (int i = enemigos->Count - 1; i >= 0; i--)
			if (!enemigos[i]->Activo) enemigos->RemoveAt(i);

		for (int i = torpedos->Count - 1; i >= 0; i--)
			if (!torpedos[i]->Activo) torpedos->RemoveAt(i);

		for (int i = explosiones->Count - 1; i >= 0; i--)
			if (!explosiones[i]->Activo) explosiones->RemoveAt(i);
	}

	void Juego::MostrarMensaje(String^ texto)
	{
		mensaje = texto;
		mensajeContador = 70;   // ~2 segundos con el timer a 30 ms
	}

	// ------------------------------------------------------------
	//  Dibujo
	// ------------------------------------------------------------
	void Juego::Dibujar(Graphics^ g)
	{
		g->InterpolationMode = InterpolationMode::NearestNeighbor;
		g->PixelOffsetMode = PixelOffsetMode::Half;

		DibujarFondo(g);

		for each (Tesoro^ t in tesoros)      t->Dibujar(g);
		for each (Torpedo^ t in torpedos)    t->Dibujar(g);
		for each (Enemigo^ e in enemigos)    e->Dibujar(g);
		if (submarino->EstaVivo())           submarino->Dibujar(g);
		for each (Explosion^ e in explosiones) e->Dibujar(g);

		DibujarCapsulas(g);
		DibujarTextos(g);
	}

	void Juego::DibujarFondo(Graphics^ g)
	{
		// degradado azul que simula el fondo del mar
		Rectangle zona(0, 0, anchoVentana, altoVentana);
		LinearGradientBrush^ mar = gcnew LinearGradientBrush(
			zona,
			Color::FromArgb(255, 22, 105, 165),
			Color::FromArgb(255, 3, 30, 62),
			LinearGradientMode::Vertical);
		g->FillRectangle(mar, zona);
		delete mar;
	}

	void Juego::DibujarCapsulas(Graphics^ g)
	{
		// las capsulas de oxigeno (vidas) van en la esquina
		// superior derecha; se dibuja una por cada vida restante
		if (imgCapsulas == nullptr) return;

		int capAncho = 26;
		int capAlto = 38;
		int separacion = 30;
		int margen = 12;

		int frameW = imgCapsulas->Width / 4;
		int frameH = imgCapsulas->Height;

		for (int i = 0; i < submarino->Vida; i++)
		{
			int cx = anchoVentana - margen - (i + 1) * separacion;
			Rectangle destino(cx, margen, capAncho, capAlto);
			Rectangle origen(frameCapsula * frameW, 0, frameW, frameH);
			g->DrawImage(imgCapsulas, destino, origen, GraphicsUnit::Pixel);
		}
	}

	void Juego::DibujarTextos(Graphics^ g)
	{
		// marcador de tesoros
		Font^ fuenteChica = gcnew Font("Segoe UI", 12, FontStyle::Bold);
		String^ marcador = String::Format("Tesoros: {0} / 4", tesorosRescatados);
		g->DrawString(marcador, fuenteChica, Brushes::White, 12.0f, 12.0f);

		// mensaje temporal: "Tesoro rescatado"
		if (mensajeContador > 0 && mensaje->Length > 0)
		{
			Font^ fuenteMedia = gcnew Font("Segoe UI", 22, FontStyle::Bold);
			SizeF medida = g->MeasureString(mensaje, fuenteMedia);
			float mx = (anchoVentana - medida.Width) / 2.0f;
			g->DrawString(mensaje, fuenteMedia, Brushes::Gold, mx, 70.0f);
			delete fuenteMedia;
		}

		// mensajes finales
		if (estado != EstadoJuego::Jugando)
		{
			String^ texto = (estado == EstadoJuego::GameOver)
				? "GAME OVER" : "MISION COMPLETA";
			Brush^ color = (estado == EstadoJuego::GameOver)
				? Brushes::OrangeRed : Brushes::LightGreen;

			Font^ fuenteGrande = gcnew Font("Segoe UI", 44, FontStyle::Bold);
			SizeF medida = g->MeasureString(texto, fuenteGrande);
			float mx = (anchoVentana - medida.Width) / 2.0f;
			float my = (altoVentana - medida.Height) / 2.0f;
			g->DrawString(texto, fuenteGrande, color, mx, my);
			delete fuenteGrande;

			String^ aviso = "Presiona R para volver a jugar";
			SizeF medida2 = g->MeasureString(aviso, fuenteChica);
			g->DrawString(aviso, fuenteChica, Brushes::White,
				(anchoVentana - medida2.Width) / 2.0f, my + medida.Height + 10);
		}

		delete fuenteChica;
	}
}
