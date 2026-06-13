#pragma once
using namespace System;
using namespace System::Drawing;
using namespace System::Drawing::Drawing2D;

namespace PoliNavis {

	// Fuentes y utilidades de dibujo compartidas por todo el juego
	public ref class Estilos abstract sealed {
	public:
		static Font^ titulo        = gcnew Font(L"Segoe UI", 42.0f, FontStyle::Bold);
		static Font^ enorme        = gcnew Font(L"Segoe UI", 32.0f, FontStyle::Bold);
		static Font^ grande        = gcnew Font(L"Segoe UI", 24.0f, FontStyle::Bold);
		static Font^ subtitulo     = gcnew Font(L"Segoe UI", 17.0f, FontStyle::Bold);
		static Font^ boton         = gcnew Font(L"Segoe UI", 15.0f, FontStyle::Bold);
		static Font^ normalNegrita = gcnew Font(L"Segoe UI", 12.0f, FontStyle::Bold);
		static Font^ normal        = gcnew Font(L"Segoe UI", 11.5f);
		static Font^ pequena       = gcnew Font(L"Segoe UI", 9.5f);
		static Font^ hud           = gcnew Font(L"Consolas", 11.0f, FontStyle::Bold);

		static StringFormat^ centrado    = crearFormato(StringAlignment::Center, StringAlignment::Center);
		static StringFormat^ izquierda   = crearFormato(StringAlignment::Near, StringAlignment::Center);
		static StringFormat^ centroArriba = crearFormato(StringAlignment::Center, StringAlignment::Near);

		static void rellenarRedondeado(Graphics^ g, Color color, float x, float y, float ancho, float alto, float radio);
		static void bordeRedondeado(Graphics^ g, Color color, float grosor, float x, float y, float ancho, float alto, float radio);
		static void textoCentrado(Graphics^ g, String^ texto, Font^ fuente, Color color, float cx, float cy);
		static void textoSombra(Graphics^ g, String^ texto, Font^ fuente, Color color, float cx, float cy);
		static void textoIzquierda(Graphics^ g, String^ texto, Font^ fuente, Color color, float x, float y);
		static Color oscurecer(Color color, float factor);   // factor 0..1 (0 = negro)
		static Color aclarar(Color color, float factor);     // factor 0..1 (1 = blanco)

	private:
		static StringFormat^ crearFormato(StringAlignment horizontal, StringAlignment vertical);
		static void agregarRectRedondeado(GraphicsPath^ ruta, float x, float y, float ancho, float alto, float radio);
	};
}
