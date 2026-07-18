#include "Controlador.h"
#include <fstream>
#include <cstdlib>
#include <ctime>

using namespace System::Windows::Forms;
using namespace System::Drawing::Drawing2D;

static int aleatorio(int minimo, int maximo)
{
	return minimo + rand() % (maximo - minimo + 1);
}

Controlador::Controlador()
{
	srand((unsigned)time(nullptr));

	estado = JUGANDO;
	ticks = 0;
	arriba = abajo = izquierda = derecha = false;
	pteranodontesGenerados = false;
	alfaGenerado = false;
	totalGenerados = 0;
	invulnerableTicks = 0;

	capturadosRaptor = capturadosPtera = capturadosAlfa = 0;
	dardosRecogidos = dardosDisparados = aciertos = fallos = 0;
	mensajeTicks = 0;

	Rectangle zj = zonaJuego();
	guarda = new Guardabosques(zj.X + zj.Width / 2, zj.Y + zj.Height / 2);
	helicoptero = new Helicoptero(zj);
	alfa = nullptr;

	generarVelocirraptores();
}

Controlador::~Controlador()
{
	delete guarda;
	delete helicoptero;
	// El alfa esta dentro del vector "dinosaurios": se libera una sola vez
	for (size_t i = 0; i < dinosaurios.size(); i++) delete dinosaurios[i];
	for (size_t i = 0; i < dardos.size(); i++) delete dardos[i];
	for (size_t i = 0; i < cajas.size(); i++) delete cajas[i];
	dinosaurios.clear();
	dardos.clear();
	cajas.clear();
	alfa = nullptr;
}

Rectangle Controlador::zonaJuego()
{
	return Rectangle(10, HUD_ALTO, 730, ALTO_VENTANA - HUD_ALTO - 10);
}

Rectangle Controlador::zonaCapturados()
{
	return Rectangle(750, HUD_ALTO, ANCHO_VENTANA - 750 - 10, ALTO_VENTANA - HUD_ALTO - 10);
}

void Controlador::teclaPresionada(int codigo)
{
	if (codigo == (int)Keys::Up    || codigo == (int)Keys::W) arriba = true;
	if (codigo == (int)Keys::Down  || codigo == (int)Keys::S) abajo = true;
	if (codigo == (int)Keys::Left  || codigo == (int)Keys::A) izquierda = true;
	if (codigo == (int)Keys::Right || codigo == (int)Keys::D) derecha = true;
	if (codigo == (int)Keys::Space) disparar();
	if (codigo == (int)Keys::P) generarPteranodontes();
}

void Controlador::teclaLiberada(int codigo)
{
	if (codigo == (int)Keys::Up    || codigo == (int)Keys::W) arriba = false;
	if (codigo == (int)Keys::Down  || codigo == (int)Keys::S) abajo = false;
	if (codigo == (int)Keys::Left  || codigo == (int)Keys::A) izquierda = false;
	if (codigo == (int)Keys::Right || codigo == (int)Keys::D) derecha = false;
}

void Controlador::actualizar()
{
	if (estado != JUGANDO) return;

	ticks++;
	if (mensajeTicks > 0) mensajeTicks--;
	if (invulnerableTicks > 0) invulnerableTicks--;

	guarda->mover(arriba, abajo, izquierda, derecha, zonaJuego());

	// Polimorfismo: cada especie ejecuta su propio algoritmo de mover()
	for (size_t i = 0; i < dinosaurios.size(); i++)
	{
		if (dinosaurios[i]->estaCapturado())
			dinosaurios[i]->moverCapturado(zonaCapturados());
		else
			dinosaurios[i]->mover(zonaJuego());
	}

	helicoptero->mover(zonaJuego());
	moverDardos();
	revisarColisiones();
	revisarAlfaYFinDeJuego();
}

void Controlador::generarVelocirraptores()
{
	int cantidad = aleatorio(6, 9);
	totalGenerados = cantidad;
	Rectangle z = zonaJuego();
	for (int i = 0; i < cantidad; i++)
	{
		int px = aleatorio(z.X, z.Right - 60);
		int py = aleatorio(z.Y, z.Bottom - 60);
		dinosaurios.push_back(new Velocirraptor(px, py));
	}
	// Cajas de suministros: entre N y 2N, con N = dinosaurios generados
	generarCajas(cantidad, 2 * cantidad);
}

