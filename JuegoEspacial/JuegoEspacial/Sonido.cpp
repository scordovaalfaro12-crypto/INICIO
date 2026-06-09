#include "Sonido.h"

using namespace System::Threading;

namespace JuegoEspacial {

	// Encola la secuencia de tonos {frecuencia, duracion, frecuencia, duracion, ...}
	void Sonido::lanzar(array<int>^ datos) {
		if (!activado) return;
		ThreadPool::QueueUserWorkItem(gcnew WaitCallback(&Sonido::trabajador), datos);
	}

	void Sonido::trabajador(Object^ estado) {
		if (sonando) return;          // evita que los pitidos se amontonen
		sonando = true;
		array<int>^ datos = (array<int>^)estado;
		for (int i = 0; i + 1 < datos->Length; i += 2) {
			Console::Beep(datos[i], datos[i + 1]);
		}
		sonando = false;
	}

	void Sonido::tono(int frecuencia, int duracion) {
		lanzar(gcnew array<int> { frecuencia, duracion });
	}

	void Sonido::mover() { tono(700, 35); }

	void Sonido::seleccionar() {
		lanzar(gcnew array<int> { 880, 55, 1175, 75 });
	}

	void Sonido::error() {
		lanzar(gcnew array<int> { 220, 130 });
	}

	void Sonido::disparo() { tono(1000, 28); }

	void Sonido::explosion() {
		lanzar(gcnew array<int> { 180, 45, 130, 55 });
	}

	void Sonido::item() {
		lanzar(gcnew array<int> { 1320, 45, 1760, 55 });
	}

	void Sonido::danio() {
		lanzar(gcnew array<int> { 200, 90 });
	}

	void Sonido::victoria() {
		lanzar(gcnew array<int> { 659, 95, 784, 95, 880, 95, 1047, 170 });
	}

	void Sonido::derrota() {
		lanzar(gcnew array<int> { 392, 130, 330, 130, 262, 200 });
	}

	void Sonido::notaSimon(int indice) {
		int frecuencia = 523;
		if (indice == 1) frecuencia = 659;
		else if (indice == 2) frecuencia = 784;
		else if (indice == 3) frecuencia = 1047;
		tono(frecuencia, 220);
	}

	void Sonido::anillo() { tono(940, 40); }

	void Sonido::despegueNave() {
		lanzar(gcnew array<int> { 150, 120, 220, 120, 320, 150 });
	}

	void Sonido::aterrizajeNave() {
		lanzar(gcnew array<int> { 320, 120, 220, 120, 150, 150 });
	}
}
