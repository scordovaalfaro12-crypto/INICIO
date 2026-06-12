#pragma once

namespace ManuscritoHuarochiri {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace WMPLib; // Musica


	public ref class CreditosForm : public System::Windows::Forms::Form {

		Bitmap^ creditos1920x1200;
		Bitmap^ imgBotonSalirEnter;
		Bitmap^ imgBotonSalirLeave;

		WMPLib::WindowsMediaPlayerClass^ reproductorEfectos;


	public:
		CreditosForm(void)
		{
			InitializeComponent();
			creditos1920x1200 = gcnew Bitmap("imgMenu/creditos1920x1200.png");
			this->BackgroundImage = creditos1920x1200;
			this->BackgroundImageLayout = ImageLayout::Stretch;

			imgBotonSalirEnter = gcnew Bitmap("imgMenu/regresarMenuEnter.png");
			imgBotonSalirLeave = gcnew Bitmap("imgMenu/regresarMenuLeave.png");

			reproductorEfectos = gcnew WMPLib::WindowsMediaPlayerClass();

			posicionarBoton();
		}

	protected:

		~CreditosForm()
		{
			if (components)
			{
				delete components;
				delete creditos1920x1200;
				delete imgBotonSalirEnter;
				delete imgBotonSalirLeave;
				delete reproductorEfectos;
			}
		}

	private:
	private: System::Windows::Forms::Button^ btnRegresarMenu;
		System::ComponentModel::Container^ components;

#pragma region Windows Form Designer generated code

		void InitializeComponent(void)
		{
			System::ComponentModel::ComponentResourceManager^ resources = (gcnew System::ComponentModel::ComponentResourceManager(CreditosForm::typeid));
			this->btnRegresarMenu = (gcnew System::Windows::Forms::Button());
			this->SuspendLayout();
			//
			// btnRegresarMenu
			//
			this->btnRegresarMenu->BackColor = System::Drawing::Color::Transparent;
			this->btnRegresarMenu->BackgroundImage = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"btnRegresarMenu.BackgroundImage")));
			this->btnRegresarMenu->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
			this->btnRegresarMenu->FlatAppearance->BorderSize = 0;
			this->btnRegresarMenu->FlatAppearance->MouseDownBackColor = System::Drawing::Color::Transparent;
			this->btnRegresarMenu->FlatAppearance->MouseOverBackColor = System::Drawing::Color::Transparent;
			this->btnRegresarMenu->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->btnRegresarMenu->Location = System::Drawing::Point(562, 930);
			this->btnRegresarMenu->Name = L"btnRegresarMenu";
			this->btnRegresarMenu->Size = System::Drawing::Size(912, 189);
			this->btnRegresarMenu->TabIndex = 0;
			this->btnRegresarMenu->UseVisualStyleBackColor = false;
			this->btnRegresarMenu->Click += gcnew System::EventHandler(this, &CreditosForm::btnRegresarMenu_Click);
			this->btnRegresarMenu->MouseEnter += gcnew System::EventHandler(this, &CreditosForm::btnRegresarMenu_MouseEnter);
			this->btnRegresarMenu->MouseLeave += gcnew System::EventHandler(this, &CreditosForm::btnRegresarMenu_MouseLeave);
			//
			// CreditosForm
			//
			this->AutoScaleDimensions = System::Drawing::SizeF(8, 16);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->BackgroundImage = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"$this.BackgroundImage")));
			this->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
			this->ClientSize = System::Drawing::Size(1920, 1200);
			this->Controls->Add(this->btnRegresarMenu);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::None;
			this->Name = L"CreditosForm";
			this->Text = L"CreditosForm";
			this->WindowState = System::Windows::Forms::FormWindowState::Maximized;
			this->Resize += gcnew System::EventHandler(this, &CreditosForm::CreditosForm_Resize);
			this->ResumeLayout(false);

		}
#pragma endregion

	private: void posicionarBoton() {
		this->btnRegresarMenu->Location = System::Drawing::Point(
			this->ClientSize.Width / 2 - this->btnRegresarMenu->Width / 2,
			(int)(this->ClientSize.Height * 0.775));
	}
	private: System::Void CreditosForm_Resize(System::Object^ sender, System::EventArgs^ e) {
		posicionarBoton();
	}
	private: System::Void btnRegresarMenu_Click(System::Object^ sender, System::EventArgs^ e) {
		reproductorEfectos->URL = "msc/sonidoClick.wav";
		reproductorEfectos->controls->play();
		this->Close();

	}
	private: System::Void btnRegresarMenu_MouseEnter(System::Object^ sender, System::EventArgs^ e) {
		this->btnRegresarMenu->BackgroundImage = imgBotonSalirEnter;
		this->BackgroundImageLayout = ImageLayout::Stretch;
		reproductorEfectos->URL = "msc/sonidoSeleccion.wav";
		reproductorEfectos->controls->play();
	}
	private: System::Void btnRegresarMenu_MouseLeave(System::Object^ sender, System::EventArgs^ e) {
		this->btnRegresarMenu->BackgroundImage = imgBotonSalirLeave;
		this->BackgroundImageLayout = ImageLayout::Stretch;
	}
	};
}
