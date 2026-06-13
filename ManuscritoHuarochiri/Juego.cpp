#include "Juego.h"
#include <msclr/marshal.h>
#include <string.h>

Juego::Juego() {
	srand((unsigned)time(0));

	// Gestion de archivos: leer configuracion y validar (requisito del curso)
	gestor = new GestorArchivos();
	config = new Configuracion(gestor->cargarParametros());
	mejoresPuntajes = new vector<Puntaje>(gestor->cargarPuntajes());
	puntajeGuardado = false;
	nombreJugador = Environment::UserName;
	if (gestor->huboError()) {
		MessageBox::Show(gcnew String(gestor->getUltimoError().c_str()),
			"Gestion de archivos",
			MessageBoxButtons::OK, MessageBoxIcon::Warning);
		gestor->limpiarError();
	}

	// Imagenes
	imgDeidadLluvia = gcnew Bitmap("imgPersonajes/deidadLluvia.png");
	imgDeidadEspejo = safe_cast<Bitmap^>(imgDeidadLluvia->Clone());
	imgDeidadEspejo->RotateFlip(RotateFlipType::RotateNoneFlipX);
	imgEscenario1 = gcnew Bitmap("imgNiveles/escenarioNivel1.png");
	imgEscenario2 = gcnew Bitmap("imgNiveles/escenarioNivel2.png");
	fondoActual = imgEscenario1;

	// Audio
	reproductorMusica = gcnew WMPLib::WindowsMediaPlayerClass();
	reproductorEfectos = gcnew WMPLib::WindowsMediaPlayerClass();
	reproductorMusica->settings->volume = 55;
	reproductorEfectos->settings->volume = 85;

	// Tipografias y pinceles
	fuenteTitulo = gcnew Font("Consolas", 46, FontStyle::Bold);
	fuenteGrande = gcnew Font("Consolas", 25, FontStyle::Bold);
	fuenteNormal = gcnew Font("Consolas", 16, FontStyle::Regular);
	fuenteChica = gcnew Font("Consolas", 12, FontStyle::Regular);
	velOscuro = gcnew SolidBrush(Color::FromArgb(215, 8, 12, 16));
	velRojo = gcnew SolidBrush(Color::FromArgb(190, 35, 0, 0));
	velCorrupcion = gcnew SolidBrush(Color::FromArgb(42, 255, 30, 30));
	brochaGlobo = gcnew SolidBrush(Color::FromArgb(240, 248, 240, 214));
	brochaTextoGlobo = gcnew SolidBrush(Color::FromArgb(62, 40, 18));
	plumaGlobo = gcnew Pen(Color::FromArgb(110, 78, 38), 3.0f);
	centrado = gcnew StringFormat();
	centrado->Alignment = StringAlignment::Center;
	centrado->LineAlignment = StringAlignment::Center;

	// Entidades
	jugador = nullptr;
	enemigos = new vector<LlamaDigital*>();
	nodos = new vector<NodoDato*>();
	pilares = new vector<PilarInformacion*>();
	rayos = new vector<RayoTrace*>();
	decoraciones = new vector<Decoracion*>();
	espiritus = new vector<PersonajeSecundario*>();
	bolasFuego = new vector<BolaFuego*>();
	particulas = new vector<Particula*>();
	textosFlotantes = new vector<TextoFlotante*>();
	jefe = nullptr;
	santuario = nullptr;
	aliado = nullptr;

	score = 0;
	scoreInicioNivel = 0;
	tickGlobal = 0;
	poderDesbloqueado = false;
	salirAlMenu = false;
	mensaje = "";
	mensajeTicks = 0;
	fragmentoQuechua = "";
	fragmentoTraduccion = "";
	fragmentoTicks = 0;
	tituloNivel = "";
	velJugadorX = velJugadorY = 0;
	sacudida = 0;

	cargarTextos();
	iniciarNivel(1);
}

Juego::~Juego() {
	detenerMusica();
	limpiarNivel();
	if (jugador != nullptr) { delete jugador; jugador = nullptr; }
	delete enemigos;
	delete nodos;
	delete pilares;
	delete rayos;
	delete decoraciones;
	delete espiritus;
	delete bolasFuego;
	delete particulas;
	delete textosFlotantes;
	delete mejoresPuntajes;
	delete config;
	delete gestor;
}

// Construye un registro con el nombre del jugador, su puntaje y la fecha,
// y lo agrega a SCORES.bin. Solo se guarda una vez por partida terminada.
void Juego::registrarPuntaje() {
	if (puntajeGuardado) return;
	puntajeGuardado = true;

	Puntaje p;
	msclr::interop::marshal_context ctx;
	const char* nombre = ctx.marshal_as<const char*>(nombreJugador);
	const char* fecha = ctx.marshal_as<const char*>(DateTime::Now.ToString("yyyy-MM-dd"));
	strncpy_s(p.nombre, sizeof(p.nombre), nombre, _TRUNCATE);
	strncpy_s(p.fecha, sizeof(p.fecha), fecha, _TRUNCATE);
	p.puntaje = score;

	gestor->guardarPuntaje(p);
	*mejoresPuntajes = gestor->cargarPuntajes();
	if (gestor->huboError()) gestor->limpiarError();
}

// Crea config->enemigos[nivel-1] llamas de patrulla en posiciones aleatorias,
// evitando el punto de aparicion del jugador. Los enemigos especiales
// (golem, lobo, jefe) se colocan aparte en iniciarNivel.
void Juego::generarEnemigos(int nivel, int W, int H) {
	int cantidad = config->enemigos[nivel - 1];
	int velocidad = (nivel == 2) ? 4 : 3;
	int radio = 360 + nivel * 30;
	for (int i = 0; i < cantidad; i++) {
		int ex, ey, intentos = 0;
		do {
			ex = 200 + rand() % (W - 400);
			ey = 200 + rand() % (H - 400);
			intentos++;
		} while (intentos < 20 &&
			abs(ex - spawnX) < 500 && abs(ey - spawnY) < 500);
		enemigos->push_back(new LlamaDigital(ex, ey, velocidad, radio));
	}
}

void Juego::cargarTextos() {
	// Fragmentos del Manuscrito de Huarochiri en quechua y su traduccion.
	quechua = gcnew array<String^> {
		"Runa yndio niscap machoncuna...",
		"Paria Caca, apu qasapi paqarimuq",
		"Huallallo Carhuincho ninawan kawsarqan",
		"Chaupi Namca, tukuy willakuykunap maman",
		"Yakuqa kawsaymi, ama qunqaychu",
		"Apukunap siminta waqaychay",
		"Kay qillqasqa winaypaqmi"
	};
	traducciones = gcnew array<String^> {
		"Los antepasados de los hombres llamados indios...",
		"Pariacaca, senor nacido en la montana nevada",
		"Huallallo Carhuincho vivia con el fuego",
		"Chaupinamca, madre de todos los relatos",
		"El agua es vida, no la olvides",
		"Guarda la palabra de los Apus",
		"Lo escrito aqui es para siempre"
	};

	// Lluvia de texto para la pantalla de victoria
	lluviaY = gcnew array<int>(14);
	lluviaVel = gcnew array<int>(14);
	for (int i = 0; i < 14; i++) {
		lluviaY[i] = rand() % 1200;
		lluviaVel[i] = 3 + rand() % 7;
	}
}

// ------------------------------------------------------------------
//  Niveles
// ------------------------------------------------------------------

