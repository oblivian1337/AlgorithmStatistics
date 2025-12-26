#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <boost/math/special_functions/erf.hpp>

using namespace std;

struct ShapiroWilkConfig {
   vector<double> data;
   double alpha = 0.05;
   string output_filename = "shapiro_wilk_results.txt";
};

vector<double> get_shapiro_wilk_coefficients(int n) {
   // используем приближенные коэффициенты


   static const vector<vector<double>> coefficients = {
       {}, // n=0
       {}, // n=1
       {}, // n=2
       {0.7071}, // n=3: a1
       {0.6872, 0.1677}, // n=4: a1, a2
       {0.6646, 0.2413}, // n=5
       {0.6431, 0.2806, 0.0875}, // n=6
       {0.6233, 0.3031, 0.1401}, // n=7
       {0.6052, 0.3164, 0.1743, 0.0561}, // n=8
       {0.5888, 0.3244, 0.1976, 0.0947}, // n=9
       {0.5739, 0.3291, 0.2141, 0.1224}, // n=10
       // Для упрощения ограничимся n<=10, для реального применения
       // нужно использовать полную таблицу коэффициентов
   };

   if (n < 3 || n > 50) {
       return {};
   }

   if (n < static_cast<int>(coefficients.size())) {
       return coefficients[n];
   }

   // Для n > 10 используем приближение
   vector<double> coeffs;
   int k = n / 2;
   for (int i = 1; i <= k; i++) {
       double x = static_cast<double>(i) / (n + 1);
       double z = sqrt(-2.0 * log(x));
       double a_i = z / sqrt(n);
       coeffs.push_back(a_i);
   }

   return coeffs;
}


//Критические значения W для критерия Шапиро-Уилка
// Приближенные значения для alpha = 0.05

double get_critical_W_value(int n, double alpha = 0.05) {
   // Приближенные критические значения для alpha = 0.05
   static const vector<pair<int, double>> critical_values_005 = {
       {3, 0.767}, {4, 0.748}, {5, 0.762}, {6, 0.788}, {7, 0.803},
       {8, 0.818}, {9, 0.829}, {10, 0.842}, {11, 0.850}, {12, 0.859},
       {13, 0.866}, {14, 0.874}, {15, 0.881}, {16, 0.887}, {17, 0.892},
       {18, 0.897}, {19, 0.901}, {20, 0.905}, {25, 0.918}, {30, 0.927},
       {35, 0.934}, {40, 0.940}, {45, 0.945}, {50, 0.947}
   };

   for (const auto& cv : critical_values_005) {
       if (n == cv.first) {
           return cv.second;
       }
   }

   // Линейная интерполяция для промежуточных значений
   for (size_t i = 0; i < critical_values_005.size() - 1; i++) {
       if (n > critical_values_005[i].first && n <= critical_values_005[i + 1].first) {
           double x1 = critical_values_005[i].first;
           double y1 = critical_values_005[i].second;
           double x2 = critical_values_005[i + 1].first;
           double y2 = critical_values_005[i + 1].second;
           return y1 + (y2 - y1) * (n - x1) / (x2 - x1);
       }
   }

   // Экстраполяция для n > 50
   if (n > 50) {
       // Приближение для больших n
       return 0.947 + (0.002 * (50 - n) / 50.0);
   }

   return 0.0;
}

/**
* Функция для чтения всех данных из файла
*/
bool read_config_from_file(const string& filename, ShapiroWilkConfig& config) {
   ifstream infile(filename);
   if (!infile.is_open()) {
       cout << "Ошибка: не удалось открыть файл " << filename << endl;
       return false;
   }

   config.data.clear();
   config.alpha = 0.05;
   config.output_filename = "shapiro_wilk_results.txt";

   string line;
   int line_num = 0;

   while (getline(infile, line)) {
       line_num++;

       // Удаление начальных и конечных пробелов
       line.erase(0, line.find_first_not_of(" \t"));
       line.erase(line.find_last_not_of(" \t") + 1);

       // Пропускаем пустые строки
       if (line.empty()) {
           continue;
       }

       // Пропускаем комментарии
       if (line[0] == '#') {
           continue;
       }

       // Проверяем секции файла
       if (line == "[PARAMETERS]") {
           // Пропускаем, параметры могут быть в любом месте
           continue;
       }
       else if (line == "[DATA]") {
           // Пропускаем
           continue;
       }

       // Чтение строки
       istringstream iss(line);
       string first_word;

       if (iss >> first_word) {
           // Проверяем, является ли первое слово параметром
           if (first_word == "alpha" || first_word == "ALPHA") {
               double alpha_value;
               if (iss >> alpha_value) {
                   config.alpha = alpha_value;
               }
               continue;
           }
           else if (first_word == "output" || first_word == "OUTPUT") {
               string filename_value;
               if (iss >> filename_value) {
                   config.output_filename = filename_value;
               }
               continue;
           }

           // Если не параметр, то это данные
           // Возвращаемся к началу строки
           iss.clear();
           iss.str(line);

           double value;
           while (iss >> value) {
               config.data.push_back(value);
           }
       }
   }

   infile.close();

   if (config.data.empty()) {
       cout << "Ошибка: файл не содержит данных" << endl;
       return false;
   }

   if (config.alpha <= 0 || config.alpha >= 1) {
       cout << "Предупреждение: некорректный уровень значимости. Используется 0.05." << endl;
       config.alpha = 0.05;
   }

   cout << "Прочитано " << config.data.size() << " значений из файла " << filename << endl;
   cout << "Уровень значимости alpha: " << config.alpha << endl;
   cout << "Выходной файл: " << config.output_filename << endl;

   return true;
}


