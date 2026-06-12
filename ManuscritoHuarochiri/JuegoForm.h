#pragma once
#include "Juego.h"

namespace ManuscritoHuarochiri {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	// Ventana donde transcurren los 3 niveles. Toda la logica vive en Juego;
	// este formulario solo captura el teclado y dibuja con doble buffer.
	public ref class JuegoForm : public System::Windows::Forms::Form {
		Juego^ juego;

	public:
		JuegoForm(void) {
			InitializeComponent();
			juego = gcnew Juego();
		}

	protected:
		~JuegoForm() {
			if (components) {
				delete components;
			}
			if (juego) {
				delete juego;
				juego = nullptr;
			}
		}

	private: System::Windows::Forms::Timer^ timer1;
	private: System::ComponentModel::IContainer^ components;

#pragma region Windows Form Designer generated code

		void InitializeComponent(void) {
			this->components = (gcnew System::ComponentModel::Container());
			this->timer1 = (gcnew System::Windows::Forms::Timer(this->components));
			this->SuspendLayout();
			//
			// timer1
			//
			this->timer1->Enabled = true;
			this->timer1->Interval = 30;
			this->timer1->Tick += gcnew System::EventHandler(this, &JuegoForm::timer1_Tick);
			//
			// JuegoForm
			//
			this->AutoScaleDimensions = System::Drawing::SizeF(8, 16);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->BackColor = System::Drawing::Color::Black;
			this->ClientSize = System::Drawing::Size(1280, 800);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::None;
			this->Name = L"JuegoForm";
			this->Text = L"Manuscrito de Huarochiri";
			this->WindowState = System::Windows::Forms::FormWindowState::Maximized;
			this->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &JuegoForm::JuegoForm_KeyDown);
			this->KeyUp += gcnew System::Windows::Forms::KeyEventHandler(this, &JuegoForm::JuegoForm_KeyUp);
			this->ResumeLayout(false);
		}
#pragma endregion

	private: System::Void timer1_Tick(System::Object^ sender, System::EventArgs^ e) {
		if (juego == nullptr) return;
		juego->actualizar();
		if (juego->getSalirAlMenu()) {
			timer1->Enabled = false;
			this->Close();
			return;
		}
		Graphics^ g = this->CreateGraphics();
		BufferedGraphicsContext^ bfc = BufferedGraphicsManager::Current;
		BufferedGraphics^ bf = bfc->Allocate(g, this->ClientRectangle);

		juego->dibujar(bf->Graphics, this->ClientSize.Width, this->ClientSize.Height);

		bf->Render(g);
		delete bf;
		delete g;
	}

	private: System::Void JuegoForm_KeyDown(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e) {
		if (e->KeyCode == Keys::Escape) {
			timer1->Enabled = false;
			this->Close();
			return;
		}
		if (juego != nullptr) juego->teclaCambiada(e->KeyCode, true);
	}

	private: System::Void JuegoForm_KeyUp(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e) {
		if (juego != nullptr) juego->teclaCambiada(e->KeyCode, false);
	}
	};
}