void Juego::limpiarNivel() {
	for (int i = 0; i < (int)enemigos->size(); i++) delete enemigos->at(i);
	enemigos->clear();
	for (int i = 0; i < (int)nodos->size(); i++) delete nodos->at(i);
	nodos->clear();
	for (int i = 0; i < (int)pilares->size(); i++) delete pilares->at(i);
	pilares->clear();
	for (int i = 0; i < (int)rayos->size(); i++) delete rayos->at(i);
	rayos->clear();
	for (int i = 0; i < (int)decoraciones->size(); i++) delete decoraciones->at(i);
	decoraciones->clear();
	for (int i = 0; i < (int)espiritus->size(); i++) delete espiritus->at(i);
	espiritus->clear();
	for (int i = 0; i < (int)bolasFuego->size(); i++) delete bolasFuego->at(i);
	bolasFuego->clear();
	for (int i = 0; i < (int)particulas->size(); i++) delete particulas->at(i);
	particulas->clear();
	for (int i = 0; i < (int)textosFlotantes->size(); i++) delete textosFlotantes->at(i);
	textosFlotantes->clear();
	if (jefe != nullptr) { delete jefe; jefe = nullptr; }
	if (santuario != nullptr) { delete santuario; santuario = nullptr; }
	if (aliado != nullptr) { delete aliado; aliado = nullptr; }
}

void Juego::iniciarNivel(int numero) {
	limpiarNivel();
	nivelActual = numero;
	estado = Estado::IntroNivel;
	scoreInicioNivel = score;
	nodosRecogidos = 0;
	ticksTrasMuerte = 0;
	cooldownRayo = 0;
	spawnEnemigos = 0;
	contadorBolaFuego = 0;
	velJugadorX = velJugadorY = 0;
	sacudida = 0;
	puntajeGuardado = false;
	teclaArriba = teclaAbajo = teclaIzquierda = teclaDerecha = teclaInteractuar = false;

	fondoActual = (numero == 2) ? imgEscenario2 : imgEscenario1;
	int W = fondoActual->Width;
	int H = fondoActual->Height;

	if (jugador != nullptr) delete jugador;
	jugador = new DeidadLluvia(imgDeidadLluvia);
	jugador->setConPoder(poderDesbloqueado);
	jugador->setVida(config->vidas); // vidas leidas de PARAMETERS.txt
	int anchoJ = jugador->getAncho();
	int altoJ = jugador->getAlto();

	if (numero == 1) {
		// Recuperar los 7 nodos de datos esquivando las llamas digitales
		tituloNivel = "RECONSTRUCCION EN LA RED";
		tiempoRestante = config->tiempo[0] * 33;
		spawnX = (int)(0.50 * W) - anchoJ / 2;
		spawnY = (int)(0.82 * H) - altoJ / 2;

		double nodoX[7] = { 0.08, 0.34, 0.63, 0.90, 0.09, 0.50, 0.88 };
		double nodoY[7] = { 0.12, 0.07, 0.10, 0.16, 0.56, 0.47, 0.62 };
		for (int n = 0; n < config->nodosNivel1; n++)
			nodos->push_back(new NodoDato((int)(nodoX[n] * W), (int)(nodoY[n] * H), n));

		generarEnemigos(1, W, H); // cantidad segun PARAMETERS.txt
		enemigos->push_back(new GolemLava((int)(0.88 * W), (int)(0.60 * H)));   // custodia un nodo lejano

		// Ambiente: lava viva en la zona volcanica, rio brillante, antorchas
		decoraciones->push_back(new BurbujaLava((int)(0.78 * W), (int)(0.10 * H)));
		decoraciones->push_back(new BurbujaLava((int)(0.88 * W), (int)(0.22 * H)));
		decoraciones->push_back(new BurbujaLava((int)(0.70 * W), (int)(0.30 * H)));
		decoraciones->push_back(new BurbujaLava((int)(0.93 * W), (int)(0.40 * H)));
		decoraciones->push_back(new DestelloAgua((int)(0.18 * W), (int)(0.18 * H)));
		decoraciones->push_back(new DestelloAgua((int)(0.27 * W), (int)(0.33 * H)));
		decoraciones->push_back(new DestelloAgua((int)(0.30 * W), (int)(0.62 * H)));
		decoraciones->push_back(new DestelloAgua((int)(0.13 * W), (int)(0.75 * H)));
		decoraciones->push_back(new AntorchaDecorativa((int)(0.45 * W), (int)(0.42 * H)));
		decoraciones->push_back(new AntorchaDecorativa((int)(0.55 * W), (int)(0.42 * H)));
		decoraciones->push_back(new AntorchaDecorativa((int)(0.47 * W), (int)(0.88 * H)));
		decoraciones->push_back(new AntorchaDecorativa((int)(0.53 * W), (int)(0.88 * H)));

		// Espiritus ancestrales que ensenan la historia
		espiritus->push_back(new PersonajeSecundario((int)(0.42 * W), (int)(0.78 * H), anchoJ, altoJ));
		espiritus->push_back(new PersonajeSecundario((int)(0.13 * W), (int)(0.44 * H), anchoJ, altoJ));
		textosEspiritus = gcnew array<String^> {
			"Soy la memoria de Tomas, el escribano. El Manuscrito de Huarochiri se escribio hacia 1608 en quechua: es el unico libro que cuenta los mitos andinos en su propia voz.",
			"Pariacaca, senor de las aguas, nacio de cinco huevos en la montana. Recupera sus 7 fragmentos antes de que el fuego de Huallallo los borre."
		};
	}
	else if (numero == 2) {
		// Escoltar la ofrenda digital hasta el Santuario de Macahuisa
		tituloNivel = "LA OFRENDA DIGITAL";
		tiempoRestante = config->tiempo[1] * 33;
		spawnX = (int)(0.10 * W) - anchoJ / 2;
		spawnY = (int)(0.86 * H) - altoJ / 2;
		jugador->setConOfrenda(true);

		santuario = new SantuarioMacahuisa((int)(0.50 * W), (int)(0.45 * H));
		aliado = new AliadoAlgoritmo();
		aliado->setX(spawnX - 80);
		aliado->setY(spawnY - 60);

		generarEnemigos(2, W, H); // cantidad segun PARAMETERS.txt
		enemigos->push_back(new LoboLava((int)(0.27 * W), (int)(0.70 * H)));  // cazador rapido
		enemigos->push_back(new LoboLava((int)(0.72 * W), (int)(0.28 * H)));  // cazador rapido

		// Ambiente: el sol de los Andes, rio chispeante y antorchas del santuario
		decoraciones->push_back(new SolAndino((int)(0.47 * W), (int)(0.05 * H)));
		decoraciones->push_back(new DestelloAgua((int)(0.42 * W), (int)(0.25 * H)));
		decoraciones->push_back(new DestelloAgua((int)(0.36 * W), (int)(0.45 * H)));
		decoraciones->push_back(new DestelloAgua((int)(0.40 * W), (int)(0.68 * H)));
		decoraciones->push_back(new DestelloAgua((int)(0.30 * W), (int)(0.80 * H)));
		decoraciones->push_back(new AntorchaDecorativa((int)(0.43 * W), (int)(0.46 * H)));
		decoraciones->push_back(new AntorchaDecorativa((int)(0.57 * W), (int)(0.46 * H)));
		decoraciones->push_back(new BurbujaLava((int)(0.80 * W), (int)(0.30 * H)));
		decoraciones->push_back(new BurbujaLava((int)(0.88 * W), (int)(0.55 * H)));
		decoraciones->push_back(new BurbujaLava((int)(0.75 * W), (int)(0.70 * H)));

		espiritus->push_back(new PersonajeSecundario((int)(0.17 * W), (int)(0.74 * H), anchoJ, altoJ));
		espiritus->push_back(new PersonajeSecundario((int)(0.45 * W), (int)(0.60 * H), anchoJ, altoJ));
		textosEspiritus = gcnew array<String^> {
			"Las huacas eran seres sagrados del paisaje: piedras, lagunas, montanas. Oblivion Corp las llama 'datos basura'... nosotros las llamamos memoria.",
			"Macahuisa, hijo de Pariacaca, defendio a los pueblos del valle. Entregale tu ofrenda: el patrimonio vivo es poder, no una pieza de museo."
		};
	}
	else {
		// Activar los 3 pilares de informacion protegidos por Huallallo
		tituloNivel = "EL CODIGO MADRE";
		tiempoRestante = config->tiempo[2] * 33;
		spawnX = (int)(0.50 * W) - anchoJ / 2;
		spawnY = (int)(0.82 * H) - altoJ / 2;

		double pilarX[3] = { 0.13, 0.86, 0.15 };
		double pilarY[3] = { 0.14, 0.18, 0.78 };
		for (int n = 0; n < config->pilaresNivel3; n++)
			pilares->push_back(new PilarInformacion((int)(pilarX[n] * W), (int)(pilarY[n] * H)));

		jefe = new HuallalloJefe((int)(0.52 * W), (int)(0.42 * H));

		generarEnemigos(3, W, H); // cantidad segun PARAMETERS.txt
		enemigos->push_back(new GolemLava((int)(0.45 * W), (int)(0.30 * H)));  // tanque que estorba la carga
		enemigos->push_back(new LoboLava((int)(0.62 * W), (int)(0.62 * H)));   // cazador rapido

		// Ambiente corrupto: mas lava viva y antorchas junto a cada pilar
		decoraciones->push_back(new BurbujaLava((int)(0.78 * W), (int)(0.10 * H)));
		decoraciones->push_back(new BurbujaLava((int)(0.88 * W), (int)(0.26 * H)));
		decoraciones->push_back(new BurbujaLava((int)(0.70 * W), (int)(0.34 * H)));
		decoraciones->push_back(new BurbujaLava((int)(0.93 * W), (int)(0.44 * H)));
		decoraciones->push_back(new BurbujaLava((int)(0.62 * W), (int)(0.70 * H)));
		decoraciones->push_back(new AntorchaDecorativa((int)(0.13 * W) + 150, (int)(0.14 * H) + 40));
		decoraciones->push_back(new AntorchaDecorativa((int)(0.86 * W) - 90, (int)(0.18 * H) + 40));
		decoraciones->push_back(new AntorchaDecorativa((int)(0.15 * W) + 150, (int)(0.78 * H) + 40));
		decoraciones->push_back(new DestelloAgua((int)(0.27 * W), (int)(0.33 * H)));
		decoraciones->push_back(new DestelloAgua((int)(0.30 * W), (int)(0.62 * H)));

		espiritus->push_back(new PersonajeSecundario((int)(0.56 * W), (int)(0.76 * H), anchoJ, altoJ));
		espiritus->push_back(new PersonajeSecundario((int)(0.20 * W), (int)(0.24 * H), anchoJ, altoJ));
		textosEspiritus = gcnew array<String^> {
			"Chaupinamca, madre de los relatos, une todas las historias en una sola red. Si su codigo cae, el manuscrito quedara mudo para siempre.",
			"El acceso abierto es la nueva chakana: un puente para que el saber cruce generaciones. Compila los 3 pilares y libera el conocimiento."
		};
	}

	jugador->setX(spawnX);
	jugador->setY(spawnY);
	detenerMusica();
}