//Функция для записи результатов в файл

void write_results_to_file(const ShapiroWilkConfig& config,
   const vector<double>& sorted_data,
   double W_statistic,
   double W_critical,
   bool hypothesis_accepted) {

   ofstream outfile(config.output_filename);
   if (!outfile.is_open()) {
       cout << "Ошибка: не удалось создать файл " << config.output_filename << endl;
       return;
   }

   outfile << fixed << setprecision(6);

   outfile << "=== РЕЗУЛЬТАТЫ КРИТЕРИЯ ШАПИРО-УИЛКА ===" << endl << endl;

   // Исходные данные
   outfile << "ИСХОДНЫЕ ДАННЫЕ:" << endl;
   outfile << "Входной файл: shapiro_wilk_input.txt" << endl;
   outfile << "Уровень значимости (alpha): " << config.alpha << endl;
   outfile << "Объем выборки (n): " << config.data.size() << endl << endl;

   outfile << "Исходные значения:" << endl;
   outfile << setw(10) << "номер" << setw(15) << "Значение" << endl;
   outfile << string(25, '-') << endl;

   for (size_t i = 0; i < config.data.size(); i++) {
       outfile << setw(10) << i + 1 << setw(15) << config.data[i] << endl;
   }
   outfile << endl;

   // Отсортированные данные
   outfile << "Отсортированные значения:" << endl;
   outfile << setw(10) << "номер" << setw(15) << "Значение" << endl;
   outfile << string(25, '-') << endl;

   for (size_t i = 0; i < sorted_data.size(); i++) {
       outfile << setw(10) << i + 1 << setw(15) << sorted_data[i] << endl;
   }
   outfile << endl;

   // Результаты вычислений
   outfile << "РЕЗУЛЬТАТЫ ВЫЧИСЛЕНИЙ:" << endl;
   outfile << "Объем выборки (n): " << config.data.size() << endl;
   outfile << "Статистика W: " << W_statistic << endl;
   outfile << "Критическое значение W: " << W_critical << endl << endl;

   // Вывод о гипотезе
   outfile << "ГИПОТЕЗА:" << endl;
   outfile << "H0: Выборка происходит из нормально распределенной генеральной совокупности" << endl;
   outfile << "H1: Выборка не происходит из нормально распределенной генеральной совокупности" << endl << endl;

   outfile << "ВЫВОД:" << endl;
   if (hypothesis_accepted) {
       outfile << "Нулевая гипотеза о нормальности распределения ПРИНЯТА." << endl;
       outfile << "(W = " << W_statistic << " >= " << W_critical << ")" << endl;
       outfile << "Выборка может считаться происходящей из нормального распределения." << endl;
   }
   else {
       outfile << "Нулевая гипотеза о нормальности распределения ОТВЕРГНУТА." << endl;
       outfile << "(W = " << W_statistic << " < " << W_critical << ")" << endl;
       outfile << "Выборка не происходит из нормального распределения." << endl;
   }

   // Дополнительная информация
   outfile << endl << "ДОПОЛНИТЕЛЬНАЯ ИНФОРМАЦИЯ:" << endl;
   outfile << "- Критерий Шапиро-Уилка наиболее мощный для n ≤ 50" << endl;
   outfile << "- Для n > 50 рекомендуется использовать другие критерии" << endl;
   outfile << "- Критерий чувствителен к отклонениям в хвостах распределения" << endl;

   outfile << endl << "======================================" << endl;
   outfile << "Критерий Шапиро-Уилка выполнен успешно." << endl;

   outfile.close();
   cout << "Результаты сохранены в файл: " << config.output_filename << endl;
}


// Функция для вычисления статистики Шапиро-Уилка
double calculate_shapiro_wilk_statistic(const vector<double>& data) {
   int n = data.size();

   // Сортируем данные
   vector<double> sorted_data = data;
   sort(sorted_data.begin(), sorted_data.end());

   // Получаем коэффициенты
   vector<double> a = get_shapiro_wilk_coefficients(n);
   if (a.empty() && n >= 3) {
       // Если коэффициенты не найдены, используем приближенную формулу
       int k = n / 2;
       for (int i = 1; i <= k; i++) {
           double x = static_cast<double>(i) / (n + 1);
           double z = sqrt(-2.0 * log(x));
           a.push_back(z / sqrt(n));
       }
   }

   // Вычисляем b (формула 3.19 в пособии)
   double b = 0.0;
   int k = n / 2;

   for (int i = 0; i < k; i++) {
       double a_i = (i < static_cast<int>(a.size())) ? a[i] : 0.0;
       b += a_i * (sorted_data[n - 1 - i] - sorted_data[i]);
   }

   // Вычисляем s² (формула 3.18)
   double mean = accumulate(sorted_data.begin(), sorted_data.end(), 0.0) / n;
   double s2 = 0.0;
   for (double x : sorted_data) {
       s2 += (x - mean) * (x - mean);
   }

   // Вычисляем W (формула 3.17)
   if (s2 == 0.0) {
       return 1.0; // Все значения одинаковы
   }

   double W = (b * b) / s2;
   return W;
}