void Controlador::generarPteranodontes()
{
	if (estado != JUGANDO) return;
	if (pteranodontesGenerados)
	{
		mostrarMensaje("Los pteranodones ya fueron liberados.");
		return;
	}
	pteranodontesGenerados = true;
	int cantidad = aleatorio(3, 5);
	totalGenerados += cantidad;
	Rectangle z = zonaJuego();
	for (int i = 0; i < cantidad; i++)
	{
		int px = aleatorio(z.X, z.Right - 60);
		int py = aleatorio(z.Y, z.Bottom - 60);
		dinosaurios.push_back(new Pteranodonte(px, py));
	}
	// Cajas adicionales para mantener la proporcion N..2N del nuevo total
	generarCajas(cantidad, 2 * cantidad);
	mostrarMensaje("Pteranodones liberados en el parque!");
}

void Controlador::generarCajas(int minimo, int maximo)
{
	int cantidad = aleatorio(minimo, maximo);
	Rectangle z = zonaJuego();
	for (int i = 0; i < cantidad; i++)
	{
		int px = aleatorio(z.X, z.Right - 22);
		int py = aleatorio(z.Y, z.Bottom - 22);
		cajas.push_back(new Caja(px, py));
	}
}

void Controlador::disparar()
{
	if (estado != JUGANDO) return;
	if (guarda->usarDardo())
	{
		dardosDisparados++;
		dardos.push_back(new Dardo(guarda->getCentroX(), guarda->getCentroY(), guarda->getDireccion()));
	}
	else
	{
		mostrarMensaje("No dispone de dardos tranquilizantes.");
	}
}

void Controlador::moverDardos()
{
	for (size_t i = 0; i < dardos.size(); i++)
	{
		if (dardos[i]->estaActivo()) dardos[i]->mover();
	}
}

void Controlador::revisarColisiones()
{
	// Dardo - dinosaurio: captura
	for (size_t i = 0; i < dardos.size(); i++)
	{
		Dardo* d = dardos[i];
		if (!d->estaActivo()) continue;

		for (size_t j = 0; j < dinosaurios.size(); j++)
		{
			Dinosaurio* dino = dinosaurios[j];
			if (dino->estaCapturado()) continue;
			if (d->getRectangulo().IntersectsWith(dino->getRectangulo()))
			{
				aciertos++;
				d->desactivar();
				if (dino->recibirImpacto())   // el alfa requiere 2 impactos
				{
					dino->capturar(zonaCapturados());
					switch (dino->getTipo())
					{
					case 0: capturadosRaptor++; break;
					case 1: capturadosPtera++; break;
					case 2: capturadosAlfa++; break;
					}
				}
				break;
			}
		}

		// Dardo fallido: sale de la zona de juego -> desaparece y -1 vida
		if (d->estaActivo() && d->estaFuera(zonaJuego()))
		{
			fallos++;
			guarda->quitarVida();
			d->desactivar();
			mostrarMensaje("Dardo perdido: el guardabosques pierde 1 vida.");
		}
	}
	// Eliminar (liberar) los dardos que ya no estan activos
	for (size_t i = 0; i < dardos.size(); )
	{
		if (!dardos[i]->estaActivo())
		{
			delete dardos[i];
			dardos.erase(dardos.begin() + i);
		}
		else i++;
	}

	// Guardabosques - caja: la caja desaparece y suma 1 dardo
	for (size_t i = 0; i < cajas.size(); )
	{
		if (guarda->getRectangulo().IntersectsWith(cajas[i]->getRectangulo()))
		{
			guarda->agregarDardos(1);
			dardosRecogidos++;
			delete cajas[i];
			cajas.erase(cajas.begin() + i);
		}
		else i++;
	}

	// Guardabosques - helicoptero: apoyo de vida y dardos
	if (helicoptero->estaActivo() &&
		guarda->getRectangulo().IntersectsWith(helicoptero->getRectangulo()))
	{
		guarda->agregarVida();
		guarda->agregarDardos(3);
		dardosRecogidos += 3;
		helicoptero->recoger();
		mostrarMensaje("Helicoptero de apoyo: +1 vida y +3 dardos.");
	}

	// Guardabosques - Dinosaurio Alfa: pierde 1 vida por colision
	if (alfaGenerado && alfa != nullptr && !alfa->estaCapturado() &&
		invulnerableTicks == 0 &&
		guarda->getRectangulo().IntersectsWith(alfa->getRectangulo()))
	{
		guarda->quitarVida();
		invulnerableTicks = 50;   // tiempo de gracia para no perder todo de golpe
		mostrarMensaje("El Dinosaurio Alfa te ha golpeado: -1 vida.");
	}
}