// ------------------------------------------------------------------
//  Audio y mensajes
// ------------------------------------------------------------------

void Juego::reproducirEfecto(String^ ruta) {
	reproductorEfectos->URL = ruta;
	reproductorEfectos->controls->play();
}

void Juego::tocarMusicaNivel() {
	reproductorMusica->URL = "msc/sonidoNivel.wav";
	reproductorMusica->settings->setMode("loop", true);
	reproductorMusica->controls->play();
}

void Juego::detenerMusica() {
	reproductorMusica->controls->stop();
}

void Juego::mostrarMensaje(String^ texto) {
	mensaje = texto;
	mensajeTicks = 110;
}

// Estallido de chispas en (px,py): un anillo de particulas que salen volando.
void Juego::estallido(int px, int py, int cantidad, int tono) {
	for (int i = 0; i < cantidad; i++) {
		double ang = (3.14159 * 2.0 * i) / cantidad + (rand() % 100) / 200.0;
		double rapidez = 2.0 + (rand() % 40) / 10.0;
		particulas->push_back(new Particula(px, py,
			cos(ang) * rapidez, sin(ang) * rapidez - 2.0,
			18 + rand() % 14, tono));
	}
}

// Suma puntos y lanza un "+valor" flotante en (px,py).
void Juego::sumarPuntos(int px, int py, int valor, int tono) {
	score += valor;
	textosFlotantes->push_back(new TextoFlotante(px, py, valor, tono));
}

// ------------------------------------------------------------------
//  Entrada
// ------------------------------------------------------------------

void Juego::teclaCambiada(Keys tecla, bool presionada) {
	// Teclas mantenidas (movimiento e interaccion)
	if (tecla == Keys::W || tecla == Keys::Up) teclaArriba = presionada;
	if (tecla == Keys::S || tecla == Keys::Down) teclaAbajo = presionada;
	if (tecla == Keys::A || tecla == Keys::Left) teclaIzquierda = presionada;
	if (tecla == Keys::D || tecla == Keys::Right) teclaDerecha = presionada;
	if (tecla == Keys::E) teclaInteractuar = presionada;

	if (!presionada) return;

	// Teclas de un solo toque
	if (tecla == Keys::Enter) {
		if (estado == Estado::IntroNivel) {
			estado = Estado::Jugando;
			tocarMusicaNivel();
		}
		else if (estado == Estado::NivelCompletado) {
			if (nivelActual < 3) iniciarNivel(nivelActual + 1);
		}
		else if (estado == Estado::Victoria || estado == Estado::Derrota) {
			salirAlMenu = true;
		}
	}
	if (tecla == Keys::R && estado == Estado::Derrota) {
		score = scoreInicioNivel;
		iniciarNivel(nivelActual);
	}
	if (tecla == Keys::P) {
		if (estado == Estado::Jugando) {
			estado = Estado::Pausa;
			reproductorMusica->controls->pause();
		}
		else if (estado == Estado::Pausa) {
			estado = Estado::Jugando;
			reproductorMusica->controls->play();
		}
	}
	if (tecla == Keys::K || tecla == Keys::Space) {
		dispararRayo();
	}
}

// ------------------------------------------------------------------
//  Logica por tick
// ------------------------------------------------------------------

void Juego::dispararRayo() {
	if (estado != Estado::Jugando) return;
	if (jugador->getAccion() == DeidadLluvia::muerte) return;
	if (!jugador->getConPoder()) {
		mostrarMensaje("Aun no tienes el Poder de los Apus (entrega la ofrenda en el nivel 2)");
		return;
	}
	if (cooldownRayo > 0) return;
	cooldownRayo = 18;
	jugador->setAccion(DeidadLluvia::ataque);
	rayos->push_back(new RayoTrace(jugador->centroX(), jugador->centroY(),
		jugador->getMiradaX(), jugador->getMiradaY()));
	reproducirEfecto("msc/sonidoRayo.wav");
}

void Juego::perderVidaJugador(int golpeDesdeX, int golpeDesdeY) {
	jugador->recibirGolpe();
	reproducirEfecto("msc/sonidoGolpe.wav");
	sacudida = 12;

	if (jugador->getVida() > 0) {
		if (nivelActual == 2) {
			// En la escolta, un golpe te regresa al inicio del camino
			jugador->setX(spawnX);
			jugador->setY(spawnY);
			velJugadorX = velJugadorY = 0;
			mostrarMensaje("La ofrenda es sagrada: el camino comienza de nuevo");
		}
		else {
			// Empujon en direccion contraria al golpe
			double difX = (double)(jugador->centroX() - golpeDesdeX);
			double difY = (double)(jugador->centroY() - golpeDesdeY);
			double dist = sqrt(difX * difX + difY * difY);
			if (dist < 1) dist = 1;
			int nx = jugador->getX() + (int)(difX / dist * 130);
			int ny = jugador->getY() + (int)(difY / dist * 130);
			if (nx < 0) nx = 0;
			if (ny < 0) ny = 0;
			if (nx + jugador->getAncho() > fondoActual->Width) nx = fondoActual->Width - jugador->getAncho();
			if (ny + jugador->getAlto() > fondoActual->Height) ny = fondoActual->Height - jugador->getAlto();
			jugador->setX(nx);
			jugador->setY(ny);
		}
	}
}

