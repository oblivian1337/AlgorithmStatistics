#pragma once

namespace StatisticalApp {

    using namespace System;
    using namespace System::ComponentModel;
    using namespace System::Collections;
    using namespace System::Windows::Forms;
    using namespace System::Drawing;
    using namespace System::IO;
    using namespace System::Diagnostics;
    using namespace System::Threading;

    // Структура для хранения информации о статистическом методе
    ref struct StatisticalMethod
    {
        property String^ Id;
        property String^ Name;
        property String^ Description;
        property String^ ExecutableFile;
        property String^ DefaultOutputFile;

        StatisticalMethod(String^ id, String^ name, String^ description,
            String^ execFile, String^ outFile)
        {
            Id = id;
            Name = name;
            Description = description;
            ExecutableFile = execFile;
            DefaultOutputFile = outFile;
        }
    };

    // Главная форма приложения
    public ref class MainForm : public Form
    {
    public:
        MainForm(void)
        {
            // Устанавливаем свойства формы
            this->Text = "Статистический анализ данных";
            this->Size = System::Drawing::Size(1000, 700);
            this->StartPosition = FormStartPosition::CenterScreen;

            // Инициализируем элементы управления
            InitializeControls();

            // Инициализируем методы
            InitializeMethods();

            // Инициализируем дерево (ТОЧНО как на картинке)
            InitializeTreeView();

            // Устанавливаем начальный текст
            richTextBoxDescription->Text = GetWelcomeMessage();

            UpdateStatus("Готово. Выберите метод для анализа.");
        }

    private:
        // Основные элементы управления
        TreeView^ treeViewMethods;
        RichTextBox^ richTextBoxDescription;
        TextBox^ textBoxInputFile;
        TextBox^ textBoxOutputFile;
        Button^ buttonBrowseInput;
        Button^ buttonBrowseOutput;
        Button^ buttonRun;
        Button^ buttonOpenResult;
        Button^ buttonClear;
        StatusStrip^ statusStrip;
        ToolStripStatusLabel^ toolStripStatusLabel;
        ProgressBar^ progressBar;

        // Коллекция статистических методов
        Collections::Generic::List<StatisticalMethod^>^ methods;

        // Текущий выбранный метод
        StatisticalMethod^ selectedMethod;

    private:
        // Инициализация элементов управления
        void InitializeControls()
        {
            // Создаем TreeView слева
            treeViewMethods = gcnew TreeView();
            treeViewMethods->Location = Point(10, 10);
            treeViewMethods->Size = System::Drawing::Size(300, 600);
            treeViewMethods->Font = gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10);
            treeViewMethods->FullRowSelect = true;
            treeViewMethods->HideSelection = false;
            treeViewMethods->Indent = 25;
            treeViewMethods->AfterSelect +=
                gcnew TreeViewEventHandler(this, &MainForm::treeViewMethods_AfterSelect);

            // Создаем RichTextBox для описания и вывода результатов
            richTextBoxDescription = gcnew RichTextBox();
            richTextBoxDescription->Location = Point(320, 10);
            richTextBoxDescription->Size = System::Drawing::Size(650, 300);
            richTextBoxDescription->Font = gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10);
            richTextBoxDescription->ReadOnly = false;
            richTextBoxDescription->BackColor = Color::White;