void Controlador::revisarAlfaYFinDeJuego()
{
	// Aparicion del Dinosaurio Alfa al capturar el 70% del total generado
	if (!alfaGenerado && totalGenerados > 0 &&
		capturadosNormales() * 10 >= totalGenerados * 7)
	{
		Rectangle z = zonaJuego();
		alfa = new DinosaurioAlfa(aleatorio(z.X, z.Right - 72), aleatorio(z.Y, z.Bottom - 72));
		dinosaurios.push_back(alfa);
		alfaGenerado = true;
		mostrarMensaje("ALERTA! El Dinosaurio Alfa ha aparecido.");
	}

	// Derrota: el guardabosques pierde todas sus vidas
	if (guarda->getVidas() <= 0)
	{
		estado = PERDIDO;
		guardarResultados();
		return;
	}

	// Victoria: se captura al Alfa o a todos los dinosaurios restantes
	bool victoria = false;
	if (alfaGenerado && alfa != nullptr && alfa->estaCapturado()) victoria = true;
	if (totalGenerados > 0 && capturadosNormales() >= totalGenerados) victoria = true;
	if (victoria)
	{
		estado = GANADO;
		guardarResultados();
	}
}

void Controlador::mostrarMensaje(const std::string& texto)
{
	mensaje = texto;
	mensajeTicks = 70;   // ~2 segundos en pantalla
}

int Controlador::capturadosNormales()
{
	return capturadosRaptor + capturadosPtera;
}

int Controlador::getEstado()
{
	return estado;
}

int Controlador::getTiempoSegundos()
{
	return ticks * INTERVALO_MS / 1000;
}

void Controlador::guardarResultados()
{
	// Gestion de archivos exclusivamente en modo texto
	std::ofstream archivo("RESULTADOS.TXT");
	if (!archivo.is_open()) return;
	archivo << "===== PARQUE JURASICO: OPERACION CONTENCION 2042 =====\n";
	archivo << "Resultado final: "
	        << (estado == GANADO ? "PARQUE ASEGURADO" : "LOS DINOSAURIOS ESCAPARON") << "\n";
	archivo << "Tiempo total de la partida: " << getTiempoSegundos() << " segundos\n";
	archivo << "Velocirraptores capturados: " << capturadosRaptor << "\n";
	archivo << "Pteranodones capturados: " << capturadosPtera << "\n";
	archivo << "Dinosaurio Alfa capturado: " << capturadosAlfa << "\n";
	archivo << "Dardos recogidos: " << dardosRecogidos << "\n";
	archivo << "Dardos disparados: " << dardosDisparados << "\n";
	archivo << "Disparos acertados: " << aciertos << "\n";
	archivo << "Disparos fallidos: " << fallos << "\n";
	archivo << "Vidas restantes: " << guarda->getVidas() << "\n";
	archivo.close();
}

void Controlador::dibujar(BufferedGraphics^ buffer, Bitmap^ bmpGuarda, Bitmap^ bmpRaptor,
	Bitmap^ bmpPtera, Bitmap^ bmpAlfa, Bitmap^ bmpDardo, Bitmap^ bmpHeli)
{
	Graphics^ g = buffer->Graphics;
	g->InterpolationMode = InterpolationMode::NearestNeighbor;   // pixel art nitido
	g->PixelOffsetMode = PixelOffsetMode::Half;

	dibujarFondo(g);

	for (size_t i = 0; i < cajas.size(); i++) cajas[i]->dibujar(g);

	helicoptero->dibujar(g, bmpHeli);

	for (size_t i = 0; i < dinosaurios.size(); i++)
	{
		Bitmap^ bmp;
		switch (dinosaurios[i]->getTipo())
		{
		case 0:  bmp = bmpRaptor; break;
		case 1:  bmp = bmpPtera; break;
		default: bmp = bmpAlfa; break;
		}
		dinosaurios[i]->dibujar(g, bmp);
	}

	for (size_t i = 0; i < dardos.size(); i++) dardos[i]->dibujar(g, bmpDardo);

	guarda->dibujar(g, bmpGuarda);

	dibujarHUD(g);
	dibujarMensajes(g);

	// El buffer se dibuja completo y se vuelca de golpe: sin parpadeo
	buffer->Render();
}