void Juego::completarNivel() {
	// Bonus por el tiempo restante
	score += (tiempoRestante / 33) * 2;
	detenerMusica();
	reproducirEfecto("msc/sonidoVictoria.wav");
	if (nivelActual >= 3) estado = Estado::Victoria;
	else estado = Estado::NivelCompletado;
}

void Juego::actualizar() {
	tickGlobal++;
	if (mensajeTicks > 0) mensajeTicks--;
	if (fragmentoTicks > 0) fragmentoTicks--;
	// Al terminar la partida, registrar el puntaje en SCORES.bin (una sola vez)
	if (estado == Estado::Victoria || estado == Estado::Derrota) registrarPuntaje();
	if (estado == Estado::Jugando) actualizarJugando();
}

void Juego::actualizarJugando() {
	if (cooldownRayo > 0) cooldownRayo--;

	// Contador de borrado del servidor
	tiempoRestante--;
	if (tiempoRestante <= 0) {
		tiempoRestante = 0;
		detenerMusica();
		reproducirEfecto("msc/sonidoDerrota.wav");
		mostrarMensaje("El contador llego a cero: los datos fueron borrados");
		estado = Estado::Derrota;
		return;
	}

	// El ambiente y los espiritus siempre estan vivos (llamada polimorfica)
	for (int i = 0; i < (int)decoraciones->size(); i++) decoraciones->at(i)->animar();
	for (int i = 0; i < (int)espiritus->size(); i++) espiritus->at(i)->animar();

	// Particulas y textos flotantes (se animan aunque el jugador este muriendo)
	for (int i = (int)particulas->size() - 1; i >= 0; i--) {
		particulas->at(i)->animar();
		if (particulas->at(i)->terminado()) {
			delete particulas->at(i);
			particulas->erase(particulas->begin() + i);
		}
	}
	for (int i = (int)textosFlotantes->size() - 1; i >= 0; i--) {
		textosFlotantes->at(i)->animar();
		if (textosFlotantes->at(i)->terminado()) {
			delete textosFlotantes->at(i);
			textosFlotantes->erase(textosFlotantes->begin() + i);
		}
	}

	// Si el jugador esta muriendo, solo se termina su animacion
	if (jugador->getAccion() == DeidadLluvia::muerte) {
		jugador->actualizarAnimacion();
		if (jugador->getAnimTerminada()) {
			ticksTrasMuerte++;
			if (ticksTrasMuerte > 25) {
				detenerMusica();
				reproducirEfecto("msc/sonidoDerrota.wav");
				estado = Estado::Derrota;
			}
		}
		return;
	}

	// ----- Movimiento del jugador con aceleracion (se siente fluido) -----
	int velMaxima = jugador->getConOfrenda() ? 6 : 8;
	int objetivoX = 0, objetivoY = 0;
	if (teclaIzquierda) objetivoX -= velMaxima;
	if (teclaDerecha) objetivoX += velMaxima;
	if (teclaArriba) objetivoY -= velMaxima;
	if (teclaAbajo) objetivoY += velMaxima;
	if (objetivoX != 0 && objetivoY != 0) { // diagonal pareja
		objetivoX = objetivoX * 3 / 4;
		objetivoY = objetivoY * 3 / 4;
	}
	if (velJugadorX < objetivoX) { velJugadorX += 2; if (velJugadorX > objetivoX) velJugadorX = objetivoX; }
	else if (velJugadorX > objetivoX) { velJugadorX -= 2; if (velJugadorX < objetivoX) velJugadorX = objetivoX; }
	if (velJugadorY < objetivoY) { velJugadorY += 2; if (velJugadorY > objetivoY) velJugadorY = objetivoY; }
	else if (velJugadorY > objetivoY) { velJugadorY -= 2; if (velJugadorY < objetivoY) velJugadorY = objetivoY; }

	jugador->setDX(velJugadorX);
	jugador->setDY(velJugadorY);
	jugador->actualizarAccionPorMovimiento();
	jugador->moverEnMundo(fondoActual->Width, fondoActual->Height);
	if (jugador->getAccion() == DeidadLluvia::ataque && jugador->getAnimTerminada())
		jugador->setAccion(DeidadLluvia::reposo);
	jugador->actualizarAnimacion();

	// ----- Enemigos -----
	for (int i = 0; i < (int)enemigos->size(); i++) {
		LlamaDigital* enemigo = enemigos->at(i);
		if (!enemigo->getActivo()) continue;
		enemigo->perseguir(jugador->centroX(), jugador->centroY());
		if (!jugador->esInvulnerable() &&
			enemigo->hitbox().IntersectsWith(jugador->hitbox())) {
			perderVidaJugador(enemigo->centroX(), enemigo->centroY());
		}
	}

	// ----- Bolas de fuego de Huallallo -----
	for (int i = (int)bolasFuego->size() - 1; i >= 0; i--) {
		BolaFuego* bola = bolasFuego->at(i);
		bola->mover(nullptr);
		if (bola->getActivo() && !jugador->esInvulnerable() &&
			bola->hitbox().IntersectsWith(jugador->hitbox())) {
			bola->setActivo(false);
			perderVidaJugador(bola->centroX(), bola->centroY());
		}
		if (bola->terminado()) {
			delete bola;
			bolasFuego->erase(bolasFuego->begin() + i);
		}
	}

	// ----- Rayos Ray-Trace -----
	for (int i = (int)rayos->size() - 1; i >= 0; i--) {
		RayoTrace* rayo = rayos->at(i);
		rayo->mover(nullptr);

		for (int j = 0; j < (int)enemigos->size(); j++) {
			LlamaDigital* enemigo = enemigos->at(j);
			if (rayo->getActivo() && enemigo->getActivo() &&
				rayo->hitbox().IntersectsWith(enemigo->hitbox())) {
				rayo->setActivo(false);
				reproducirEfecto("msc/sonidoGolpe.wav");
				bool murio = enemigo->recibirImpacto();
				if (murio) {
					estallido(enemigo->centroX(), enemigo->centroY(), 14, enemigo->colorParticula());
					sumarPuntos(enemigo->centroX(), enemigo->getY(), 50, 1);
				}
				else {
					// Tanque que aun resiste: solo chispas
					estallido(enemigo->centroX(), enemigo->centroY(), 6, 1);
				}
			}
		}
		// El rayo tambien desintegra bolas de fuego
		for (int j = 0; j < (int)bolasFuego->size(); j++) {
			BolaFuego* bola = bolasFuego->at(j);
			if (rayo->getActivo() && bola->getActivo() &&
				rayo->hitbox().IntersectsWith(bola->hitbox())) {
				bola->setActivo(false);
				rayo->setActivo(false);
				estallido(bola->centroX(), bola->centroY(), 8, 1);
				sumarPuntos(bola->centroX(), bola->getY(), 25, 1);
			}
		}
		if (nivelActual == 3 && jefe != nullptr && jefe->getActivo() && rayo->getActivo() &&
			rayo->hitbox().IntersectsWith(jefe->hitbox())) {
			jefe->recibirRayo();
			rayo->setActivo(false);
			reproducirEfecto("msc/sonidoGolpe.wav");
			estallido(jefe->centroX(), jefe->centroY(), 10, 1);
			if (!jefe->getActivo()) {
				estallido(jefe->centroX(), jefe->centroY(), 40, 0);
				sumarPuntos(jefe->centroX(), jefe->getY(), 500, 0);
				mostrarMensaje("HUALLALLO DESFRAGMENTADO: los pilares estan a salvo");
			}
		}
		if (rayo->terminado()) {
			delete rayo;
			rayos->erase(rayos->begin() + i);
		}
	}

	// ----- Logica especifica del nivel -----
	if (nivelActual == 1) actualizarNivel1();
	else if (nivelActual == 2) actualizarNivel2();
	else actualizarNivel3();
}

