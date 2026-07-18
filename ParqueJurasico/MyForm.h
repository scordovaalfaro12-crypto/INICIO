#pragma once
#include "Controlador.h"

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
			// Se convierte a 32 bpp y se vuelve transparente el color de fondo
			Bitmap^ original = gcnew Bitmap(ruta);
			Bitmap^ bmp = gcnew Bitmap(original->Width, original->Height,
				System::Drawing::Imaging::PixelFormat::Format32bppArgb);
			Graphics^ g = Graphics::FromImage(bmp);
			g->DrawImage(original, 0, 0, original->Width, original->Height);
			delete g;
			delete original;
			Color fondo = bmp->GetPixel(0, 0);
			if (fondo.A == 255) bmp->MakeTransparent(fondo);
			return bmp;
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
