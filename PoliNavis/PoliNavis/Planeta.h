#pragma once
#include "Entidad.h"

namespace PoliNavis {

	// ===== CLASE ABSTRACTA Planeta (hereda de Entidad) =====
	// Modela un planeta con sus DOS movimientos obligatorios:
	//   - TRASLACION: orbita alrededor del Sol (cambia x, y).
	//   - ROTACION: gira sobre su propio eje (cambia 'rotacion').
	// Cada planeta concreto solo implementa su aspecto en dibujarSuperficie().
	// Demuestra HERENCIA y POLIMORFISMO: la lista de Planeta^ se dibuja igual
	// sin saber el tipo concreto.
	public ref class Planeta abstract : public Entidad {
	protected:
		float solX, solY;        // centro de la orbita (el Sol)
		float radioOrbita;       // distancia al Sol (traslacion)
		float anguloOrbita;      // posicion actual en la orbita (grados)
		float velOrbita;         // velocidad de traslacion
		float rotacion;          // angulo de giro propio (rotacion)
		float velRotacion;       // velocidad de rotacion
		float radio;             // tamano del planeta
		float impulso;           // multiplicador temporal de velocidad (evento)
		String^ nombre;
		String^ dato;            // dato educativo para la leyenda

	public:
		Planeta(float psolX, float psolY, float pradioOrbita, float panguloIni,
			float pvelOrbita, float pvelRotacion, float pradio, String^ pnombre, String^ pdato);

		virtual void mover() override;
		virtual void dibujar(Graphics^ g) override;
		virtual RectangleF area() override;

		// cada planeta dibuja su superficie (relieve, bandas, etc.) usando
		// 'rot' para que el detalle gire de forma visible.
		virtual void dibujarSuperficie(Graphics^ g, float r, float rot) = 0;
		// extra que se dibuja FUERA del recorte circular (anillos). Por defecto nada.
		virtual void dibujarExtra(Graphics^ g, float r) {}

		void acelerar();         // evento: gira/traslada mas rapido un instante
		void dibujarOrbita(Graphics^ g);

		// dibuja una mancha de superficie en cierta "longitud" del planeta;
		// al variar 'rot' la mancha se desplaza por el disco => rotacion visible.
		static void marca(Graphics^ g, float cx, float cy, float r,
			float longitud, float rot, float lat, float tam, Color color);

		String^ getNombre();
		String^ getDato();
		float getRadio();
		Color colorLeyenda;      // color para el punto de la leyenda
	};
}