void Juego::actualizarNivel1() {
	for (int i = 0; i < (int)nodos->size(); i++) {
		NodoDato* nodo = nodos->at(i);
		if (!nodo->getActivo()) continue;
		nodo->animar();
		if (jugador->hitbox().IntersectsWith(nodo->area())) {
			nodo->setActivo(false);
			nodosRecogidos++;
			sumarPuntos(nodo->centroX(), nodo->getY(), 100, 1);
			estallido(nodo->centroX(), nodo->centroY(), 12, 1);
			reproducirEfecto("msc/sonidoNodo.wav");
			fragmentoQuechua = quechua[nodo->getFragmentoID()];
			fragmentoTraduccion = traducciones[nodo->getFragmentoID()];
			fragmentoTicks = 170;
		}
	}
	if (nodosRecogidos >= (int)nodos->size()) completarNivel();
}

void Juego::actualizarNivel2() {
	santuario->animar();
	aliado->seguir(jugador->getX(), jugador->getY());

	if (jugador->getConOfrenda() && jugador->distanciaA(santuario) < 250) {
		jugador->setConOfrenda(false);
		poderDesbloqueado = true;
		jugador->setConPoder(true);
		sumarPuntos(santuario->centroX(), santuario->getY(), 500, 0);
		estallido(santuario->centroX(), santuario->centroY(), 30, 3);
		reproducirEfecto("msc/sonidoPilar.wav");
		mostrarMensaje("PODER DE LOS APUS DESBLOQUEADO: Ray-Trace (K o ESPACIO)");
		completarNivel();
	}
}

void Juego::actualizarNivel3() {
	// Pilares: carga del jugador
	int completos = 0;
	for (int i = 0; i < (int)pilares->size(); i++) {
		PilarInformacion* pilar = pilares->at(i);
		pilar->animar();
		pilar->setCorrompiendo(false);
		if (teclaInteractuar && !pilar->estaCompleto() && jugador->distanciaA(pilar) < 220) {
			pilar->cargar(0.8);
			if (pilar->estaCompleto()) {
				sumarPuntos(pilar->centroX(), pilar->getY(), 200, 2);
				estallido(pilar->centroX(), pilar->centroY(), 20, 2);
				reproducirEfecto("msc/sonidoPilar.wav");
				mostrarMensaje("PILAR COMPILADO: este fragmento ya es indestructible");
			}
		}
		if (pilar->estaCompleto()) completos++;
	}

	// Jefe: corrompe el pilar mas avanzado o persigue al jugador
	if (jefe != nullptr && jefe->getActivo()) {
		PilarInformacion* objetivo = nullptr;
		double mejor = -1;
		for (int i = 0; i < (int)pilares->size(); i++) {
			PilarInformacion* pilar = pilares->at(i);
			if (!pilar->estaCompleto() && pilar->getProgreso() > mejor) {
				mejor = pilar->getProgreso();
				objetivo = pilar;
			}
		}
		jefe->setCorrompiendo(false);
		if (objetivo != nullptr && mejor > 0) {
			jefe->perseguir(objetivo->centroX(), objetivo->centroY());
			if (jefe->distanciaA(objetivo) < 260) {
				objetivo->drenar(0.5);
				objetivo->setCorrompiendo(true);
				jefe->setCorrompiendo(true);
			}
		}
		else {
			jefe->perseguir(jugador->centroX(), jugador->centroY());
		}
		if (!jugador->esInvulnerable() &&
			jefe->hitbox().IntersectsWith(jugador->hitbox())) {
			perderVidaJugador(jefe->centroX(), jefe->centroY());
		}

		// Cada cierto tiempo lanza una bola de fuego hacia el jugador
		contadorBolaFuego++;
		if (contadorBolaFuego >= 170) {
			contadorBolaFuego = 0;
			if (jugador->distanciaA(jefe) > 280) {
				bolasFuego->push_back(new BolaFuego(jefe->centroX(), jefe->centroY(),
					jugador->centroX(), jugador->centroY()));
				reproducirEfecto("msc/sonidoRayo.wav");
			}
		}
	}

	// Esbirros que aparecen cerca de los pilares
	spawnEnemigos++;
	if (spawnEnemigos >= 240 && jefe != nullptr && jefe->getActivo()) {
		spawnEnemigos = 0;
		int activos = 0;
		for (int i = 0; i < (int)enemigos->size(); i++)
			if (enemigos->at(i)->getActivo()) activos++;
		if (activos < 4) {
			PilarInformacion* pilar = pilares->at(rand() % (int)pilares->size());
			int px = pilar->getX() + (rand() % 600) - 300;
			int py = pilar->getY() + (rand() % 600) - 300;
			if (px < 0) px = 0;
			if (py < 0) py = 0;
			enemigos->push_back(new LlamaDigital(px, py, 3, 460));
		}
	}

	if (completos >= (int)pilares->size()) completarNivel();
}

// ------------------------------------------------------------------
//  Dibujo
// ------------------------------------------------------------------

void Juego::calcularCamara(int anchoPantalla, int altoPantalla) {
	if (fondoActual->Width <= anchoPantalla) {
		camaraX = 0;
		offsetX = (anchoPantalla - fondoActual->Width) / 2;
	}
	else {
		camaraX = jugador->centroX() - anchoPantalla / 2;
		if (camaraX < 0) camaraX = 0;
		if (camaraX > fondoActual->Width - anchoPantalla) camaraX = fondoActual->Width - anchoPantalla;
		offsetX = 0;
	}
	if (fondoActual->Height <= altoPantalla) {
		camaraY = 0;
		offsetY = (altoPantalla - fondoActual->Height) / 2;
	}
	else {
		camaraY = jugador->centroY() - altoPantalla / 2;
		if (camaraY < 0) camaraY = 0;
		if (camaraY > fondoActual->Height - altoPantalla) camaraY = fondoActual->Height - altoPantalla;
		offsetY = 0;
	}

	// Sacudida al recibir dano (y se vuelve a encuadrar dentro del mapa)
	if (sacudida > 0) {
		sacudida--;
		camaraX += (rand() % 13) - 6;
		camaraY += (rand() % 13) - 6;
		if (camaraX < 0) camaraX = 0;
		if (camaraY < 0) camaraY = 0;
		if (fondoActual->Width > anchoPantalla && camaraX > fondoActual->Width - anchoPantalla)
			camaraX = fondoActual->Width - anchoPantalla;
		if (fondoActual->Height > altoPantalla && camaraY > fondoActual->Height - altoPantalla)
			camaraY = fondoActual->Height - altoPantalla;
	}
}