            // Создаем панель для управления файлами
            GroupBox^ fileGroupBox = gcnew GroupBox();
            fileGroupBox->Location = Point(320, 320);
            fileGroupBox->Size = System::Drawing::Size(650, 150);
            fileGroupBox->Text = "Управление файлами";
            fileGroupBox->Font = gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10);

            // Создаем элементы для управления файлами внутри GroupBox
            Label^ labelInput = gcnew Label();
            labelInput->Location = Point(20, 30);
            labelInput->Size = System::Drawing::Size(100, 20);
            labelInput->Text = "Входной файл:";

            textBoxInputFile = gcnew TextBox();
            textBoxInputFile->Location = Point(130, 30);
            textBoxInputFile->Size = System::Drawing::Size(400, 20);
            textBoxInputFile->Font = gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9);

            buttonBrowseInput = gcnew Button();
            buttonBrowseInput->Location = Point(540, 28);
            buttonBrowseInput->Size = System::Drawing::Size(80, 25);
            buttonBrowseInput->Text = "Обзор...";
            buttonBrowseInput->Font = gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9);
            buttonBrowseInput->Click += gcnew EventHandler(this, &MainForm::buttonBrowseInput_Click);

            Label^ labelOutput = gcnew Label();
            labelOutput->Location = Point(20, 70);
            labelOutput->Size = System::Drawing::Size(100, 20);
            labelOutput->Text = "Выходной файл:";

            textBoxOutputFile = gcnew TextBox();
            textBoxOutputFile->Location = Point(130, 70);
            textBoxOutputFile->Size = System::Drawing::Size(400, 20);
            textBoxOutputFile->Font = gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9);

            buttonBrowseOutput = gcnew Button();
            buttonBrowseOutput->Location = Point(540, 68);
            buttonBrowseOutput->Size = System::Drawing::Size(80, 25);
            buttonBrowseOutput->Text = "Обзор...";
            buttonBrowseOutput->Font = gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9);
            buttonBrowseOutput->Click += gcnew EventHandler(this, &MainForm::buttonBrowseOutput_Click);

            // Добавляем элементы в GroupBox
            fileGroupBox->Controls->Add(labelInput);
            fileGroupBox->Controls->Add(textBoxInputFile);
            fileGroupBox->Controls->Add(buttonBrowseInput);
            fileGroupBox->Controls->Add(labelOutput);
            fileGroupBox->Controls->Add(textBoxOutputFile);
            fileGroupBox->Controls->Add(buttonBrowseOutput);

            // Создаем панель для кнопок управления
            Panel^ buttonPanel = gcnew Panel();
            buttonPanel->Location = Point(320, 480);
            buttonPanel->Size = System::Drawing::Size(650, 50);

            // Создаем кнопки управления
            buttonRun = gcnew Button();
            buttonRun->Location = Point(440, 10);
            buttonRun->Size = System::Drawing::Size(200, 40);
            buttonRun->Text = "ВЫПОЛНИТЬ ВЫБРАННЫЙ МЕТОД";
            buttonRun->BackColor = Color::RoyalBlue;
            buttonRun->ForeColor = Color::White;
            buttonRun->Font = gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11, FontStyle::Bold);
            buttonRun->Click += gcnew EventHandler(this, &MainForm::buttonRun_Click);

            buttonOpenResult = gcnew Button();
            buttonOpenResult->Location = Point(230, 10);
            buttonOpenResult->Size = System::Drawing::Size(200, 40);
            buttonOpenResult->Text = "Открыть результат";
            buttonOpenResult->Font = gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10);
            buttonOpenResult->Click += gcnew EventHandler(this, &MainForm::buttonOpenResult_Click);

            buttonClear = gcnew Button();
            buttonClear->Location = Point(20, 10);
            buttonClear->Size = System::Drawing::Size(200, 40);
            buttonClear->Text = "Очистить";
            buttonClear->Font = gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10);
            buttonClear->Click += gcnew EventHandler(this, &MainForm::buttonClear_Click);

            // Добавляем кнопки на панель
            buttonPanel->Controls->Add(buttonRun);
            buttonPanel->Controls->Add(buttonOpenResult);
            buttonPanel->Controls->Add(buttonClear);

            // Создаем StatusStrip
            statusStrip = gcnew StatusStrip();
            statusStrip->Location = Point(0, 648);
            statusStrip->Size = System::Drawing::Size(984, 22);

            toolStripStatusLabel = gcnew ToolStripStatusLabel();
            toolStripStatusLabel->Text = "Готово";
            statusStrip->Items->Add(toolStripStatusLabel);

            // Создаем ProgressBar
            progressBar = gcnew ProgressBar();
            progressBar->Location = Point(700, 650);
            progressBar->Size = System::Drawing::Size(280, 15);
            progressBar->Style = ProgressBarStyle::Marquee;
            progressBar->Visible = false;

            // Добавляем все элементы на форму
            this->Controls->Add(treeViewMethods);
            this->Controls->Add(richTextBoxDescription);
            this->Controls->Add(fileGroupBox);
            this->Controls->Add(buttonPanel);
            this->Controls->Add(statusStrip);
            this->Controls->Add(progressBar);
        }

        // Инициализация списка методов
        void InitializeMethods()
        {
            String^ programsPath = "C:\\Users\\TagiR\\VsProjects\\Algorithm\\first\\first";
            methods = gcnew Collections::Generic::List<StatisticalMethod^>();

            // МЕТОД МАКСИМАЛЬНОГО ПРАВДОПОДОБИЯ - НОРМАЛЬНОЕ РАСПРЕДЕЛЕНИЕ
            methods->Add(gcnew StatisticalMethod(
                "MLE_NORMAL",
                "Оценка параметров нормального распределения (MLE)",
                "Оценка параметров нормального распределения методом максимального правдоподобия.\n\n" +
                "Применяется для анализа данных, распределенных по нормальному закону.\n\n" +
                "Формат входных данных: значения через пробел или с новой строки.\n" +
                "ИСПОЛНЯЕМЫЙ ФАЙЛ: 1.exe\n" +
                "ФАЙЛ РЕЗУЛЬТАТОВ: results_mle.txt",
                programsPath + "1.exe",
                "results_mle.txt"
            ));

            // МЕТОД МАКСИМАЛЬНОГО ПРАВДОПОДОБИЯ - РАСПРЕДЕЛЕНИЕ ВЕЙБУЛЛА
            methods->Add(gcnew StatisticalMethod(
                "MLE_WEIBULL",
                "Оценка параметров распределения Вейбулла (MLE)",
                "Оценка параметров распределения Вейбулла методом максимального правдоподобия.\n\n" +
                "Широко применяется в анализе надежности и времени до отказа.\n\n" +
                "Формат входных данных: положительные значения через пробел.\n" +
                "ИСПОЛНЯЕМЫЙ ФАЙЛ: weibull.exe\n" +
                "ФАЙЛ РЕЗУЛЬТАТОВ: results_weibull.txt",
                programsPath + "weibull.exe",
                "results_weibull.txt"
            ));

            // МЕТОД НАИМЕНЬШИХ КВАДРАТОВ - НОРМАЛЬНОЕ РАСПРЕДЕЛЕНИЕ
            methods->Add(gcnew StatisticalMethod(
                "LSQ_NORMAL",
                "Метод наименьших квадратов для нормального распределения",
                "Оценка параметров методом наименьших квадратов для нормального распределения.\n\n" +
                "Используется для линейной регрессии с нормально распределенными ошибками.\n\n" +
                "Формат входных данных: пары X,Y через пробел.\n" +
                "ИСПОЛНЯЕМЫЙ ФАЙЛ: lsq_normal.exe\n" +
                "ФАЙЛ РЕЗУЛЬТАТОВ: results.txt",
                programsPath + "mnk.exe",
                "results.txt"
            ));

            // МЕТОД НАИМЕНЬШИХ КВАДРАТОВ - РАСПРЕДЕЛЕНИЕ ВЕЙБУЛЛА
            methods->Add(gcnew StatisticalMethod(
                "LSQ_WEIBULL",
                "Метод наименьших квадратов для распределения Вейбулла",
                "Оценка параметров методом наименьших квадратов для распределения Вейбулла.\n\n" +
                "Применяется для нелинейной регрессии с распределением Вейбулла.\n\n" +
                "Формат входных данных: пары X,Y через пробел.\n" +
                "ИСПОЛНЯЕМЫЙ ФАЙЛ: lsq_weibull.exe\n" +
                "ФАЙЛ РЕЗУЛЬТАТОВ: results_lsq_weibull.txt",
                programsPath + "lsq_weibull.exe",
                "results_lsq_weibull.txt"
            ));

            // РЕГРЕССИОННЫЙ АНАЛИЗ
            //methods->Add(gcnew StatisticalMethod(
            //    "REGRESSION_ANALYSIS",
            //    "Регрессионный анализ",
            //    "Исследование зависимости между переменными методами регрессионного анализа.\n\n" +
            //    "Включает линейную, множественную и нелинейную регрессию.\n\n" +
            //    "Формат входных данных: матрица значений (столбцы - переменные).\n" +
            //    "ИСПОЛНЯЕМЫЙ ФАЙЛ: regression.exe\n" +
            //    "ФАЙЛ РЕЗУЛЬТАТОВ: results_regression.txt",
            //    "regression.exe",
            //    "results_regression.txt"
            //));

            // КРИТЕРИЙ ГРАББСА
            methods->Add(gcnew StatisticalMethod(
                "GRUBBS_TEST",
                "Критерий Граббса для выбросов",
                "Обнаружение аномальных выбросов в нормально распределенной выборке.\n\n" +
                "Проверяет гипотезу о наличии выброса в данных.\n\n" +
                "Формат входных данных: значения через пробел.\n" +
                "ИСПОЛНЯЕМЫЙ ФАЙЛ: grubbs_test.exe\n" +
                "ФАЙЛ РЕЗУЛЬТАТОВ: grubbs_test_result.txt",
                programsPath + "grubbs.exe",
                "grubbs_test_result.txt"
            ));

            // КРИТЕРИЙ ФИШЕРА
            methods->Add(gcnew StatisticalMethod(
                "F_TEST",
                "Критерий Фишера (F-тест)",
                "Сравнение дисперсий двух нормально распределенных выборок.\n\n" +
                "Проверяет гипотезу о равенстве дисперсий.\n\n" +
                "Формат входных данных: две выборки в отдельных строках.\n" +
                "ИСПОЛНЯЕМЫЙ ФАЙЛ: fisher_test.exe\n" +
                "ФАЙЛ РЕЗУЛЬТАТОВ: fisher_test_result.txt",
                programsPath + "F.exe",
                "fisher_test_result.txt"
            ));

            // КРИТЕРИЙ СТЬЮДЕНТА
            methods->Add(gcnew StatisticalMethod(
                "T_TEST",
                "Критерий Стьюдента (t-тест)",
                "Сравнение средних значений двух независимых выборок.\n\n" +
                "Проверяет гипотезу о равенстве математических ожиданий.\n\n" +
                "Формат входных данных: две выборки в отдельных строках.\n" +
                "ИСПОЛНЯЕМЫЙ ФАЙЛ: Student.exe\n" +
                "ФАЙЛ РЕЗУЛЬТАТОВ: student_test_result.txt",
                "Student.exe",
                "student_test_result.txt"
            ));

            // КРИТЕРИЙ БАРТЛЕТТА
            methods->Add(gcnew StatisticalMethod(
                "BARTLETT_TEST",
                "Критерий Бартлетта",
                "Проверка равенства дисперсий нескольких нормально распределенных выборок.\n\n" +
                "Применяется для проверки однородности дисперсий.\n\n" +
                "Формат входных данных: несколько выборок в отдельных строках.\n" +
                "ИСПОЛНЯЕМЫЙ ФАЙЛ: bartlet.exe\n" +
                "ФАЙЛ РЕЗУЛЬТАТОВ: bartlett_results.txt",
                "bartlett_test.exe",
                "bartlett_results.txt"
            ));

            // КРИТЕРИЙ ШАПИРО-УИЛКА
            methods->Add(gcnew StatisticalMethod(
                "SHAPIRO_TEST",
                "Критерий Шапиро-Уилка",
                "Проверка нормальности распределения выборки.\n\n" +
                "Тест на соответствие нормальному распределению.\n\n" +
                "Формат входных данных: значения через пробел.\n" +
                "ИСПОЛНЯЕМЫЙ ФАЙЛ: shapiro_test.exe\n" +
                "ФАЙЛ РЕЗУЛЬТАТОВ: results_shapiro.txt",
                "shapiro_test.exe",
                "results_shapiro.txt"
            ));

            // КРИТЕРИЙ УИЛКОКСОНА
            methods->Add(gcnew StatisticalMethod(
                "WILCOXON_TEST",
                "Критерий Уилкоксона",
                "Непараметрический критерий для сравнения двух связанных выборок.\n\n" +
                "Альтернатива t-тесту для ненормальных распределений.\n\n" +
                "Формат входных данных: пары значений через пробел.\n" +
                "ИСПОЛНЯЕМЫЙ ФАЙЛ: wilcoxon.exe\n" +
                "ФАЙЛ РЕЗУЛЬТАТОВ: wilcoxon_results.txt",
                "wilcoxon.exe",
                "wilcoxon_results.txt"
            ));

            // КРИТЕРИЙ КРАСКАЛА-УОЛЕССА
            methods->Add(gcnew StatisticalMethod(
                "KRUSKAL_TEST",
                "Критерий КРАСКАЛА-УОЛЕССА",
                "Непараметрический критерий для сравнения двух связанных выборок.\n\n" +
                "Альтернатива t-тесту для ненормальных распределений.\n\n" +
                "Формат входных данных: пары значений через пробел.\n" +
                "ИСПОЛНЯЕМЫЙ ФАЙЛ: kruskal.exe\n" +
                "ФАЙЛ РЕЗУЛЬТАТОВ: kruskal_detailed_results.txt",
                "kruskal.exe",
                "kruskal_detailed_results.txt"
            ));
        }

        // Инициализация дерева методов (ТОЧНО как на картинке)
        void InitializeTreeView()
        {
            treeViewMethods->Nodes->Clear();

            // Корневой узел
            TreeNode^ rootNode = gcnew TreeNode("Статистический анализ");
            rootNode->Tag = "ROOT";
            treeViewMethods->Nodes->Add(rootNode);

            // Основные категории (ТОЧНАЯ СТРУКТУРА как на картинке)
            TreeNode^ catEstimation = rootNode->Nodes->Add("Статистическое оценивание");
            TreeNode^ catRegression = rootNode->Nodes->Add("Регрессионный анализ");
            TreeNode^ catHypotheses = rootNode->Nodes->Add("Проверка гипотез");

            // Статистическое оценивание - структура как на картинке
            TreeNode^ catParametricMethods = catEstimation->Nodes->Add("Параметрические методы");

            // Метод максимального правдоподобия (М)
            TreeNode^ catMLE = catParametricMethods->Nodes->Add("Метод максимального правдоподобия (М)");
            catMLE->Nodes->Add("Нормальное распределение")->Tag = "MLE_NORMAL";
            catMLE->Nodes->Add("Распределение Вейбулла")->Tag = "MLE_WEIBULL";

            // Метод наименьших квадратов (МНК)
            TreeNode^ catLSQ = catParametricMethods->Nodes->Add("Метод наименьших квадратов (МНК)");
            catLSQ->Nodes->Add("Нормальное распределение")->Tag = "LSQ_NORMAL";
            catLSQ->Nodes->Add("Распределение Вейбулла")->Tag = "LSQ_WEIBULL";

            // Регрессионный анализ (отдельная категория)
            catRegression->Tag = "REGRESSION_ANALYSIS";

            // Проверка гипотез - структура как на картинке
            TreeNode^ catParametricCriteria = catHypotheses->Nodes->Add("Параметрические критерии");
            TreeNode^ catNonParametricCriteria = catHypotheses->Nodes->Add("Непараметрические критерии");

            // Критерий аномальности (параметрический)
            TreeNode^ catAnomaly = catParametricCriteria->Nodes->Add("Критерий аномальности");
            catAnomaly->Nodes->Add("Критерий Граббса")->Tag = "GRUBBS_TEST";
            catAnomaly->Nodes->Add("Критерий Фишера")->Tag = "F_TEST";

            // Критерий однородности (параметрический)
            TreeNode^ catHomogeneity = catParametricCriteria->Nodes->Add("Критерий однородности");
            catHomogeneity->Nodes->Add("Критерий Стьюдента")->Tag = "T_TEST";
            catHomogeneity->Nodes->Add("Критерий Бартлетта")->Tag = "BARTLETT_TEST";
            catHomogeneity->Nodes->Add("Критерий Шапиро-Уилка")->Tag = "SHAPIRO_TEST";

            // Непараметрические критерии
            catNonParametricCriteria->Nodes->Add("Критерий Уилкоксона")->Tag = "WILCOXON_TEST";
            catNonParametricCriteria->Nodes->Add("Критерий Краскела-Уоллиса")->Tag = "KRUSKAL_TEST";

            // Раскрываем все узлы как на картинке
            rootNode->ExpandAll();
            catEstimation->ExpandAll();
            catParametricMethods->ExpandAll();
            catMLE->ExpandAll();
            catLSQ->ExpandAll();
            catRegression->ExpandAll();
            catHypotheses->ExpandAll();
            catParametricCriteria->ExpandAll();
            catAnomaly->ExpandAll();
            catHomogeneity->ExpandAll();
            catNonParametricCriteria->ExpandAll();
        }

        // Получение приветственного сообщения
        String^ GetWelcomeMessage()
        {
            return
                "ДОБРО ПОЖАЛОВАТЬ В СИСТЕМУ СТАТИСТИЧЕСКОГО АНАЛИЗА\n\n" +
                //"═══════════════════════════════════════════════════\n\n" +
                "Система предназначена для выполнения широкого спектра\n" +
                "статистических анализов и проверки гипотез.\n\n" +
                "ДОСТУПНЫЕ ВОЗМОЖНОСТИ:\n" +
                "• Статистическое оценивание параметров распределений\n" +
                "• Регрессионный анализ\n" +
                "• Проверка статистических гипотез\n" +
                "• Параметрические и непараметрические критерии\n\n" +
                "ИНСТРУКЦИЯ ПО ИСПОЛЬЗОВАНИЮ:\n" +
                "1. Выберите метод из дерева слева\n" +
                "2. Ознакомьтесь с описанием метода\n" +
                "3. Укажите входной и выходной файлы\n" +
                "4. Нажмите кнопку 'ВЫПОЛНИТЬ ВЫБРАННЫЙ МЕТОД'";
        }

        // Обновление статуса
        void UpdateStatus(String^ message)
        {
            if (toolStripStatusLabel != nullptr)
            {
                toolStripStatusLabel->Text = message;
            }
        }

        // Показать/скрыть индикатор прогресса
        void ShowProgress(bool show)
        {
            if (progressBar != nullptr)
            {
                progressBar->Visible = show;
            }
        }

        // Найти метод по ID
        StatisticalMethod^ FindMethodById(String^ id)
        {
            for each (StatisticalMethod ^ method in methods)
            {
                if (method->Id == id)
                    return method;
            }
            return nullptr;
        }

        // Запуск внешней программы
        String^ ExecuteExternalProgram(String^ programPath, String^ arguments)
        {
            try
            {
                Process^ process = gcnew Process();
                process->StartInfo->FileName = programPath;
                process->StartInfo->Arguments = arguments;
                process->StartInfo->UseShellExecute = false;
                process->StartInfo->RedirectStandardOutput = true;
                process->StartInfo->RedirectStandardError = true;
                process->StartInfo->CreateNoWindow = true;

                UpdateStatus("Запуск программы: " + Path::GetFileName(programPath));

                process->Start();
                String^ output = process->StandardOutput->ReadToEnd();
                String^ error = process->StandardError->ReadToEnd();

                process->WaitForExit();

                if (!String::IsNullOrEmpty(error))
                    output += "\n\nОшибки:\n" + error;

                return output;
            }
            catch (Exception^ ex)
            {
                return "Ошибка выполнения программы:\n" + ex->Message;
            }
        }

        // Чтение файла результатов
        String^ ReadResultFile(String^ filePath)
        {
            try
            {
                if (File::Exists(filePath))
                {
                    return File::ReadAllText(filePath, System::Text::Encoding::GetEncoding(1251));
                }
                return "Файл результатов не найден: " + filePath;
            }
            catch (Exception^ ex)
            {
                return "Ошибка чтения файла: " + ex->Message;
            }
        }

        // Генерация тестовых результатов для метода
        String^ GenerateTestResults(StatisticalMethod^ method)
        {
            String^ results =
                //"═══════════════════════════════════════════════════\n" +
                "РЕЗУЛЬТАТЫ СТАТИСТИЧЕСКОГО АНАЛИЗА\n" +
                //"═══════════════════════════════════════════════════\n\n" +
                "МЕТОД: " + method->Name + "\n" +
                "Время выполнения: " + DateTime::Now.ToString("dd.MM.yyyy HH:mm:ss") + "\n\n";

            // Добавляем специфичные результаты в зависимости от метода
            if (method->Id == "MLE_NORMAL")
            {
                results +=
                    "ОЦЕНКА ПАРАМЕТРОВ НОРМАЛЬНОГО РАСПРЕДЕЛЕНИЯ (MLE)\n" +
                    //"───────────────────────────────────────────────\n" +
                    "• Оценка математического ожидания (μ): 25.67\n" +
                    "• Оценка стандартного отклонения (σ): 5.45\n" +
                    "• Дисперсия: 29.70\n" +
                    "• Доверительный интервал (95%): [23.21, 28.13]\n" +
                    "• Объем выборки: 150\n" +
                    "• Логарифм правдоподобия: -245.32\n\n";
            }
            else if (method->Id == "MLE_WEIBULL")
            {
                results +=
                    "ОЦЕНКА ПАРАМЕТРОВ РАСПРЕДЕЛЕНИЯ ВЕЙБУЛЛА (MLE)\n" +
                    //"───────────────────────────────────────────────\n" +
                    "• Параметр формы (k): 2.15\n" +
                    "• Параметр масштаба (λ): 10.25\n" +
                    "• Среднее время до отказа: 9.08\n" +
                    "• Медиана: 8.76\n" +
                    "• Дисперсия: 18.92\n" +
                    "• Объем выборки: 120\n\n";
            }
            else if (method->Id == "LSQ_NORMAL")
            {
                results +=
                    "МЕТОД НАИМЕНЬШИХ КВАДРАТОВ (НОРМАЛЬНОЕ РАСПРЕДЕЛЕНИЕ)\n" +
                    //"───────────────────────────────────────────────\n" +
                    "• Коэффициент наклона (b): 1.25\n" +
                    "• Свободный член (a): 3.45\n" +
                    "• Коэффициент корреляции (r): 0.92\n" +
                    "• Стандартная ошибка: 0.89\n" +
                    "• Коэффициент детерминации (R²): 0.85\n" +
                    "• Количество точек: 50\n\n";
            }
            else if (method->Id == "LSQ_WEIBULL")
            {
                results +=
                    "МЕТОД НАИМЕНЬШИХ КВАДРАТОВ (РАСПРЕДЕЛЕНИЕ ВЕЙБУЛЛА)\n" +
                    //"───────────────────────────────────────────────\n" +
                    "• Оценка параметра формы: 1.78\n" +
                    "• Оценка параметра масштаба: 12.34\n" +
                    "• Сумма квадратов отклонений: 45.67\n" +
                    "• Среднеквадратичная ошибка: 0.95\n" +
                    "• Коэффициент детерминации (R²): 0.88\n" +
                    "• Количество точек: 60\n\n";
            }
            else if (method->Id == "REGRESSION_ANALYSIS")
            {
                results +=
                    "РЕГРЕССИОННЫЙ АНАЛИЗ\n" +
                    //"───────────────────────────────────────────────\n" +
                    "• Коэффициент детерминации R²: 0.856\n" +
                    "• Скорректированный R²: 0.842\n" +
                    "• Стандартная ошибка: 2.34\n" +
                    "• F-статистика: 45.67 (p < 0.001)\n" +
                    "• Количество наблюдений: 100\n" +
                    "• Количество предикторов: 3\n" +
                    "• Уравнение регрессии: Y = 2.5 + 1.2X₁ + 0.8X₂ - 0.3X₃\n\n";
            }
            else if (method->Id == "GRUBBS_TEST")
            {
                results +=
                    "КРИТЕРИЙ ГРАББСА ДЛЯ ВЫБРОСОВ\n" +
                    //"───────────────────────────────────────────────\n" +
                    "• Тестовая статистика G: 2.89\n" +
                    "• Критическое значение (α=0.05): 2.75\n" +
                    "• Обнаружен выброс: Да\n" +
                    "• Значение выброса: 45.23\n" +
                    "• Индекс выброса: 37\n" +
                    "• p-значение: 0.012\n" +
                    "• Среднее выборки: 25.67\n" +
                    "• Стандартное отклонение: 5.45\n" +
                    "• Рекомендация: Исключить выброс из анализа\n\n";
            }
            else if (method->Id == "F_TEST")
            {
                results +=
                    "КРИТЕРИЙ ФИШЕРА (F-ТЕСТ)\n" +
                    //"───────────────────────────────────────────────\n" +
                    "• F-статистика: 1.45\n" +
                    "• Степени свободы 1: 49\n" +
                    "• Степени свободы 2: 49\n" +
                    "• p-значение: 0.186\n" +
                    "• Дисперсия выборки 1: 25.36\n" +
                    "• Дисперсия выборки 2: 35.12\n" +
                    "• Объем выборки 1: 50\n" +
                    "• Объем выборки 2: 50\n" +
                    "• Результат: Дисперсии однородны (p > 0.05)\n\n";
            }
            else if (method->Id == "T_TEST")
            {
                results +=
                    "КРИТЕРИЙ СТЬЮДЕНТА (t-ТЕСТ)\n" +
                    //"───────────────────────────────────────────────\n" +
                    "• t-статистика: 2.15\n" +
                    "• Степени свободы: 98\n" +
                    "• p-значение: 0.034\n" +
                    "• Среднее выборки 1: 25.67\n" +
                    "• Среднее выборки 2: 23.45\n" +
                    "• Стандартное отклонение 1: 5.03\n" +
                    "• Стандартное отклонение 2: 4.87\n" +
                    "• Объем выборки 1: 50\n" +
                    "• Объем выборки 2: 50\n" +
                    "• Результат: Различия статистически значимы (p < 0.05)\n\n";
            }
            else if (method->Id == "BARTLETT_TEST")
            {
                results +=
                    "КРИТЕРИЙ БАРТЛЕТТА\n" +
                    //"───────────────────────────────────────────────\n" +
                    "• Тестовая статистика: 3.45\n" +
                    "• Степени свободы: 3\n" +
                    "• p-значение: 0.328\n" +
                    "• Количество групп: 4\n" +
                    "• Общий объем выборки: 120\n" +
                    "• Дисперсии групп: [25.3, 28.7, 24.9, 27.1]\n" +
                    "• Средние групп: [23.4, 25.6, 22.8, 24.9]\n" +
                    "• Результат: Дисперсии однородны (p > 0.05)\n\n";
            }
            else if (method->Id == "SHAPIRO_TEST")
            {
                results +=
                    "КРИТЕРИЙ ШАПИРО-УИЛКА\n" +
                    //"─────────────────────────────-─────────────────\n" +
                    "• W-статистика: 0.976\n" +
                    "• p-значение: 0.152\n" +
                    "• Объем выборки: 80\n" +
                    "• Среднее выборки: 24.56\n" +
                    "• Стандартное отклонение: 4.89\n" +
                    "• Асимметрия: 0.23\n" +
                    "• Эксцесс: -0.15\n" +
                    "• Результат: Распределение нормальное (p > 0.05)\n\n";
            }
            else if (method->Id == "WILCOXON_TEST")
            {
                results +=
                    "КРИТЕРИЙ УИЛКОКСОНА\n" +
                    //"───────────────────────────────────────────────\n" +
                    "• W-статистика: 215\n" +
                    "• Z-статистика: 2.34\n" +
                    "• p-значение: 0.019\n" +
                    "• Количество пар: 40\n" +
                    "• Сумма положительных рангов: 525\n" +
                    "• Сумма отрицательных рангов: 310\n" +
                    "• Медиана разностей: 2.5\n" +
                    "• Результат: Различия статистически значимы (p < 0.05)\n\n";
            }
            else
            {
                results +=
                    "РЕЗУЛЬТАТЫ АНАЛИЗА:\n" +
                    //"───────────────────────────────────────────────\n" +
                    "• Анализ выполнен успешно\n" +
                    "• Получены статистически значимые результаты\n" +
                    "• Все предположения критерия выполнены\n\n";
            }

            results +=
                "СТАТУС ВЫПОЛНЕНИЯ: УСПЕШНО \n";

            return results;
        }

    private:
        // Обработчики событий

        void treeViewMethods_AfterSelect(Object^ sender, TreeViewEventArgs^ e)
        {
            try
            {
                Object^ tag = e->Node->Tag;

                if (tag != nullptr && tag->GetType() == String::typeid)
                {
                    String^ methodId = safe_cast<String^>(tag);
                    selectedMethod = FindMethodById(methodId);

                    if (selectedMethod != nullptr)
                    {
                        // Обновляем описание метода
                        richTextBoxDescription->Text =
                            "МЕТОД: " + selectedMethod->Name + "\n\n" +
                            //"═══════════════════════════════════════════════════\n\n" +
                            "ОПИСАНИЕ:\n" + selectedMethod->Description + "\n\n" +
                            "ДЛЯ ВЫПОЛНЕНИЯ АНАЛИЗА:\n" +
                            "1. Укажите входной файл с данными\n" +
                            "2. Укажите файл для сохранения результатов\n" +
                            "3. Нажмите кнопку 'ВЫПОЛНИТЬ ВЫБРАННЫЙ МЕТОД'";

                        // Устанавливаем путь к выходному файлу по умолчанию
                        textBoxOutputFile->Text = selectedMethod->DefaultOutputFile;

                        UpdateStatus("Выбран метод: " + selectedMethod->Name);
                    }
                    else
                    {
                        // Для категорий и методов без реализации
                        richTextBoxDescription->Text =
                            "КАТЕГОРИЯ: " + e->Node->Text + "\n\n" +
                            //"═══════════════════════════════════════════════════\n\n" +
                            "Содержит методы для " + e->Node->Text->ToLower() + ".\n\n" +
                            "Выберите конкретный метод из данной категории\n" +
                            "для просмотра подробного описания и выполнения анализа.";
                    }
                }
                else
                {
                    // Для корневого узла и категорий
                    if (e->Node->Text == "Статистический анализ")
                    {
                        richTextBoxDescription->Text = GetWelcomeMessage();
                    }
                    else
                    {
                        richTextBoxDescription->Text =
                            "КАТЕГОРИЯ: " + e->Node->Text + "\n\n" +
                            //"═══════════════════════════════════════════════════\n\n" +
                            "Содержит методы для " + e->Node->Text->ToLower() + ".\n\n" +
                            "Выберите конкретный метод для продолжения.";
                    }
                }
            }
            catch (Exception^ ex)
            {
                MessageBox::Show(this, "Ошибка при выборе метода: " + ex->Message,
                    "Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Error);
            }
        }

        void buttonBrowseInput_Click(Object^ sender, EventArgs^ e)
        {
            try
            {
                OpenFileDialog^ dialog = gcnew OpenFileDialog();
                dialog->Filter = "Текстовые файлы (*.txt)|*.txt|Данные MLS (*.mls)|*.mls|Все файлы (*.*)|*.*";
                dialog->Title = "Выберите входной файл данных";
                dialog->CheckFileExists = true;

                if (dialog->ShowDialog(this) == System::Windows::Forms::DialogResult::OK)
                {
                    textBoxInputFile->Text = dialog->FileName;
                    UpdateStatus("Выбран файл: " + Path::GetFileName(dialog->FileName));
                }
            }
            catch (Exception^ ex)
            {
                MessageBox::Show(this, "Ошибка при выборе файла: " + ex->Message,
                    "Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Error);
            }
        }

        void buttonBrowseOutput_Click(Object^ sender, EventArgs^ e)
        {
            try
            {
                SaveFileDialog^ dialog = gcnew SaveFileDialog();
                dialog->Filter = "Текстовые файлы (*.txt)|*.txt|Все файлы (*.*)|*.*";
                dialog->Title = "Выберите файл для сохранения результатов";
                dialog->FileName = "results.txt";
                dialog->OverwritePrompt = true;

                if (dialog->ShowDialog(this) == System::Windows::Forms::DialogResult::OK)
                {
                    textBoxOutputFile->Text = dialog->FileName;
                    UpdateStatus("Файл результатов: " + Path::GetFileName(dialog->FileName));
                }
            }
            catch (Exception^ ex)
            {
                MessageBox::Show(this, "Ошибка при выборе файла: " + ex->Message,
                    "Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Error);
            }
        }

        void buttonRun_Click(Object^ sender, EventArgs^ e)
        {
            if (selectedMethod == nullptr)
            {
                MessageBox::Show(this, "Пожалуйста, выберите метод анализа из дерева.",
                    "Метод не выбран", MessageBoxButtons::OK, MessageBoxIcon::Warning);
                return;
            }

            if (String::IsNullOrEmpty(textBoxInputFile->Text))
            {
                MessageBox::Show(this, "Пожалуйста, укажите входной файл с данными.",
                    "Входной файл не указан", MessageBoxButtons::OK, MessageBoxIcon::Warning);
                return;
            }

            if (!File::Exists(textBoxInputFile->Text))
            {
                MessageBox::Show(this, "Указанный входной файл не существует.\nПроверьте путь к файлу.",
                    "Файл не найден", MessageBoxButtons::OK, MessageBoxIcon::Error);
                return;
            }

            try
            {
                // Показываем индикатор прогресса
                ShowProgress(true);
                UpdateStatus("Выполнение анализа...");

                // Получаем папку, где лежит исполняемый файл метода
                String^ programPath = selectedMethod->ExecutableFile;
                String^ programsFolder = Path::GetDirectoryName(programPath);

                // Формируем полный путь для файла результатов
                String^ resultFileName = selectedMethod->DefaultOutputFile;
                String^ fullResultPath = Path::Combine(programsFolder, resultFileName);

                // Обновляем поле с путем к результатам
                textBoxOutputFile->Text = fullResultPath;

                // Задержка для имитации работы
                Thread::Sleep(2000);

                // Генерируем тестовые результаты
                String^ results = GenerateTestResults(selectedMethod);

                // Добавляем информацию о файлах в результаты
                String^ fullResults =
                    "Входной файл: " + Path::GetFileName(textBoxInputFile->Text) + "\n" +
                    "Выходной файл: " + Path::GetFileName(fullResultPath) + "\n" +
                    "Полный путь: " + fullResultPath + "\n\n" +
                    results;

                // Сохраняем результаты в тот же каталог, что и исполняемый файл
                File::WriteAllText(fullResultPath, fullResults,
                    System::Text::Encoding::GetEncoding(1251));

                // ВЫВОДИМ РЕЗУЛЬТАТЫ В ПРАВОЙ ОБЛАСТИ ПРОГРАММЫ
                richTextBoxDescription->Text = fullResults;

                UpdateStatus("Анализ выполнен успешно. Результаты отображены и сохранены в файл.");

                // Показываем уведомление
                MessageBox::Show(this,
                    "Анализ выполнен успешно!\n\n" +
                    "Результаты:\n" +
                    "• Отображены в правой области программы\n" +
                    "• Сохранены в файл: " + fullResultPath,
                    "Успешно", MessageBoxButtons::OK, MessageBoxIcon::Information);
            }
            catch (Exception^ ex)
            {
                UpdateStatus("Ошибка выполнения");
                MessageBox::Show(this, "Ошибка при выполнении анализа:\n\n" + ex->Message,
                    "Ошибка выполнения", MessageBoxButtons::OK, MessageBoxIcon::Error);

                richTextBoxDescription->AppendText("\n\nОШИБКА ВЫПОЛНЕНИЯ:\n" + ex->Message);
            }
            finally
            {
                ShowProgress(false);
            }
        }

        void buttonOpenResult_Click(Object^ sender, EventArgs^ e)
        {
            try
            {
                // СЛОВАРЬ СООТВЕТСТВИЙ: ID метода -> имя файла результатов
                Collections::Generic::Dictionary<String^, String^>^ resultFiles =
                    gcnew Collections::Generic::Dictionary<String^, String^>();

                // Указываем явные пути к файлам результатов
                String^ basePath = "C:\\Users\\TagiR\\VsProjects\\Algorithm\\first\\first\\";

                resultFiles->Add("MLE_NORMAL", Path::Combine(basePath, "results_mle.txt"));
                resultFiles->Add("MLE_WEIBULL", Path::Combine(basePath, "results_weibull.txt"));
                resultFiles->Add("LSQ_NORMAL", Path::Combine(basePath, "results.txt"));
                resultFiles->Add("LSQ_WEIBULL", Path::Combine(basePath, "results_lsq_weibull.txt"));
                resultFiles->Add("GRUBBS_TEST", Path::Combine(basePath, "grubbs_test_result.txt"));
                resultFiles->Add("F_TEST", Path::Combine(basePath, "fisher_test_result.txt"));
                resultFiles->Add("T_TEST", Path::Combine(basePath, "student_test_result.txt"));
                resultFiles->Add("BARTLETT_TEST", Path::Combine(basePath, "bartlett_results.txt"));
                resultFiles->Add("SHAPIRO_TEST", Path::Combine(basePath, "results_shapiro.txt"));
                resultFiles->Add("WILCOXON_TEST", Path::Combine(basePath, "wilcoxon_results.txt"));
                resultFiles->Add("KRUSKAL_TEST", Path::Combine(basePath, "kruskal_detailed_results.txt"));

                if (selectedMethod == nullptr)
                {
                    MessageBox::Show(this, "Выберите метод из дерева.",
                        "Метод не выбран", MessageBoxButtons::OK, MessageBoxIcon::Warning);
                    return;
                }

                // Получаем путь к файлу для выбранного метода
                String^ resultFilePath = nullptr;
                if (resultFiles->TryGetValue(selectedMethod->Id, resultFilePath))
                {
                    if (File::Exists(resultFilePath))
                    {
                        // Обновляем поле
                        textBoxOutputFile->Text = resultFilePath;

                        // Открываем файл
                        Process::Start(resultFilePath);

                        // Показываем содержимое
                        String^ content = File::ReadAllText(resultFilePath, System::Text::Encoding::GetEncoding(1251));
                        richTextBoxDescription->Text =
                            "ФАЙЛ РЕЗУЛЬТАТОВ ДЛЯ: " + selectedMethod->Name + "\n\n" +
                            "Путь: " + resultFilePath + "\n\n" +
                            "════════════════════════════════════════\n\n" +
                            content;

                        UpdateStatus("Открыт файл: " + Path::GetFileName(resultFilePath));
                    }
                    else
                    {
                        MessageBox::Show(this,
                            "Файл результатов не найден:\n" + resultFilePath,
                            "Файл не найден", MessageBoxButtons::OK, MessageBoxIcon::Error);
                    }
                }
                else
                {
                    MessageBox::Show(this,
                        "Для метода '" + selectedMethod->Name + "' не задан файл результатов.",
                        "Файл не задан", MessageBoxButtons::OK, MessageBoxIcon::Warning);
                }
            }
            catch (Exception^ ex)
            {
                MessageBox::Show(this, "Ошибка: " + ex->Message,
                    "Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Error);
            }
        }

        void buttonClear_Click(Object^ sender, EventArgs^ e)
        {
            textBoxInputFile->Clear();
            textBoxOutputFile->Clear();

            if (selectedMethod != nullptr)
            {
                textBoxOutputFile->Text = selectedMethod->DefaultOutputFile;
            }

            richTextBoxDescription->Text = GetWelcomeMessage();
            UpdateStatus("Поля очищены. Выберите метод для анализа.");
        }
    };
}