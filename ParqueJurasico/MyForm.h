#pragma once
#include "Controlador.h"
#include <vector>

namespace ParqueJurasico {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	// Formulario principal: crea el buffer de dibujo, carga los sprites y
	// delega el ciclo de animacion, las teclas y el dibujo en el Controlador.
	public ref class MyForm : public System::Windows::Forms::Form
	{
	public:
		MyForm(void)
		{
			InitializeComponent();
			cargarSprites();
			controlador = new Controlador();
			finMostrado = false;

			// Buffer para evitar el parpadeo de la animacion
			BufferedGraphicsContext^ contexto = BufferedGraphicsManager::Current;
			contexto->MaximumBuffer = System::Drawing::Size(
				Controlador::ANCHO_VENTANA + 1, Controlador::ALTO_VENTANA + 1);
			buffer = contexto->Allocate(this->CreateGraphics(), this->ClientRectangle);

			timerJuego->Start();
		}

	protected:
		~MyForm()
		{
			if (controlador != nullptr)
			{
				delete controlador;   // libera todos los objetos del juego
				controlador = nullptr;
			}
			if (buffer != nullptr)
			{
				delete buffer;
				buffer = nullptr;
			}
			if (components)
			{
				delete components;
			}
		}

	private:
		System::ComponentModel::Container^ components;
		System::Windows::Forms::Timer^ timerJuego;

		Controlador* controlador;
		BufferedGraphics^ buffer;
		bool finMostrado;

		Bitmap^ bmpGuarda;
		Bitmap^ bmpRaptor;     // _dinosaurio2.png (velocirraptor)
		Bitmap^ bmpPtera;      // _dinosaurio.png (pteranodonte)
		Bitmap^ bmpAlfa;       // DINOALFA.png
		Bitmap^ bmpDardo;
		Bitmap^ bmpHeli;

		void InitializeComponent(void)
		{
			this->components = gcnew System::ComponentModel::Container();
			this->timerJuego = gcnew System::Windows::Forms::Timer(this->components);
			this->SuspendLayout();
			//
			// timerJuego
			//
			this->timerJuego->Interval = Controlador::INTERVALO_MS;
			this->timerJuego->Tick += gcnew System::EventHandler(this, &MyForm::timerJuego_Tick);
			//
			// MyForm
			//
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::None;
			this->ClientSize = System::Drawing::Size(Controlador::ANCHO_VENTANA, Controlador::ALTO_VENTANA);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
			this->MaximizeBox = false;
			this->KeyPreview = true;
			this->Name = L"MyForm";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			this->Text = L"Parque Jurasico: Operacion Contencion 2042";
			this->BackColor = System::Drawing::Color::Black;
			this->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &MyForm::MyForm_KeyDown);
			this->KeyUp += gcnew System::Windows::Forms::KeyEventHandler(this, &MyForm::MyForm_KeyUp);
			this->ResumeLayout(false);
		}

		// Busca el sprite en varias rutas relativas (proyecto o carpeta del .exe)
		Bitmap^ cargarBitmap(String^ nombre)
		{
			array<String^>^ carpetas = gcnew array<String^> {
				L"sprites\\", L"..\\sprites\\", L"..\\..\\sprites\\", L"..\\..\\..\\sprites\\"
			};
			String^ ruta = nullptr;
			for each (String^ carpeta in carpetas)
			{
				if (System::IO::File::Exists(carpeta + nombre))
				{
					ruta = carpeta + nombre;
					break;
				}
			}
			if (ruta == nullptr)
			{
				MessageBox::Show("No se encontro el sprite: " + nombre +
					"\nCopie la carpeta 'sprites' junto al ejecutable o al proyecto.",
					"Error de recursos", MessageBoxButtons::OK, MessageBoxIcon::Error);
				Environment::Exit(1);
			}
			// Se convierte a 32 bpp y se elimina el fondo casi blanco de la hoja
			Bitmap^ original = gcnew Bitmap(ruta);
			Bitmap^ bmp = gcnew Bitmap(original->Width, original->Height,
				System::Drawing::Imaging::PixelFormat::Format32bppArgb);
			Graphics^ g = Graphics::FromImage(bmp);
			g->DrawImage(original, 0, 0, original->Width, original->Height);
			delete g;
			delete original;
			quitarFondoBlanco(bmp);
			return bmp;
		}