void Juego::dibujarMundo(Graphics^ g, int anchoPantalla, int altoPantalla) {
	int visW = (fondoActual->Width < anchoPantalla) ? fondoActual->Width : anchoPantalla;
	int visH = (fondoActual->Height < altoPantalla) ? fondoActual->Height : altoPantalla;
	Rectangle origen = Rectangle(camaraX, camaraY, visW, visH);
	Rectangle destino = Rectangle(offsetX, offsetY, visW, visH);
	g->DrawImage(fondoActual, destino, origen, GraphicsUnit::Pixel);

	// Velo de corrupcion en el nivel final
	if (nivelActual == 3) g->FillRectangle(velCorrupcion, destino);

	// Camara efectiva para las entidades (incluye el centrado)
	int cx = camaraX - offsetX;
	int cy = camaraY - offsetY;

	// Ambiente animado primero (queda "pegado" al suelo del mapa)
	for (int i = 0; i < (int)decoraciones->size(); i++)
		decoraciones->at(i)->mostrar(g, nullptr, cx, cy);

	if (santuario != nullptr) santuario->mostrar(g, nullptr, cx, cy);
	for (int i = 0; i < (int)pilares->size(); i++) pilares->at(i)->mostrar(g, nullptr, cx, cy);
	for (int i = 0; i < (int)nodos->size(); i++)
		if (nodos->at(i)->getActivo()) nodos->at(i)->mostrar(g, nullptr, cx, cy);
	for (int i = 0; i < (int)enemigos->size(); i++)
		if (enemigos->at(i)->getActivo()) enemigos->at(i)->mostrar(g, nullptr, cx, cy);
	if (jefe != nullptr) jefe->mostrar(g, nullptr, cx, cy);
	for (int i = 0; i < (int)bolasFuego->size(); i++) bolasFuego->at(i)->mostrar(g, nullptr, cx, cy);
	for (int i = 0; i < (int)rayos->size(); i++) rayos->at(i)->mostrar(g, nullptr, cx, cy);
	if (aliado != nullptr) aliado->mostrar(g, nullptr, cx, cy);

	// Espiritus ancestrales (con el sprite fantasma de la deidad)
	for (int i = 0; i < (int)espiritus->size(); i++)
		espiritus->at(i)->mostrar(g, imgDeidadLluvia, cx, cy);

	jugador->mostrar(g, imgDeidadLluvia, imgDeidadEspejo, cx, cy);

	// Chispas y puntajes flotantes por encima de todo el mundo
	for (int i = 0; i < (int)particulas->size(); i++)
		particulas->at(i)->mostrar(g, nullptr, cx, cy);
	for (int i = 0; i < (int)textosFlotantes->size(); i++)
		textosFlotantes->at(i)->mostrar(g, fuenteNormal, cx, cy);

	if (nivelActual == 2 && jugador->getConOfrenda()) dibujarFlechaGuia(g);

	// Ayuda contextual del nivel 3 sobre los pilares
	if (nivelActual == 3) {
		for (int i = 0; i < (int)pilares->size(); i++) {
			PilarInformacion* pilar = pilares->at(i);
			if (!pilar->estaCompleto() && jugador->distanciaA(pilar) < 220) {
				g->DrawString("MANTEN [E]", fuenteChica, Brushes::White,
					(float)(pilar->getX() - cx), (float)(pilar->getY() - cy - 44));
			}
		}
	}

	// Globos de dialogo de los espiritus cercanos
	for (int i = 0; i < (int)espiritus->size(); i++) {
		PersonajeSecundario* espiritu = espiritus->at(i);
		if (jugador->distanciaA(espiritu) < 280 && i < textosEspiritus->Length) {
			dibujarGlobo(g, anchoPantalla,
				espiritu->centroX() - cx,
				espiritu->getY() - cy - 36,
				textosEspiritus[i]);
		}
	}
}

void Juego::dibujarFlechaGuia(Graphics^ g) {
	// Flecha dorada desde el jugador hacia el santuario
	double difX = (double)(santuario->centroX() - jugador->centroX());
	double difY = (double)(santuario->centroY() - jugador->centroY());
	double dist = sqrt(difX * difX + difY * difY);
	if (dist < 300) return; // ya esta cerca
	double ux = difX / dist, uy = difY / dist;
	int baseX = jugador->centroX() - (camaraX - offsetX);
	int baseY = jugador->getY() - (camaraY - offsetY) - 40;
	int puntaX = baseX + (int)(ux * 110);
	int puntaY = baseY + (int)(uy * 110);
	Pen^ dorado = gcnew Pen(Color::Gold, 7.0f);
	g->DrawLine(dorado, baseX + (int)(ux * 55), baseY + (int)(uy * 55), puntaX, puntaY);
	// Punta de flecha
	double ang = atan2(uy, ux);
	Point p1 = Point(puntaX, puntaY);
	Point p2 = Point(puntaX - (int)(24 * cos(ang - 0.5)), puntaY - (int)(24 * sin(ang - 0.5)));
	Point p3 = Point(puntaX - (int)(24 * cos(ang + 0.5)), puntaY - (int)(24 * sin(ang + 0.5)));
	array<Point>^ punta = { p1, p2, p3 };
	g->FillPolygon(Brushes::Gold, punta);
	delete dorado;
}

void Juego::dibujarGlobo(Graphics^ g, int anchoPantalla, int centroX, int baseY, String^ texto) {
	// Mide el texto envuelto a un ancho maximo
	SizeF medida = g->MeasureString(texto, fuenteChica, 340);
	int gw = (int)medida.Width + 28;
	int gh = (int)medida.Height + 22;
	int gx = centroX - gw / 2;
	int gy = baseY - gh - 18;

	// Que no se salga de la pantalla
	if (gx < 8) gx = 8;
	if (gx + gw > anchoPantalla - 8) gx = anchoPantalla - 8 - gw;
	if (gy < 70) gy = 70; // debajo de la barra del HUD

	// Globo estilo pergamino
	g->FillRectangle(brochaGlobo, gx, gy, gw, gh);
	g->DrawRectangle(plumaGlobo, gx, gy, gw, gh);
	// Colita hacia el espiritu
	array<Point>^ cola = {
		Point(centroX - 10, gy + gh), Point(centroX + 10, gy + gh), Point(centroX, gy + gh + 16)
	};
	g->FillPolygon(brochaGlobo, cola);

	RectangleF zonaTexto = RectangleF((float)gx + 14, (float)gy + 11, 344.0f, (float)gh - 16);
	g->DrawString(texto, fuenteChica, brochaTextoGlobo, zonaTexto);
}

void Juego::dibujarMinimapa(Graphics^ g, int anchoPantalla, int altoPantalla) {
	// Caja proporcional al mundo, anclada arriba a la derecha
	int mw = 190;
	double esc = (double)mw / fondoActual->Width;
	int mh = (int)(fondoActual->Height * esc);
	if (mh > 150) { mh = 150; esc = (double)mh / fondoActual->Height; mw = (int)(fondoActual->Width * esc); }
	int mx = anchoPantalla - mw - 14;
	int my = 70;

	SolidBrush^ fondo = gcnew SolidBrush(Color::FromArgb(150, 0, 0, 0));
	g->FillRectangle(fondo, mx, my, mw, mh);
	delete fondo;
	g->DrawRectangle(Pens::Gold, mx, my, mw, mh);

	// Objetivos
	for (int i = 0; i < (int)nodos->size(); i++) {
		if (!nodos->at(i)->getActivo()) continue;
		g->FillRectangle(Brushes::Cyan,
			mx + (int)(nodos->at(i)->centroX() * esc) - 2,
			my + (int)(nodos->at(i)->centroY() * esc) - 2, 5, 5);
	}
	for (int i = 0; i < (int)pilares->size(); i++) {
		Brush^ c = pilares->at(i)->estaCompleto() ? Brushes::LimeGreen : Brushes::Cyan;
		g->FillRectangle(c,
			mx + (int)(pilares->at(i)->centroX() * esc) - 3,
			my + (int)(pilares->at(i)->centroY() * esc) - 3, 7, 7);
	}
	if (santuario != nullptr)
		g->FillRectangle(Brushes::Gold,
			mx + (int)(santuario->centroX() * esc) - 4,
			my + (int)(santuario->centroY() * esc) - 4, 9, 9);

	// Enemigos
	for (int i = 0; i < (int)enemigos->size(); i++) {
		if (!enemigos->at(i)->getActivo()) continue;
		g->FillRectangle(Brushes::Red,
			mx + (int)(enemigos->at(i)->centroX() * esc) - 2,
			my + (int)(enemigos->at(i)->centroY() * esc) - 2, 5, 5);
	}
	if (jefe != nullptr && jefe->getActivo())
		g->FillRectangle(Brushes::OrangeRed,
			mx + (int)(jefe->centroX() * esc) - 4,
			my + (int)(jefe->centroY() * esc) - 4, 9, 9);

	// Jugador
	g->FillRectangle(Brushes::White,
		mx + (int)(jugador->centroX() * esc) - 3,
		my + (int)(jugador->centroY() * esc) - 3, 6, 6);
}

