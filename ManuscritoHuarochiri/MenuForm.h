#pragma once
#include "MenuInstruccionesForm.h"
#include "CreditosForm.h"
#include "JuegoForm.h"


namespace ManuscritoHuarochiri {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace WMPLib; // Musica

	public ref class MenuForm : public Form {
		//Musica
		WMPLib::WindowsMediaPlayerClass^ reproductorMusica;
		WMPLib::WindowsMediaPlayerClass^ reproductorEfectos;

		//Declarar ventanas (Forms)
		MenuInstruccionesForm^ ventanaInstrucciones;
		CreditosForm^ ventanaCreditos;

		Bitmap^ imgMenu1920x1200;

		Bitmap^ imgJugarEnter;
		Bitmap^ imgJugarLeave;
		Bitmap^ imgInstruccionesEnter;
		Bitmap^ imgInstruccionesLeave;
		Bitmap^ imgCreditosEnter;
		Bitmap^ imgCreditosLeave;
		Bitmap^ imgSalirEnter;
		Bitmap^ imgSalirLeave;


	public:
		MenuForm(void) {
			InitializeComponent();
			imgMenu1920x1200 = gcnew Bitmap("imgMenu/menu1920x1200.png");
			this->BackgroundImage = imgMenu1920x1200;
			this->BackgroundImageLayout = ImageLayout::Stretch;

			imgSalirEnter = gcnew Bitmap("imgMenu/salirEnter.png");
			imgSalirLeave = gcnew Bitmap("imgMenu/salirLeave.png");

			imgJugarEnter = gcnew Bitmap("imgMenu/jugarEnter.png");
			imgJugarLeave = gcnew Bitmap("imgMenu/jugarLeave.png");

			imgInstruccionesEnter = gcnew Bitmap("imgMenu/instruccionesEnter.png");
			imgInstruccionesLeave = gcnew Bitmap("imgMenu/instruccionesLeave.png");

			imgCreditosEnter = gcnew Bitmap("imgMenu/creditosEnter.png");
			imgCreditosLeave = gcnew Bitmap("imgMenu/creditosLeave.png");


			reproductorMusica = gcnew WMPLib::WindowsMediaPlayerClass();
			reproductorMusica->settings->volume = 60;
			reproductorMusica->URL = "msc/sonidoMenu.wav";
			reproductorMusica->settings->setMode("loop", true); // Bucle infinito
			reproductorMusica->controls->play();

			ventanaInstrucciones = gcnew MenuInstruccionesForm();
			ventanaCreditos = gcnew CreditosForm();
			reproductorEfectos = gcnew WMPLib::WindowsMediaPlayerClass();

			posicionarBotones();
		}

	protected:

		~MenuForm() {
			if (components) {
				delete components;
				delete reproductorMusica;
				delete reproductorEfectos;
				delete imgMenu1920x1200;
				delete imgJugarEnter;
				delete imgJugarLeave;
				delete imgInstruccionesEnter;
				delete imgInstruccionesLeave;
				delete imgCreditosEnter;
				delete imgCreditosLeave;
				delete imgSalirEnter;
				delete imgSalirLeave;
				delete ventanaCreditos;
				delete ventanaInstrucciones;
			}
		}

	private: System::Windows::Forms::Button^ btnJugar;
	private: System::Windows::Forms::Button^ btnInstruccion;
	private: System::Windows::Forms::Button^ btnCreditos;
	private: System::Windows::Forms::Button^ btnSalir;
	protected:
	private: System::ComponentModel::IContainer^ components;
	private:



#pragma region Windows Form Designer generated code

		void InitializeComponent(void) {
			this->components = (gcnew System::ComponentModel::Container());
			System::ComponentModel::ComponentResourceManager^ resources = (gcnew System::ComponentModel::ComponentResourceManager(MenuForm::typeid));
			this->btnJugar = (gcnew System::Windows::Forms::Button());
			this->btnInstruccion = (gcnew System::Windows::Forms::Button());
			this->btnCreditos = (gcnew System::Windows::Forms::Button());
			this->btnSalir = (gcnew System::Windows::Forms::Button());
			this->SuspendLayout();
			//
			// btnJugar
			//
			this->btnJugar->BackColor = System::Drawing::Color::Transparent;
			this->btnJugar->BackgroundImage = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"btnJugar.BackgroundImage")));
			this->btnJugar->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
			this->btnJugar->FlatAppearance->BorderSize = 0;
			this->btnJugar->FlatAppearance->MouseDownBackColor = System::Drawing::Color::Transparent;
			this->btnJugar->FlatAppearance->MouseOverBackColor = System::Drawing::Color::Transparent;
			this->btnJugar->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->btnJugar->ForeColor = System::Drawing::Color::Black;
			this->btnJugar->Location = System::Drawing::Point(800, 636);
			this->btnJugar->Name = L"btnJugar";
			this->btnJugar->Size = System::Drawing::Size(400, 100);
			this->btnJugar->TabIndex = 0;
			this->btnJugar->UseVisualStyleBackColor = false;
			this->btnJugar->Click += gcnew System::EventHandler(this, &MenuForm::btnJugar_Click);
			this->btnJugar->MouseEnter += gcnew System::EventHandler(this, &MenuForm::btnJugar_MouseEnter);
			this->btnJugar->MouseLeave += gcnew System::EventHandler(this, &MenuForm::btnJugar_MouseLeave);
			//
			// btnInstruccion
			//
			this->btnInstruccion->BackColor = System::Drawing::Color::Transparent;
			this->btnInstruccion->BackgroundImage = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"btnInstruccion.BackgroundImage")));
			this->btnInstruccion->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
			this->btnInstruccion->FlatAppearance->BorderSize = 0;
			this->btnInstruccion->FlatAppearance->MouseDownBackColor = System::Drawing::Color::Transparent;
			this->btnInstruccion->FlatAppearance->MouseOverBackColor = System::Drawing::Color::Transparent;
			this->btnInstruccion->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->btnInstruccion->Location = System::Drawing::Point(800, 742);
			this->btnInstruccion->Name = L"btnInstruccion";
			this->btnInstruccion->Size = System::Drawing::Size(400, 100);
			this->btnInstruccion->TabIndex = 1;
			this->btnInstruccion->UseVisualStyleBackColor = false;
			this->btnInstruccion->Click += gcnew System::EventHandler(this, &MenuForm::btnInstruccion_Click);
			this->btnInstruccion->MouseEnter += gcnew System::EventHandler(this, &MenuForm::btnInstruccion_MouseEnter);
			this->btnInstruccion->MouseLeave += gcnew System::EventHandler(this, &MenuForm::btnInstruccion_MouseLeave);
			//
			// btnCreditos
			//
			this->btnCreditos->BackColor = System::Drawing::Color::Transparent;
			this->btnCreditos->BackgroundImage = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"btnCreditos.BackgroundImage")));
			this->btnCreditos->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
			this->btnCreditos->FlatAppearance->BorderSize = 0;
			this->btnCreditos->FlatAppearance->MouseDownBackColor = System::Drawing::Color::Transparent;
			this->btnCreditos->FlatAppearance->MouseOverBackColor = System::Drawing::Color::Transparent;
			this->btnCreditos->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->btnCreditos->Location = System::Drawing::Point(800, 848);
			this->btnCreditos->Name = L"btnCreditos";
			this->btnCreditos->Size = System::Drawing::Size(400, 100);
			this->btnCreditos->TabIndex = 3;
			this->btnCreditos->UseVisualStyleBackColor = false;
			this->btnCreditos->Click += gcnew System::EventHandler(this, &MenuForm::btnCreditos_Click);
			this->btnCreditos->MouseEnter += gcnew System::EventHandler(this, &MenuForm::btnCreditos_MouseEnter);
			this->btnCreditos->MouseLeave += gcnew System::EventHandler(this, &MenuForm::btnCreditos_MouseLeave);
			//
			// btnSalir
			//
			this->btnSalir->BackColor = System::Drawing::Color::Transparent;
			this->btnSalir->BackgroundImage = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"btnSalir.BackgroundImage")));
			this->btnSalir->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
			this->btnSalir->FlatAppearance->BorderSize = 0;
			this->btnSalir->FlatAppearance->MouseDownBackColor = System::Drawing::Color::Transparent;
			this->btnSalir->FlatAppearance->MouseOverBackColor = System::Drawing::Color::Transparent;
			this->btnSalir->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->btnSalir->Location = System::Drawing::Point(800, 954);
			this->btnSalir->Name = L"btnSalir";
			this->btnSalir->Size = System::Drawing::Size(400, 100);
			this->btnSalir->TabIndex = 4;
			this->btnSalir->UseVisualStyleBackColor = false;
			this->btnSalir->Click += gcnew System::EventHandler(this, &MenuForm::btnSalir_Click);
			this->btnSalir->MouseEnter += gcnew System::EventHandler(this, &MenuForm::btnSalir_MouseEnter);
			this->btnSalir->MouseLeave += gcnew System::EventHandler(this, &MenuForm::btnSalir_MouseLeave);
			//
			// MenuForm
			//
			this->AutoScaleDimensions = System::Drawing::SizeF(8, 16);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->BackColor = System::Drawing::SystemColors::ActiveCaptionText;
			this->BackgroundImage = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"$this.BackgroundImage")));
			this->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
			this->ClientSize = System::Drawing::Size(1920, 1200);
			this->Controls->Add(this->btnSalir);
			this->Controls->Add(this->btnCreditos);
			this->Controls->Add(this->btnInstruccion);
			this->Controls->Add(this->btnJugar);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::None;
			this->Icon = (cli::safe_cast<System::Drawing::Icon^>(resources->GetObject(L"$this.Icon")));
			this->Name = L"MenuForm";
			this->Text = L"MenuForm";
			this->WindowState = System::Windows::Forms::FormWindowState::Maximized;
			this->Resize += gcnew System::EventHandler(this, &MenuForm::MenuForm_Resize);
			this->ResumeLayout(false);

		}