		// Vuelve transparente el fondo de la hoja de sprites. Como el fondo no
		// es un blanco uniforme (tiene ruido), se hace un relleno por difusion
		// desde los bordes: solo se borran los pixeles casi blancos CONECTADOS
		// con el exterior, conservando los blancos internos del dibujo (ojos, etc.).
		void quitarFondoBlanco(Bitmap^ bmp)
		{
			Color esquina = bmp->GetPixel(0, 0);
			// Si la hoja ya trae transparencia real o no es fondo claro, no se toca
			if (esquina.A != 255 || esquina.R < 200 || esquina.G < 200 || esquina.B < 200)
				return;

			const int UMBRAL = 225;   // se considera fondo todo canal >= 225
			int w = bmp->Width, h = bmp->Height;
			System::Drawing::Imaging::BitmapData^ datos = bmp->LockBits(
				System::Drawing::Rectangle(0, 0, w, h),
				System::Drawing::Imaging::ImageLockMode::ReadWrite,
				System::Drawing::Imaging::PixelFormat::Format32bppArgb);
			unsigned char* base = (unsigned char*)datos->Scan0.ToPointer();
			int stride = datos->Stride;

			std::vector<unsigned char> visitado(w * h, 0);
			std::vector<int> pila;
			// Semillas: todos los pixeles del borde de la imagen
			for (int x = 0; x < w; x++) { pila.push_back(x); pila.push_back((h - 1) * w + x); }
			for (int y = 0; y < h; y++) { pila.push_back(y * w); pila.push_back(y * w + w - 1); }

			while (!pila.empty())
			{
				int idx = pila.back();
				pila.pop_back();
				if (visitado[idx]) continue;
				visitado[idx] = 1;
				int x = idx % w, y = idx / w;
				unsigned char* p = base + y * stride + x * 4;   // orden B, G, R, A
				if (p[0] < UMBRAL || p[1] < UMBRAL || p[2] < UMBRAL) continue;
				p[3] = 0;   // pixel de fondo: se vuelve transparente
				if (x > 0)     pila.push_back(idx - 1);
				if (x < w - 1) pila.push_back(idx + 1);
				if (y > 0)     pila.push_back(idx - w);
				if (y < h - 1) pila.push_back(idx + w);
			}
			bmp->UnlockBits(datos);
		}

		void cargarSprites()
		{
			bmpGuarda = cargarBitmap(L"_guardabosque.png");
			bmpRaptor = cargarBitmap(L"_dinosaurio2.png");
			bmpPtera = cargarBitmap(L"_dinosaurio.png");
			bmpAlfa = cargarBitmap(L"DINOALFA.png");
			bmpDardo = cargarBitmap(L"_dardo.png");
			bmpHeli = cargarBitmap(L"_helicoptero.png");
		}

		void timerJuego_Tick(System::Object^ sender, System::EventArgs^ e)
		{
			controlador->actualizar();
			controlador->dibujar(buffer, bmpGuarda, bmpRaptor, bmpPtera, bmpAlfa, bmpDardo, bmpHeli);

			if (controlador->getEstado() != Controlador::JUGANDO && !finMostrado)
			{
				finMostrado = true;
				timerJuego->Stop();
				String^ titulo = (controlador->getEstado() == Controlador::GANADO)
					? L"PARQUE ASEGURADO"
					: L"LOS DINOSAURIOS ESCAPARON";
				MessageBox::Show(this,
					titulo + L"\n\n===== RESULTADOS DE LA PARTIDA =====\n" +
					controlador->getResumenPartida() +
					L"\n\n(El reporte tambien se guardo en RESULTADOS.TXT)",
					L"Fin del juego", MessageBoxButtons::OK, MessageBoxIcon::Information);
			}
		}

		void MyForm_KeyDown(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e)
		{
			controlador->teclaPresionada((int)e->KeyCode);
		}

		void MyForm_KeyUp(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e)
		{
			controlador->teclaLiberada((int)e->KeyCode);
		}
	};
}