void Juego::dibujarCorazon(Graphics^ g, int px, int py, bool lleno) {
	Brush^ color = lleno ? Brushes::Red : Brushes::DimGray;
	g->FillEllipse(color, px, py, 16, 16);
	g->FillEllipse(color, px + 13, py, 16, 16);
	array<Point>^ pico = { Point(px + 1, py + 11), Point(px + 28, py + 11), Point(px + 14, py + 28) };
	g->FillPolygon(color, pico);
}

void Juego::dibujarHUD(Graphics^ g, int anchoPantalla, int altoPantalla) {
	// ----- Barra superior estilo retro: NIVEL | TIEMPO | PUNTOS | VIDAS -----
	g->FillRectangle(velOscuro, 0, 0, anchoPantalla, 56);
	g->FillRectangle(Brushes::DarkGoldenrod, 0, 56, anchoPantalla, 4);

	g->DrawString(String::Format("NIVEL {0}: {1}", nivelActual, tituloNivel),
		fuenteNormal, Brushes::Gold, 18.0f, 16.0f);

	int segundos = tiempoRestante / 33;
	Brush^ colorTiempo = (segundos < 30) ? Brushes::Red : Brushes::Cyan;
	g->DrawString(String::Format("TIEMPO {0}:{1:D2}", segundos / 60, segundos % 60),
		fuenteNormal, colorTiempo, (float)(anchoPantalla - 700), 16.0f);

	g->DrawString(String::Format("PUNTOS: {0:D6}", score),
		fuenteNormal, Brushes::Gold, (float)(anchoPantalla - 520), 16.0f);

	g->DrawString("VIDAS:", fuenteNormal, Brushes::White, (float)(anchoPantalla - 280), 16.0f);
	for (int i = 0; i < 3; i++)
		dibujarCorazon(g, anchoPantalla - 180 + i * 38, 13, i < jugador->getVida());

	// Tira que muestra cuanto falta para el borrado del servidor
	int tiempoTotal = config->tiempo[nivelActual - 1];
	double proporcion = (double)segundos / tiempoTotal;
	if (proporcion > 1) proporcion = 1;
	g->FillRectangle(colorTiempo, 0, 60, (int)(anchoPantalla * proporcion), 5);

	// ----- Barra inferior con el objetivo -----
	g->FillRectangle(velOscuro, 0, altoPantalla - 54, anchoPantalla, 54);
	String^ objetivo = "";
	if (nivelActual == 1)
		objetivo = String::Format("NODOS DE DATOS: {0}/{1}  -  Recupera los fragmentos del manuscrito", nodosRecogidos, (int)nodos->size());
	else if (nivelActual == 2)
		objetivo = "Lleva la OFRENDA DIGITAL al Santuario de Macahuisa (sigue la flecha dorada)";
	else {
		int completos = 0;
		for (int i = 0; i < (int)pilares->size(); i++)
			if (pilares->at(i)->estaCompleto()) completos++;
		objetivo = String::Format("PILARES COMPILADOS: {0}/{1}  -  Manten E junto a un pilar; no dejes que Huallallo los drene", completos, (int)pilares->size());
	}
	g->DrawString(objetivo, fuenteNormal, Brushes::White, 20.0f, (float)(altoPantalla - 42));
	g->DrawString("P: pausa   ESC: menu", fuenteChica, Brushes::Gray,
		(float)(anchoPantalla - 230), (float)(altoPantalla - 36));

	// Tarjeta del fragmento quechua recuperado
	if (fragmentoTicks > 0 && estado == Estado::Jugando) {
		int cw = (int)(anchoPantalla * 0.62);
		int ch = 120;
		int cxx = anchoPantalla / 2 - cw / 2;
		int cyy = altoPantalla - 200;
		g->FillRectangle(velOscuro, cxx, cyy, cw, ch);
		g->DrawRectangle(Pens::Cyan, cxx, cyy, cw, ch);
		g->DrawString("NODO RECUPERADO - TRADUCCION AUTOMATICA", fuenteChica, Brushes::Cyan,
			RectangleF((float)cxx, (float)cyy + 8, (float)cw, 20.0f), centrado);
		g->DrawString(fragmentoQuechua, fuenteGrande, Brushes::Cyan,
			RectangleF((float)cxx, (float)cyy + 30, (float)cw, 44.0f), centrado);
		g->DrawString(fragmentoTraduccion, fuenteNormal, Brushes::White,
			RectangleF((float)cxx, (float)cyy + 76, (float)cw, 34.0f), centrado);
	}

	// Mensaje flotante
	if (mensajeTicks > 0) {
		RectangleF zona = RectangleF(0.0f, (float)(altoPantalla / 5), (float)anchoPantalla, 60.0f);
		RectangleF sombra = RectangleF(3.0f, (float)(altoPantalla / 5) + 3, (float)anchoPantalla, 60.0f);
		g->DrawString(mensaje, fuenteGrande, Brushes::Black, sombra, centrado);
		g->DrawString(mensaje, fuenteGrande, Brushes::Gold, zona, centrado);
	}
}

// Tabla de mejores puntajes leida de SCORES.bin (demuestra lectura binaria).
void Juego::dibujarTablaPuntajes(Graphics^ g, int anchoPantalla, float y) {
	g->DrawString("MEJORES PUNTAJES (FILES/SCORES.bin)", fuenteNormal, Brushes::Cyan,
		RectangleF(0.0f, y, (float)anchoPantalla, 30.0f), centrado);
	int filas = (int)mejoresPuntajes->size();
	if (filas > 5) filas = 5;
	if (filas == 0) {
		g->DrawString("(aun no hay puntajes registrados)", fuenteChica, Brushes::Gray,
			RectangleF(0.0f, y + 32, (float)anchoPantalla, 24.0f), centrado);
		return;
	}
	for (int i = 0; i < filas; i++) {
		Puntaje p = mejoresPuntajes->at(i);
		String^ nombre = gcnew String(p.nombre);
		String^ fecha = gcnew String(p.fecha);
		String^ linea = String::Format("{0}. {1,-16}  {2:D6}   {3}",
			i + 1, nombre, p.puntaje, fecha);
		g->DrawString(linea, fuenteNormal, Brushes::White,
			RectangleF(0.0f, y + 34 + i * 26, (float)anchoPantalla, 26.0f), centrado);
	}
}

void Juego::dibujarPanel(Graphics^ g, int anchoPantalla, int altoPantalla,
	String^ titulo, array<String^>^ lineas, String^ pie, Brush^ colorTitulo) {
	int pw = (int)(anchoPantalla * 0.74);
	if (pw > 1400) pw = 1400;
	int ph = 170 + lineas->Length * 46 + 70;
	int px = anchoPantalla / 2 - pw / 2;
	int py = altoPantalla / 2 - ph / 2;

	g->FillRectangle(velOscuro, px, py, pw, ph);
	Pen^ borde = gcnew Pen(Color::Gold, 4.0f);
	g->DrawRectangle(borde, px, py, pw, ph);
	delete borde;

	g->DrawString(titulo, fuenteTitulo, colorTitulo,
		RectangleF((float)px, (float)py + 20, (float)pw, 90.0f), centrado);

	for (int i = 0; i < lineas->Length; i++) {
		g->DrawString(lineas[i], fuenteNormal, Brushes::White,
			RectangleF((float)px + 30, (float)(py + 140 + i * 46), (float)pw - 60, 44.0f), centrado);
	}

	g->DrawString(pie, fuenteGrande, Brushes::Gold,
		RectangleF((float)px, (float)(py + ph - 64), (float)pw, 50.0f), centrado);
}

