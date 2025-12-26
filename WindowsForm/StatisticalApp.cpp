// StatisticalApp.cpp
#include "MainForm.h"

using namespace System;
using namespace System::Windows::Forms;

[STAThread]
int main(array<System::String^>^ args)
{
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false);

    StatisticalApp::MainForm^ mainForm = gcnew StatisticalApp::MainForm();
    Application::Run(mainForm);

    return 0;
}