#pragma endregion

		// Coloca los botones de forma proporcional a la pantalla real
		// (el diseno original era fijo para 1920x1200).
	private: void posicionarBotones() {
		int anchoBtn = 400, altoBtn = 100;
		int centroX = this->ClientSize.Width / 2 - anchoBtn / 2;
		int alto = this->ClientSize.Height;
		this->btnJugar->Location = System::Drawing::Point(centroX, (int)(alto * 0.53));
		this->btnInstruccion->Location = System::Drawing::Point(centroX, (int)(alto * 0.53) + 106);
		this->btnCreditos->Location = System::Drawing::Point(centroX, (int)(alto * 0.53) + 212);
		this->btnSalir->Location = System::Drawing::Point(centroX, (int)(alto * 0.53) + 318);
	}
	private: System::Void MenuForm_Resize(System::Object^ sender, System::EventArgs^ e) {
		posicionarBotones();
	}

		   //Boton Jugar
	private: System::Void btnJugar_MouseEnter(System::Object^ sender, System::EventArgs^ e) {
		this->btnJugar->BackgroundImage = imgJugarEnter;
		this->BackgroundImageLayout = ImageLayout::Stretch;
		reproductorEfectos->URL = "msc/sonidoSeleccion.wav";
		reproductorEfectos->controls->play();
	}
	private: System::Void btnJugar_MouseLeave(System::Object^ sender, System::EventArgs^ e) {
		this->btnJugar->BackgroundImage = imgJugarLeave;
		this->BackgroundImageLayout = ImageLayout::Stretch;
	}
	private: System::Void btnJugar_Click(System::Object^ sender, System::EventArgs^ e) {
		reproductorEfectos->URL = "msc/sonidoClick.wav";
		reproductorEfectos->controls->play();

		//Cerrar el menu y abrir el juego (siempre una partida nueva)
		this->Hide();
		reproductorMusica->controls->stop();
		JuegoForm^ ventanaJuego = gcnew JuegoForm();
		ventanaJuego->ShowDialog();
		delete ventanaJuego;
		this->Show();
		reproductorMusica->controls->play();

	}

		   //Boton Instrucciones
	private: System::Void btnInstruccion_MouseEnter(System::Object^ sender, System::EventArgs^ e) {
		this->btnInstruccion->BackgroundImage = imgInstruccionesEnter;
		this->BackgroundImageLayout = ImageLayout::Stretch;
		reproductorEfectos->URL = "msc/sonidoSeleccion.wav";
		reproductorEfectos->controls->play();
	}
	private: System::Void btnInstruccion_MouseLeave(System::Object^ sender, System::EventArgs^ e) {
		this->btnInstruccion->BackgroundImage = imgInstruccionesLeave;
		this->BackgroundImageLayout = ImageLayout::Stretch;
	}
	private: System::Void btnInstruccion_Click(System::Object^ sender, System::EventArgs^ e) {
		reproductorEfectos->URL = "msc/sonidoClick.wav";
		reproductorEfectos->controls->play();
		//Abrir menu instrucciones
		this->Hide();
		ventanaInstrucciones->ShowDialog();
		this->Show();
	}

		   //Boton Creditos
	private: System::Void btnCreditos_MouseEnter(System::Object^ sender, System::EventArgs^ e) {
		this->btnCreditos->BackgroundImage = imgCreditosEnter;
		this->BackgroundImageLayout = ImageLayout::Stretch;
		reproductorEfectos->URL = "msc/sonidoSeleccion.wav";
		reproductorEfectos->controls->play();
	}
	private: System::Void btnCreditos_MouseLeave(System::Object^ sender, System::EventArgs^ e) {
		this->btnCreditos->BackgroundImage = imgCreditosLeave;
		this->BackgroundImageLayout = ImageLayout::Stretch;
	}
	private: System::Void btnCreditos_Click(System::Object^ sender, System::EventArgs^ e) {
		reproductorEfectos->URL = "msc/sonidoClick.wav";
		reproductorEfectos->controls->play();
		//Abrir menu creditos
		this->Hide();
		ventanaCreditos->ShowDialog();
		this->Show();
	}

		   //Boton Salir
	private: System::Void btnSalir_MouseEnter(System::Object^ sender, System::EventArgs^ e) {
		this->btnSalir->BackgroundImage = imgSalirEnter;
		this->BackgroundImageLayout = ImageLayout::Stretch;
		reproductorEfectos->URL = "msc/sonidoSeleccion.wav";
		reproductorEfectos->controls->play();
	}
	private: System::Void btnSalir_MouseLeave(System::Object^ sender, System::EventArgs^ e) {
		this->btnSalir->BackgroundImage = imgSalirLeave;
		this->BackgroundImageLayout = ImageLayout::Stretch;
	}

	private: System::Void btnSalir_Click(System::Object^ sender, System::EventArgs^ e) {
		reproductorEfectos->URL = "msc/sonidoClick.wav";
		reproductorEfectos->controls->play();
		this->Close();
	}



	};
}