// Основная функция для выполнения критерия Шапиро-Уилка

bool shapiro_wilk_test(const ShapiroWilkConfig& config) {
   int n = config.data.size();

   if (n < 3) {
       cout << "Ошибка: для критерия Шапиро-Уилка необходимо минимум 3 наблюдения" << endl;
       return false;
   }

   if (n > 5000) {
       cout << "Предупреждение: критерий Шапиро-Уилка рекомендуется для n ≤ 50" << endl;
   }

   // Вычисляем статистику W
   double W_statistic = calculate_shapiro_wilk_statistic(config.data);

   // Получаем критическое значение
   double W_critical = get_critical_W_value(n, config.alpha);

   // Проверяем гипотезу
   bool hypothesis_accepted = (W_statistic >= W_critical);

   // Сортируем данные для вывода
   vector<double> sorted_data = config.data;
   sort(sorted_data.begin(), sorted_data.end());

   // Вывод в консоль
   cout << fixed << setprecision(6);
   cout << "=== Результаты критерия Шапиро-Уилка ===" << endl;
   cout << "Объем выборки (n): " << n << endl;
   cout << "Статистика W: " << W_statistic << endl;
   cout << "Критическое значение W: " << W_critical << endl;
   cout << "Уровень значимости (alpha): " << config.alpha << endl;

   if (hypothesis_accepted) {
       cout << "Вывод: Нулевая гипотеза о нормальности ПРИНИМАЕТСЯ" << endl;
   }
   else {
       cout << "Вывод: Нулевая гипотеза о нормальности ОТВЕРГАЕТСЯ" << endl;
   }

   // Запись в файл
   write_results_to_file(config, sorted_data, W_statistic,
       W_critical, hypothesis_accepted);

   return hypothesis_accepted;
}


//Функция для создания файла с примером формата данных

void create_example_config_file() {
   ofstream outfile("example_shapiro_wilk.txt");
   if (!outfile.is_open()) {
       cout << "Ошибка: не удалось создать примерный файл" << endl;
       return;
   }

   outfile << "# Пример файла данных для критерия Шапиро-Уилка" << endl;
   outfile << "# Проверка нормальности распределения" << endl;
   outfile << endl;

   outfile << "# Параметры теста" << endl;
   outfile << "alpha 0.05" << endl;
   outfile << "output shapiro_wilk_results.txt" << endl;
   outfile << endl;

   outfile << "# Данные для теста (пример нормально распределенных данных)" << endl;
   outfile << "# Среднее = 100, стандартное отклонение = 15" << endl;
   outfile << "85.2" << endl;
   outfile << "92.7" << endl;
   outfile << "97.3" << endl;
   outfile << "101.8" << endl;
   outfile << "103.5" << endl;
   outfile << "105.9" << endl;
   outfile << "109.2" << endl;
   outfile << "112.4" << endl;
   outfile << "115.7" << endl;
   outfile << "118.3" << endl;

   outfile.close();
   cout << "Создан примерный файл конфигурации: example_shapiro_wilk.txt" << endl;
}

int main() {
   setlocale(LC_ALL, "rus");
   // Имя входного файла
   string input_filename = "shapiro_wilk_input.txt";

   cout << "Программа для вычисления критерия Шапиро-Уилка" << endl;
   cout << "==============================================" << endl;

   // Проверяем существование входного файла
   ifstream test_file(input_filename);
   if (!test_file.good()) {
       cout << "Файл " << input_filename << " не найден." << endl;
       cout << "Создайте файл " << input_filename << " в формате:" << endl;
       cout << "------------------------------------------" << endl;
       cout << "# Проверка нормальности распределения" << endl;
       cout << "alpha 0.05" << endl;
       cout << "output shapiro_wilk_results.txt" << endl;
       cout << endl;
       cout << "# Данные (по одному значению на строку)" << endl;
       cout << "85.2" << endl;
       cout << "92.7" << endl;
       cout << "97.3" << endl;
       cout << "101.8" << endl;
       cout << "103.5" << endl;
       cout << "105.9" << endl;
       cout << "109.2" << endl;
       cout << "112.4" << endl;
       cout << "115.7" << endl;
       cout << "118.3" << endl;
       cout << "------------------------------------------" << endl;

       // Создаем примерный файл для справки
       create_example_config_file();

       return 1;
   }
   test_file.close();

   // Чтение конфигурации из файла
   ShapiroWilkConfig config;

   if (!read_config_from_file(input_filename, config)) {
       return 1;
   }

   // Выполнение критерия Шапиро-Уилка
   cout << endl;
   shapiro_wilk_test(config);

   return 0;
}
