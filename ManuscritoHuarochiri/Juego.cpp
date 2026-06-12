#include "Juego.h"

Juego::Juego() {
	srand((unsigned)time(0));

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
	centrado = gcnew StringFormat();
	centrado->Alignment = StringAlignment::Center;
	centrado->LineAlignment = StringAlignment::Center;

	// Entidades
	jugador = nullptr;
	enemigos = new vector<LlamaDigital*>();
	nodos = new vector<NodoDato*>();
	pilares = new vector<PilarInformacion*>();
	rayos = new vector<RayoTrace*>();
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
	teclaArriba = teclaAbajo = teclaIzquierda = teclaDerecha = teclaInteractuar = false;

	fondoActual = (numero == 2) ? imgEscenario2 : imgEscenario1;
	int W = fondoActual->Width;
	int H = fondoActual->Height;

	if (jugador != nullptr) delete jugador;
	jugador = new DeidadLluvia(imgDeidadLluvia);
	jugador->setConPoder(poderDesbloqueado);

	if (numero == 1) {
		// Recuperar los 7 nodos de datos esquivando las llamas digitales
		tiempoRestante = 180 * 33;
		spawnX = (int)(0.50 * W) - jugador->getAncho() / 2;
		spawnY = (int)(0.82 * H) - jugador->getAlto() / 2;

		nodos->push_back(new NodoDato((int)(0.08 * W), (int)(0.12 * H), 0));
		nodos->push_back(new NodoDato((int)(0.34 * W), (int)(0.07 * H), 1));
		nodos->push_back(new NodoDato((int)(0.63 * W), (int)(0.10 * H), 2));
		nodos->push_back(new NodoDato((int)(0.90 * W), (int)(0.16 * H), 3));
		nodos->push_back(new NodoDato((int)(0.09 * W), (int)(0.56 * H), 4));
		nodos->push_back(new NodoDato((int)(0.50 * W), (int)(0.47 * H), 5));
		nodos->push_back(new NodoDato((int)(0.88 * W), (int)(0.62 * H), 6));

		enemigos->push_back(new LlamaDigital((int)(0.74 * W), (int)(0.18 * H), 3, 340));
		enemigos->push_back(new LlamaDigital((int)(0.86 * W), (int)(0.48 * H), 3, 340));
		enemigos->push_back(new LlamaDigital((int)(0.55 * W), (int)(0.30 * H), 3, 330));
		enemigos->push_back(new LlamaDigital((int)(0.63 * W), (int)(0.74 * H), 3, 330));
		enemigos->push_back(new LlamaDigital((int)(0.25 * W), (int)(0.32 * H), 2, 300));
	}
	else if (numero == 2) {
		// Escoltar la ofrenda digital hasta el Santuario de Macahuisa
		tiempoRestante = 200 * 33;
		spawnX = (int)(0.10 * W) - jugador->getAncho() / 2;
		spawnY = (int)(0.86 * H) - jugador->getAlto() / 2;
		jugador->setConOfrenda(true);

		santuario = new SantuarioMacahuisa((int)(0.50 * W), (int)(0.45 * H));
		aliado = new AliadoAlgoritmo();
		aliado->setX(spawnX - 80);
		aliado->setY(spawnY - 60);

		enemigos->push_back(new LlamaDigital((int)(0.50 * W), (int)(0.30 * H), 4, 400));
		enemigos->push_back(new LlamaDigital((int)(0.62 * W), (int)(0.47 * H), 4, 400));
		enemigos->push_back(new LlamaDigital((int)(0.38 * W), (int)(0.47 * H), 4, 400));
		enemigos->push_back(new LlamaDigital((int)(0.50 * W), (int)(0.64 * H), 4, 400));
		enemigos->push_back(new LlamaDigital((int)(0.27 * W), (int)(0.70 * H), 3, 380));
		enemigos->push_back(new LlamaDigital((int)(0.72 * W), (int)(0.28 * H), 3, 380));
		enemigos->push_back(new LlamaDigital((int)(0.30 * W), (int)(0.20 * H), 3, 380));
	}
	else {
		// Activar los 3 pilares de informacion protegidos por Huallallo
		tiempoRestante = 260 * 33;
		spawnX = (int)(0.50 * W) - jugador->getAncho() / 2;
		spawnY = (int)(0.82 * H) - jugador->getAlto() / 2;

		pilares->push_back(new PilarInformacion((int)(0.13 * W), (int)(0.14 * H)));
		pilares->push_back(new PilarInformacion((int)(0.86 * W), (int)(0.18 * H)));
		pilares->push_back(new PilarInformacion((int)(0.15 * W), (int)(0.78 * H)));

		jefe = new HuallalloJefe((int)(0.52 * W), (int)(0.42 * H));

		enemigos->push_back(new LlamaDigital((int)(0.30 * W), (int)(0.40 * H), 3, 420));
		enemigos->push_back(new LlamaDigital((int)(0.70 * W), (int)(0.55 * H), 3, 420));
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

	if (jugador->getVida() > 0) {
		if (nivelActual == 2) {
			// En la escolta, un golpe te regresa al inicio del camino
			jugador->setX(spawnX);
			jugador->setY(spawnY);
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

	// ----- Movimiento del jugador -----
	int velocidad = jugador->getConOfrenda() ? 6 : 8;
	int vx = 0, vy = 0;
	if (teclaIzquierda) vx -= velocidad;
	if (teclaDerecha) vx += velocidad;
	if (teclaArriba) vy -= velocidad;
	if (teclaAbajo) vy += velocidad;
	if (vx != 0 && vy != 0) { vx = vx * 3 / 4; vy = vy * 3 / 4; } // diagonal pareja
	jugador->setDX(vx);
	jugador->setDY(vy);
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

	// ----- Rayos Ray-Trace -----
	for (int i = (int)rayos->size() - 1; i >= 0; i--) {
		RayoTrace* rayo = rayos->at(i);
		rayo->mover(nullptr);

		for (int j = 0; j < (int)enemigos->size(); j++) {
			LlamaDigital* enemigo = enemigos->at(j);
			if (rayo->getActivo() && enemigo->getActivo() &&
				rayo->hitbox().IntersectsWith(enemigo->hitbox())) {
				enemigo->setActivo(false);
				rayo->setActivo(false);
				score += 50;
				reproducirEfecto("msc/sonidoGolpe.wav");
			}
		}
		if (nivelActual == 3 && jefe != nullptr && jefe->getActivo() && rayo->getActivo() &&
			rayo->hitbox().IntersectsWith(jefe->hitbox())) {
			jefe->recibirRayo();
			rayo->setActivo(false);
			reproducirEfecto("msc/sonidoGolpe.wav");
			if (!jefe->getActivo()) {
				score += 500;
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
			score += 100;
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
		score += 500;
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
				score += 200;
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

	if (completos >= 3) completarNivel();
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

	if (santuario != nullptr) santuario->mostrar(g, nullptr, cx, cy);
	for (int i = 0; i < (int)pilares->size(); i++) pilares->at(i)->mostrar(g, nullptr, cx, cy);
	for (int i = 0; i < (int)nodos->size(); i++)
		if (nodos->at(i)->getActivo()) nodos->at(i)->mostrar(g, nullptr, cx, cy);
	for (int i = 0; i < (int)enemigos->size(); i++)
		if (enemigos->at(i)->getActivo()) enemigos->at(i)->mostrar(g, nullptr, cx, cy);
	if (jefe != nullptr) jefe->mostrar(g, nullptr, cx, cy);
	for (int i = 0; i < (int)rayos->size(); i++) rayos->at(i)->mostrar(g, nullptr, cx, cy);
	if (aliado != nullptr) aliado->mostrar(g, nullptr, cx, cy);
	jugador->mostrar(g, imgDeidadLluvia, imgDeidadEspejo, cx, cy);

	if (nivelActual == 2 && jugador->getConOfrenda()) dibujarFlechaGuia(g);
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

void Juego::dibujarCorazon(Graphics^ g, int px, int py, bool lleno) {
	Brush^ color = lleno ? Brushes::Red : Brushes::DimGray;
	g->FillEllipse(color, px, py, 16, 16);
	g->FillEllipse(color, px + 13, py, 16, 16);
	array<Point>^ pico = { Point(px + 1, py + 11), Point(px + 28, py + 11), Point(px + 14, py + 28) };
	g->FillPolygon(color, pico);
}

void Juego::dibujarHUD(Graphics^ g, int anchoPantalla, int altoPantalla) {
	// Barra superior
	g->FillRectangle(velOscuro, 0, 0, anchoPantalla, 66);

	// Vidas
	for (int i = 0; i < 3; i++)
		dibujarCorazon(g, 22 + i * 40, 18, i < jugador->getVida());

	// Contador de borrado (centro)
	int segundos = tiempoRestante / 33;
	int barraW = 320;
	int barraX = anchoPantalla / 2 - barraW / 2;
	double proporcion = 0;
	int tiempoTotal = (nivelActual == 1) ? 180 : (nivelActual == 2 ? 200 : 260);
	proporcion = (double)segundos / tiempoTotal;
	if (proporcion > 1) proporcion = 1;
	g->DrawString("BORRADO DEL SERVIDOR EN", fuenteChica, Brushes::OrangeRed,
		(float)barraX + barraW / 2 - 110, 6.0f);
	g->FillRectangle(Brushes::Black, barraX, 26, barraW, 18);
	Brush^ colorTiempo = (segundos < 30) ? Brushes::Red : Brushes::Cyan;
	g->FillRectangle(colorTiempo, barraX + 2, 28, (int)((barraW - 4) * proporcion), 14);
	g->DrawRectangle(Pens::White, barraX, 26, barraW, 18);
	g->DrawString(String::Format("{0}:{1:D2}", segundos / 60, segundos % 60),
		fuenteNormal, Brushes::White, (float)(barraX + barraW + 12), 22.0f);

	// Score y nivel (derecha)
	g->DrawString(String::Format("SCORE {0:D5}", score), fuenteGrande, Brushes::Gold,
		(float)(anchoPantalla - 330), 10.0f);
	g->DrawString(String::Format("NIVEL {0}/3", nivelActual), fuenteNormal, Brushes::White,
		(float)(anchoPantalla - 120), 40.0f);

	// Barra inferior con el objetivo
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
		objetivo = String::Format("PILARES COMPILADOS: {0}/3  -  Manten E junto a un pilar; no dejes que Huallallo los drene", completos);
	}
	g->DrawString(objetivo, fuenteNormal, Brushes::White, 20.0f, (float)(altoPantalla - 42));
	g->DrawString("P: pausa   ESC: menu", fuenteChica, Brushes::Gray,
		(float)(anchoPantalla - 230), (float)(altoPantalla - 36));

	// Tarjeta del fragmento quechua recuperado
	if (fragmentoTicks > 0 && estado == Estado::Jugando) {
		int cw = (int)(anchoPantalla * 0.62);
		int ch = 120;
		int cx = anchoPantalla / 2 - cw / 2;
		int cy = altoPantalla - 200;
		g->FillRectangle(velOscuro, cx, cy, cw, ch);
		g->DrawRectangle(Pens::Cyan, cx, cy, cw, ch);
		g->DrawString("NODO RECUPERADO - TRADUCCION AUTOMATICA", fuenteChica, Brushes::Cyan,
			RectangleF((float)cx, (float)cy + 8, (float)cw, 20.0f), centrado);
		g->DrawString(fragmentoQuechua, fuenteGrande, Brushes::Cyan,
			RectangleF((float)cx, (float)cy + 30, (float)cw, 44.0f), centrado);
		g->DrawString(fragmentoTraduccion, fuenteNormal, Brushes::White,
			RectangleF((float)cx, (float)cy + 76, (float)cw, 34.0f), centrado);
	}

	// Mensaje flotante
	if (mensajeTicks > 0) {
		RectangleF zona = RectangleF(0.0f, (float)(altoPantalla / 5), (float)anchoPantalla, 60.0f);
		RectangleF sombra = RectangleF(3.0f, (float)(altoPantalla / 5) + 3, (float)anchoPantalla, 60.0f);
		g->DrawString(mensaje, fuenteGrande, Brushes::Black, sombra, centrado);
		g->DrawString(mensaje, fuenteGrande, Brushes::Gold, zona, centrado);
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
				"Huallallo drenara el pilar mas avanzado: decide cual proteger.",
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
		g->DrawString(String::Format("SCORE FINAL: {0}", score), fuenteGrande, Brushes::Gold,
			RectangleF(0.0f, (float)altoPantalla * 0.65f, (float)anchoPantalla, 50.0f), centrado);
		g->DrawString("Quien decide que conocimiento merece ser recordado?",
			fuenteChica, Brushes::Gray,
			RectangleF(0.0f, (float)altoPantalla * 0.74f, (float)anchoPantalla, 30.0f), centrado);
		g->DrawString("ENTER: volver al menu", fuenteNormal, Brushes::Gold,
			RectangleF(0.0f, (float)altoPantalla * 0.84f, (float)anchoPantalla, 40.0f), centrado);
	}
	else if (estado == Estado::Derrota) {
		g->FillRectangle(velRojo, 0, 0, anchoPantalla, altoPantalla);
		g->DrawString("GAME OVER", fuenteTitulo, Brushes::Red,
			RectangleF(0.0f, (float)altoPantalla * 0.28f, (float)anchoPantalla, 110.0f), centrado);
		g->DrawString("El olvido gano esta vez... pero la memoria resiste.",
			fuenteNormal, Brushes::White,
			RectangleF(0.0f, (float)altoPantalla * 0.46f, (float)anchoPantalla, 40.0f), centrado);
		g->DrawString(String::Format("SCORE: {0}", score), fuenteGrande, Brushes::Gold,
			RectangleF(0.0f, (float)altoPantalla * 0.54f, (float)anchoPantalla, 50.0f), centrado);
		g->DrawString("R: reintentar nivel      ENTER: volver al menu",
			fuenteGrande, Brushes::White,
			RectangleF(0.0f, (float)altoPantalla * 0.66f, (float)anchoPantalla, 50.0f), centrado);
	}
}

void Juego::dibujar(Graphics^ g, int anchoPantalla, int altoPantalla) {
	g->Clear(Color::Black);
	if (fondoActual == nullptr || jugador == nullptr) return;

	calcularCamara(anchoPantalla, altoPantalla);
	dibujarMundo(g, anchoPantalla, altoPantalla);
	if (estado != Estado::Victoria && estado != Estado::Derrota)
		dibujarHUD(g, anchoPantalla, altoPantalla);
	dibujarOverlays(g, anchoPantalla, altoPantalla);
}

bool Juego::getSalirAlMenu() { return salirAlMenu; }
Juego::Estado Juego::getEstado() { return estado; }
