#pragma once

namespace ManuscritoHuarochiri {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace WMPLib; // Musica

	public ref class MenuInstruccionesForm : public System::Windows::Forms::Form {
		Bitmap^ instrucciones1920x1200;
		Bitmap^ imgBotonSalirEnter;
		Bitmap^ imgBotonSalirLeave;

		WMPLib::WindowsMediaPlayerClass^ reproductorEfectos;

	public:
		MenuInstruccionesForm(void)
		{
			InitializeComponent();
			instrucciones1920x1200 = gcnew Bitmap("imgMenu/instrucciones1920x1200.png");
			this->BackgroundImage = instrucciones1920x1200;
			this->BackgroundImageLayout = ImageLayout::Stretch;

			imgBotonSalirEnter = gcnew Bitmap("imgMenu/regresarMenuEnter.png");
			imgBotonSalirLeave = gcnew Bitmap("imgMenu/regresarMenuLeave.png");

			reproductorEfectos = gcnew WMPLib::WindowsMediaPlayerClass();

			posicionarBoton();
		}

	protected:

		~MenuInstruccionesForm()
		{
			if (components)
			{
				delete components;
				delete instrucciones1920x1200;
				delete imgBotonSalirEnter;
				delete imgBotonSalirLeave;
				delete reproductorEfectos;
			}
		}
	private: System::Windows::Forms::Button^ btnMenuPrincipal;
	protected:

	private:

		System::ComponentModel::Container^ components;

#pragma region Windows Form Designer generated code

		void InitializeComponent(void)
		{
			System::ComponentModel::ComponentResourceManager^ resources = (gcnew System::ComponentModel::ComponentResourceManager(MenuInstruccionesForm::typeid));
			this->btnMenuPrincipal = (gcnew System::Windows::Forms::Button());
			this->SuspendLayout();
			//
			// btnMenuPrincipal
			//
			this->btnMenuPrincipal->BackColor = System::Drawing::Color::Transparent;
			this->btnMenuPrincipal->BackgroundImage = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"btnMenuPrincipal.BackgroundImage")));
			this->btnMenuPrincipal->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
			this->btnMenuPrincipal->FlatAppearance->BorderSize = 0;
			this->btnMenuPrincipal->FlatAppearance->MouseDownBackColor = System::Drawing::Color::Transparent;
			this->btnMenuPrincipal->FlatAppearance->MouseOverBackColor = System::Drawing::Color::Transparent;
			this->btnMenuPrincipal->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->btnMenuPrincipal->Location = System::Drawing::Point(594, 943);
			this->btnMenuPrincipal->Name = L"btnMenuPrincipal";
			this->btnMenuPrincipal->Size = System::Drawing::Size(850, 169);
			this->btnMenuPrincipal->TabIndex = 0;
			this->btnMenuPrincipal->UseVisualStyleBackColor = false;
			this->btnMenuPrincipal->Click += gcnew System::EventHandler(this, &MenuInstruccionesForm::btnMenuPrincipal_Click);
			this->btnMenuPrincipal->MouseEnter += gcnew System::EventHandler(this, &MenuInstruccionesForm::btnMenuPrincipal_MouseEnter);
			this->btnMenuPrincipal->MouseLeave += gcnew System::EventHandler(this, &MenuInstruccionesForm::btnMenuPrincipal_MouseLeave);
			//
			// MenuInstruccionesForm
			//
			this->AutoScaleDimensions = System::Drawing::SizeF(8, 16);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->BackgroundImage = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"$this.BackgroundImage")));
			this->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
			this->ClientSize = System::Drawing::Size(1920, 1200);
			this->Controls->Add(this->btnMenuPrincipal);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::None;
			this->Icon = (cli::safe_cast<System::Drawing::Icon^>(resources->GetObject(L"$this.Icon")));
			this->Name = L"MenuInstruccionesForm";
			this->Text = L"MenuInstruccionesForm";
			this->WindowState = System::Windows::Forms::FormWindowState::Maximized;
			this->Resize += gcnew System::EventHandler(this, &MenuInstruccionesForm::MenuInstruccionesForm_Resize);
			this->ResumeLayout(false);

		}
#pragma endregion

	private: void posicionarBoton() {
		this->btnMenuPrincipal->Location = System::Drawing::Point(
			this->ClientSize.Width / 2 - this->btnMenuPrincipal->Width / 2,
			(int)(this->ClientSize.Height * 0.78));
	}
	private: System::Void MenuInstruccionesForm_Resize(System::Object^ sender, System::EventArgs^ e) {
		posicionarBoton();
	}
	private: System::Void btnMenuPrincipal_Click(System::Object^ sender, System::EventArgs^ e) {
		reproductorEfectos->URL = "msc/sonidoClick.wav";
		reproductorEfectos->controls->play();
		this->Close();
	}
	private: System::Void btnMenuPrincipal_MouseEnter(System::Object^ sender, System::EventArgs^ e) {
		this->btnMenuPrincipal->BackgroundImage = imgBotonSalirEnter;
		this->BackgroundImageLayout = ImageLayout::Stretch;
		reproductorEfectos->URL = "msc/sonidoSeleccion.wav";
		reproductorEfectos->controls->play();
	}
	private: System::Void btnMenuPrincipal_MouseLeave(System::Object^ sender, System::EventArgs^ e) {
		this->btnMenuPrincipal->BackgroundImage = imgBotonSalirLeave;
		this->BackgroundImageLayout = ImageLayout::Stretch;
	}
	};
}
