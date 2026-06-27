#pragma once
//==============================================================================
//  MyForm.h  -  Declaracion del formulario principal (Windows Forms / C++ CLR)
//------------------------------------------------------------------------------
//  Crea el lienzo, el temporizador (cronometro) y delega TODA la logica del
//  juego al objeto Controlador. La implementacion de los manejadores de eventos
//  esta en MyForm.cpp.
//==============================================================================
#include "Controlador.h"

namespace LB2_Bathero {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	public ref class MyForm : public System::Windows::Forms::Form {
	public:
		MyForm(void) {
			InitializeComponent();
			ctrl = gcnew Controlador(ANCHO, ALTO);
		}

	protected:
		~MyForm() {
			if (components) delete components;
			if (ctrl != nullptr) ctrl->Liberar(); // liberar recursos del juego
		}

	private:
		static const int ANCHO = 1000;
		static const int ALTO = 600;

		Controlador^ ctrl;
		System::Windows::Forms::Panel^ lienzo;
		System::Windows::Forms::Timer^ cronometro;
		System::ComponentModel::IContainer^ components;

		// Manejadores de eventos (implementados en MyForm.cpp).
		void MyForm_Load(System::Object^ sender, System::EventArgs^ e);
		void cronometro_Tick(System::Object^ sender, System::EventArgs^ e);
		void MyForm_KeyDown(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e);
		void MyForm_KeyUp(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e);
		void MyForm_PreviewKeyDown(System::Object^ sender, System::Windows::Forms::PreviewKeyDownEventArgs^ e);

#pragma region Windows Form Designer generated code
		void InitializeComponent(void) {
			this->components = gcnew System::ComponentModel::Container();
			this->lienzo = gcnew System::Windows::Forms::Panel();
			this->cronometro = gcnew System::Windows::Forms::Timer(this->components);
			this->SuspendLayout();
			//
			// lienzo
			//
			this->lienzo->BackColor = System::Drawing::Color::Black;
			this->lienzo->Location = System::Drawing::Point(0, 0);
			this->lienzo->Name = L"lienzo";
			this->lienzo->Size = System::Drawing::Size(ANCHO, ALTO);
			this->lienzo->TabStop = false;
			//
			// cronometro
			//
			this->cronometro->Interval = 20; // ~50 cuadros por segundo
			this->cronometro->Tick += gcnew System::EventHandler(this, &MyForm::cronometro_Tick);
			//
			// MyForm
			//
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(ANCHO, ALTO);
			this->Controls->Add(this->lienzo);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
			this->KeyPreview = true;
			this->MaximizeBox = false;
			this->Name = L"MyForm";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			this->Text = L"LB2 - Bathero atrapa villanos con su bumeran";
			this->Load += gcnew System::EventHandler(this, &MyForm::MyForm_Load);
			this->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &MyForm::MyForm_KeyDown);
			this->KeyUp += gcnew System::Windows::Forms::KeyEventHandler(this, &MyForm::MyForm_KeyUp);
			this->PreviewKeyDown += gcnew System::Windows::Forms::PreviewKeyDownEventHandler(this, &MyForm::MyForm_PreviewKeyDown);
			this->ResumeLayout(false);
		}
#pragma endregion
	};
}