void Juego::dibujarOverlays(Graphics^ g, int anchoPantalla, int altoPantalla) {
	if (estado == Estado::IntroNivel) {
		String^ titulo;
		array<String^>^ lineas;
		if (nivelActual == 1) {
			titulo = "NIVEL 1: RECONSTRUCCION EN LA RED";
			lineas = gcnew array<String^> {
				"Ano 2026. Oblivion Corp lanzo un virus para borrar de internet",
				"las lenguas originarias y los mitos antiguos.",
				"El mito de Pariacaca se esta corrompiendo: recupera los 7 nodos",
				"de datos en quechua y esquiva las llamas digitales de Huallallo.",
				"Acercate a los espiritus dorados: ellos guardan la historia.",
				"",
				"WASD o FLECHAS: moverte    P: pausa    ESC: salir"
			};
		}
		else if (nivelActual == 2) {
			titulo = "NIVEL 2: LA OFRENDA DIGITAL";
			lineas = gcnew array<String^> {
				"Oblivion Corp bloqueo el acceso al conocimiento de las huacas.",
				"Lleva la ofrenda digital hasta el Santuario de Macahuisa.",
				"Si un firewall te toca, volveras al inicio del camino.",
				"El algoritmo ancestral te acompana y la flecha dorada te guia.",
				"",
				"Entregala intacta para recibir el PODER DE LOS APUS (Ray-Trace)"
			};
		}
		else {
			titulo = "NIVEL 3: EL CODIGO MADRE";
			lineas = gcnew array<String^> {
				"El jefe de Oblivion Corp intenta borrar a Chaupinamca,",
				"la deidad que une todos los relatos.",
				"Manten E junto a cada pilar para compilar el manuscrito.",
				"Huallallo drenara el pilar mas avanzado y lanza bolas de fuego:",
				"decide cual proteger y esquiva sus llamas.",
				"",
				"K o ESPACIO: Ray-Trace para desfragmentar a los enemigos"
			};
		}
		dibujarPanel(g, anchoPantalla, altoPantalla, titulo, lineas,
			"Presiona ENTER para comenzar", Brushes::Cyan);
	}
	else if (estado == Estado::Pausa) {
		g->FillRectangle(velOscuro, 0, 0, anchoPantalla, altoPantalla);
		g->DrawString("PAUSA", fuenteTitulo, Brushes::White,
			RectangleF(0.0f, (float)altoPantalla / 2 - 80, (float)anchoPantalla, 100.0f), centrado);
		g->DrawString("Presiona P para continuar", fuenteNormal, Brushes::Gray,
			RectangleF(0.0f, (float)altoPantalla / 2 + 30, (float)anchoPantalla, 40.0f), centrado);
	}
	else if (estado == Estado::NivelCompletado) {
		String^ pregunta1;
		String^ pregunta2;
		if (nivelActual == 1) {
			pregunta1 = "Si un mito se borra de internet,";
			pregunta2 = "realmente deja de existir en nuestra memoria?";
		}
		else {
			pregunta1 = "Es el patrimonio un objeto estatico";
			pregunta2 = "o una fuente de poder para el presente?";
		}
		array<String^>^ lineas = gcnew array<String^> {
			String::Format("SCORE: {0}", score),
			"",
			"PREGUNTA PARA REFLEXIONAR:",
			pregunta1,
			pregunta2
		};
		dibujarPanel(g, anchoPantalla, altoPantalla,
			String::Format("NIVEL {0} COMPLETADO", nivelActual), lineas,
			"Presiona ENTER para el siguiente nivel", Brushes::LimeGreen);
	}
	else if (estado == Estado::Victoria) {
		g->FillRectangle(velOscuro, 0, 0, anchoPantalla, altoPantalla);

		// Lluvia del manuscrito propagandose por la red
		for (int i = 0; i < 14; i++) {
			lluviaY[i] += lluviaVel[i];
			if (lluviaY[i] > altoPantalla + 60) lluviaY[i] = -40;
			int lx = (i * anchoPantalla) / 14 + 10;
			g->DrawString(quechua[i % 7], fuenteChica, Brushes::Green,
				(float)lx, (float)lluviaY[i]);
		}

		g->DrawString("YOU WIN", fuenteTitulo, Brushes::Gold,
			RectangleF(0.0f, (float)altoPantalla * 0.22f, (float)anchoPantalla, 110.0f), centrado);
		g->DrawString("El manuscrito fue compilado y subido a un servidor de acceso abierto.",
			fuenteNormal, Brushes::White,
			RectangleF(0.0f, (float)altoPantalla * 0.42f, (float)anchoPantalla, 40.0f), centrado);
		g->DrawString("Has asegurado que la voz de los ancestros sea indestructible en la red.",
			fuenteNormal, Brushes::White,
			RectangleF(0.0f, (float)altoPantalla * 0.47f, (float)anchoPantalla, 40.0f), centrado);
		g->DrawString("El conocimiento ahora es libre.", fuenteGrande, Brushes::Cyan,
			RectangleF(0.0f, (float)altoPantalla * 0.54f, (float)anchoPantalla, 50.0f), centrado);
		g->DrawString(String::Format("{0}  -  SCORE FINAL: {1}", nombreJugador, score),
			fuenteGrande, Brushes::Gold,
			RectangleF(0.0f, (float)altoPantalla * 0.63f, (float)anchoPantalla, 50.0f), centrado);
		dibujarTablaPuntajes(g, anchoPantalla, (float)altoPantalla * 0.71f);
		g->DrawString("ENTER: volver al menu", fuenteNormal, Brushes::Gold,
			RectangleF(0.0f, (float)altoPantalla * 0.90f, (float)anchoPantalla, 40.0f), centrado);
	}
	else if (estado == Estado::Derrota) {
		g->FillRectangle(velRojo, 0, 0, anchoPantalla, altoPantalla);
		g->DrawString("YOU LOST", fuenteTitulo, Brushes::Red,
			RectangleF(0.0f, (float)altoPantalla * 0.20f, (float)anchoPantalla, 110.0f), centrado);
		g->DrawString("El olvido gano esta vez... pero la memoria resiste.",
			fuenteNormal, Brushes::White,
			RectangleF(0.0f, (float)altoPantalla * 0.38f, (float)anchoPantalla, 40.0f), centrado);
		g->DrawString(String::Format("{0}  -  SCORE: {1}", nombreJugador, score),
			fuenteGrande, Brushes::Gold,
			RectangleF(0.0f, (float)altoPantalla * 0.45f, (float)anchoPantalla, 50.0f), centrado);
		dibujarTablaPuntajes(g, anchoPantalla, (float)altoPantalla * 0.55f);
		g->DrawString("R: reintentar nivel      ENTER: volver al menu",
			fuenteGrande, Brushes::White,
			RectangleF(0.0f, (float)altoPantalla * 0.85f, (float)anchoPantalla, 50.0f), centrado);
	}
}

void Juego::dibujar(Graphics^ g, int anchoPantalla, int altoPantalla) {
	g->Clear(Color::Black);
	if (fondoActual == nullptr || jugador == nullptr) return;

	calcularCamara(anchoPantalla, altoPantalla);
	dibujarMundo(g, anchoPantalla, altoPantalla);
	if (estado != Estado::Victoria && estado != Estado::Derrota) {
		dibujarHUD(g, anchoPantalla, altoPantalla);
		if (estado == Estado::Jugando || estado == Estado::Pausa)
			dibujarMinimapa(g, anchoPantalla, altoPantalla);
	}
	dibujarOverlays(g, anchoPantalla, altoPantalla);
}

bool Juego::getSalirAlMenu() { return salirAlMenu; }
Juego::Estado Juego::getEstado() { return estado; }