void Controlador::dibujarFondo(Graphics^ g)
{
	g->Clear(Color::FromArgb(18, 34, 18));

	// Zona de juego: selva del parque
	Rectangle zj = zonaJuego();
	LinearGradientBrush^ cesped = gcnew LinearGradientBrush(
		zj, Color::FromArgb(54, 88, 42), Color::FromArgb(28, 54, 26), 90.0f);
	g->FillRectangle(cesped, zj);
	delete cesped;

	// Vegetacion decorativa (posiciones fijas dentro de la zona)
	int arboles[8][2] = { {40,60},{190,40},{340,90},{560,50},{80,370},{420,300},{610,420},{240,470} };
	SolidBrush^ tronco = gcnew SolidBrush(Color::FromArgb(92, 62, 30));
	SolidBrush^ copa = gcnew SolidBrush(Color::FromArgb(24, 70, 24));
	SolidBrush^ copa2 = gcnew SolidBrush(Color::FromArgb(34, 92, 30));
	for (int i = 0; i < 8; i++)
	{
		int ax = zj.X + arboles[i][0];
		int ay = zj.Y + arboles[i][1];
		g->FillRectangle(tronco, ax + 20, ay + 34, 10, 22);
		g->FillEllipse(copa, ax, ay, 50, 40);
		g->FillEllipse(copa2, ax + 8, ay + 4, 34, 28);
	}
	delete tronco; delete copa; delete copa2;

	Pen^ cerca = gcnew Pen(Color::FromArgb(120, 84, 40), 4);
	g->DrawRectangle(cerca, zj);
	delete cerca;

	// Zona de capturados (zona asegurada)
	Rectangle zc = zonaCapturados();
	SolidBrush^ fondoZc = gcnew SolidBrush(Color::FromArgb(40, 48, 34));
	g->FillRectangle(fondoZc, zc);
	delete fondoZc;
	Pen^ bordeZc = gcnew Pen(Color::Goldenrod, 3);
	g->DrawRectangle(bordeZc, zc);
	delete bordeZc;

	Font^ fz = gcnew Font("Segoe UI", 10, FontStyle::Bold);
	String^ titulo = "ZONA DE CAPTURADOS";
	SizeF medida = g->MeasureString(titulo, fz);
	g->DrawString(titulo, fz, Brushes::Gold,
		zc.X + (zc.Width - medida.Width) / 2.0f, (float)(zc.Y + 6));
	delete fz;
}

void Controlador::dibujarHUD(Graphics^ g)
{
	SolidBrush^ barra = gcnew SolidBrush(Color::FromArgb(12, 12, 12));
	g->FillRectangle(barra, 0, 0, ANCHO_VENTANA, HUD_ALTO);
	delete barra;

	Font^ f = gcnew Font("Segoe UI", 11, FontStyle::Bold);
	Font^ fChica = gcnew Font("Segoe UI", 8, FontStyle::Regular);

	int totalActual = totalGenerados + (alfaGenerado ? 1 : 0);
	int capturadosTotal = capturadosNormales() + capturadosAlfa;

	g->DrawString(String::Format("Tiempo: {0} s", getTiempoSegundos()), f, Brushes::White, 15, 11);
	g->DrawString(String::Format("Capturados: {0} / {1}", capturadosTotal, totalActual), f, Brushes::LightGreen, 160, 11);
	g->DrawString(String::Format("Dardos: {0}", guarda->getDardos()), f, Brushes::Orange, 360, 11);
	g->DrawString(String::Format("Vidas: {0}", guarda->getVidas()), f, Brushes::Red, 490, 11);

	if (alfaGenerado && alfa != nullptr && !alfa->estaCapturado())
		g->DrawString("ALFA SUELTO!", f, Brushes::OrangeRed, 610, 11);

	g->DrawString("[Flechas/WASD] Mover   [Espacio] Disparar   [P] Pteranodones",
		fChica, Brushes::Gray, 770, 15);

	delete f;
	delete fChica;
}

void Controlador::dibujarMensajes(Graphics^ g)
{
	if (mensajeTicks > 0)
	{
		Font^ f = gcnew Font("Segoe UI", 12, FontStyle::Bold);
		String^ txt = gcnew String(mensaje.c_str());
		Rectangle zj = zonaJuego();
		SizeF medida = g->MeasureString(txt, f);
		float mx = zj.X + (zj.Width - medida.Width) / 2.0f;
		float my = (float)(zj.Bottom - 45);
		SolidBrush^ fondo = gcnew SolidBrush(Color::FromArgb(170, 0, 0, 0));
		g->FillRectangle(fondo, mx - 8, my - 4, medida.Width + 16, medida.Height + 8);
		g->DrawString(txt, f, Brushes::Yellow, mx, my);
		delete fondo;
		delete f;
	}

	if (estado != JUGANDO)
	{
		String^ txt = (estado == GANADO) ? "PARQUE ASEGURADO" : "LOS DINOSAURIOS ESCAPARON";
		Font^ f = gcnew Font("Segoe UI", 30, FontStyle::Bold);
		SolidBrush^ velo = gcnew SolidBrush(Color::FromArgb(180, 0, 0, 0));
		g->FillRectangle(velo, 0, 0, ANCHO_VENTANA, ALTO_VENTANA);
		SizeF medida = g->MeasureString(txt, f);
		Brush^ color = (estado == GANADO) ? Brushes::LightGreen : Brushes::OrangeRed;
		g->DrawString(txt, f, color,
			(ANCHO_VENTANA - medida.Width) / 2.0f, (ALTO_VENTANA - medida.Height) / 2.0f);
		delete velo;
		delete f;
	}
}
