#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <string>
#include <sstream>
#include <iomanip>
#include <boost/math/distributions/chi_squared.hpp>

using namespace std;
using namespace boost::math;


// Структура для хранения конфигурации теста

struct BartlettConfig {
   vector<double> variances;
   vector<int> sizes;
   double alpha = 0.05;
   string output_filename = "bartlett_results.txt";
};

// Функция для чтения всех данных из файла
bool read_config_from_file(const string& filename, BartlettConfig& config) {
   ifstream infile(filename);
   if (!infile.is_open()) {
       cout << "Ошибка: не удалось открыть файл " << filename << endl;
       return false;
   }

   config.variances.clear();
   config.sizes.clear();
   config.alpha = 0.05; // значение по умолчанию
   config.output_filename = "bartlett_results.txt"; // значение по умолчанию

   string line;
   int line_num = 0;
   bool reading_data = false;
   bool reading_params = false;

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
           reading_params = true;
           reading_data = false;
           continue;
       }
       else if (line == "[DATA]") {
           reading_data = true;
           reading_params = false;
           continue;
       }

       // Чтение параметров
       if (reading_params) {
           istringstream iss(line);
           string key;
           if (iss >> key) {
               if (key == "alpha" || key == "ALPHA") {
                   if (!(iss >> config.alpha)) {
                       cout << "Предупреждение: некорректное значение alpha в строке " << line_num << endl;
                   }
               }
               else if (key == "output" || key == "OUTPUT") {
                   string filename;
                   if (iss >> filename) {
                       config.output_filename = filename;
                   }
               }
           }
       }

       // Чтение данных
       else if (reading_data || (!reading_params && !reading_data)) {
           // По умолчанию читаем как данные, если нет явных секций
           istringstream iss(line);
           double variance;
           int size;

           if (iss >> variance >> size) {
               config.variances.push_back(variance);
               config.sizes.push_back(size);
           }
           else {
               // Пробуем прочитать как параметр, если не удалось как данные
               iss.clear();
               iss.str(line);
               string key;
               if (iss >> key) {
                   if (key == "alpha" || key == "ALPHA") {
                       if (!(iss >> config.alpha)) {
                           cout << "Предупреждение: некорректное значение alpha в строке " << line_num << endl;
                       }
                   }
                   else if (key == "output" || key == "OUTPUT") {
                       string filename;
                       if (iss >> filename) {
                           config.output_filename = filename;
                       }
                   }
               }
           }
       }
   }

   infile.close();

   if (config.variances.empty()) {
       cout << "Ошибка: файл не содержит данных о дисперсиях" << endl;
       return false;
   }

   cout << "Прочитано " << config.variances.size() << " выборок из файла " << filename << endl;
   cout << "Уровень значимости alpha: " << config.alpha << endl;
   cout << "Выходной файл: " << config.output_filename << endl;

   return true;
}


void write_results_to_file(const BartlettConfig& config,
   double s2_pooled,
   double c,
   double chi2_stat,
   int df,
   double chi2_critical,
   bool hypothesis_accepted) {

   ofstream outfile(config.output_filename);
   if (!outfile.is_open()) {
       cout << "Ошибка: не удалось создать файл " << config.output_filename << endl;
       return;
   }

   outfile << fixed << setprecision(6);

   outfile << "=== РЕЗУЛЬТАТЫ КРИТЕРИЯ БАРТЛЕТА ===" << endl << endl;

   // Исходные данные
   outfile << "ИСХОДНЫЕ ДАННЫЕ:" << endl;
   outfile << "Входной файл: bartlett_input.txt" << endl;
   outfile << "Уровень значимости (alpha): " << config.alpha << endl << endl;

   outfile << setw(5) << "№" << setw(15) << "Дисперсия" << setw(10) << "Объем" << endl;
   outfile << string(35, '-') << endl;

   for (size_t i = 0; i < config.variances.size(); i++) {
       outfile << setw(5) << i + 1
           << setw(15) << config.variances[i]
           << setw(10) << config.sizes[i] << endl;
   }
   outfile << endl;

   // Результаты вычислений
   outfile << "РЕЗУЛЬТАТЫ ВЫЧИСЛЕНИЙ:" << endl;
   outfile << "Количество выборок (m): " << config.variances.size() << endl;
   outfile << "Объединенная дисперсия (s^2): " << s2_pooled << endl;
   outfile << "Коэффициент c: " << c << endl;
   outfile << "Статистика хи-квадрат: " << chi2_stat << endl;
   outfile << "Степени свободы (df): " << df << endl;
   outfile << "Критическое значение хи-квадрат: " << chi2_critical << endl << endl;

   // Вывод о гипотезе
   outfile << "ВЫВОД:" << endl;
   if (hypothesis_accepted) {
       outfile << "Нулевая гипотеза об однородности дисперсий ПРИНЯТА." << endl;
       outfile << "(χ² = " << chi2_stat << " <= " << chi2_critical << ")" << endl;
       outfile << "Все дисперсии можно считать статистически одинаковыми." << endl;
   }
   else {
       outfile << "Нулевая гипотеза об однородности дисперсий ОТВЕРГНУТА." << endl;
       outfile << "(χ² = " << chi2_stat << " > " << chi2_critical << ")" << endl;
       outfile << "Дисперсии статистически различаются." << endl;
   }

   outfile << endl << "======================================" << endl;
   outfile << "Критерий Бартлета выполнен успешно." << endl;

   outfile.close();
   cout << "Результаты сохранены в файл: " << config.output_filename << endl;
}


bool bartlett_test(const BartlettConfig& config) {

   int m = config.variances.size();

   if (m <= 1) {
       cout << "Ошибка: необходимо хотя бы 2 выборки" << endl;
       return false;
   }

   if (config.variances.size() != config.sizes.size()) {
       cout << "Ошибка: размеры векторов variances и sizes должны совпадать" << endl;
       return false;
   }

   // Проверка данных
   for (int i = 0; i < m; i++) {
       if (config.variances[i] <= 0) {
           cout << "Ошибка: дисперсия должна быть положительной" << endl;
           return false;
       }
       if (config.sizes[i] <= 1) {
           cout << "Ошибка: объем выборки должен быть больше 1" << endl;
           return false;
       }
   }

   // Проверка уровня значимости
   if (config.alpha <= 0 || config.alpha >= 1) {
       cout << "Ошибка: уровень значимости должен быть в диапазоне (0, 1)" << endl;
       return false;
   }

   // Вычисление сумм
   double sum_log_s2 = 0.0;
   double sum_fi_s2 = 0.0;
   double sum_fi = 0.0;
   double sum_inv_fi = 0.0;

   for (int i = 0; i < m; i++) {
       int fi = config.sizes[i] - 1;
       double s2_i = config.variances[i];

       sum_log_s2 += fi * log(s2_i);
       sum_fi_s2 += fi * s2_i;
       sum_fi += fi;
       sum_inv_fi += 1.0 / fi;
   }

   // Вычисление по формулам (3.10)-(3.12)
   double s2_pooled = sum_fi_s2 / sum_fi;
   double c = 1.0 + (1.0 / (3.0 * (m - 1))) * (sum_inv_fi - 1.0 / sum_fi);
   double chi2_stat = (1.0 / c) * (log(s2_pooled) * sum_fi - sum_log_s2);

   int df = m - 1;
   double chi2_critical = quantile(chi_squared_distribution<double>(df), 1.0 - config.alpha);
   bool hypothesis_accepted = (chi2_stat <= chi2_critical);

   // Вывод в консоль
   cout << fixed << setprecision(6);
   cout << "=== Результаты критерия Бартлета ===" << endl;
   cout << "Количество выборок (m): " << m << endl;
   cout << "Объединенная дисперсия (s^2): " << s2_pooled << endl;
   cout << "Коэффициент c: " << c << endl;
   cout << "Статистика хи-квадрат: " << chi2_stat << endl;
   cout << "Степени свободы: " << df << endl;
   cout << "Уровень значимости (alpha): " << config.alpha << endl;
   cout << "Критическое значение хи-квадрат: " << chi2_critical << endl;

   if (hypothesis_accepted) {
       cout << "Вывод: Нулевая гипотеза об однородности дисперсий ПРИНИМАЕТСЯ" << endl;
   }
   else {
       cout << "Вывод: Нулевая гипотеза об однородности дисперсий ОТВЕРГАЕТСЯ" << endl;
   }

   // Запись в файл
   write_results_to_file(config, s2_pooled, c, chi2_stat, df,
       chi2_critical, hypothesis_accepted);

   return hypothesis_accepted;
}

/**
* Функция для создания файла с примером формата данных
*/
void create_example_config_file() {
   ofstream outfile("example_config.txt");
   if (!outfile.is_open()) {
       cout << "Ошибка: не удалось создать примерный файл" << endl;
       return;
   }

   outfile << "# Пример файла конфигурации для критерия Бартлета" << endl;
   outfile << "# Все параметры можно указывать в любом порядке" << endl;
   outfile << "# Пустые строки и строки, начинающиеся с #, игнорируются" << endl;
   outfile << endl;

   outfile << "# Параметры теста (необязательные, значения по умолчанию указаны)" << endl;
   outfile << "alpha 0.05           # уровень значимости (по умолчанию 0.05)" << endl;
   outfile << "output results.txt   # имя выходного файла (по умолчанию bartlett_results.txt)" << endl;
   outfile << endl;

   outfile << "# Данные: дисперсия и объем выборки (обязательные)" << endl;
   outfile << "# Каждая строка - отдельная выборка" << endl;
   outfile << "# Данные из контрольного вопроса 5 (квадраты СКО: 0.15, 0.17, 0.21, 0.25, 0.27)" << endl;
   outfile << endl;

   outfile << "0.0225 10" << endl;
   outfile << "0.0289 12" << endl;
   outfile << "0.0441 15" << endl;
   outfile << "0.0625 9" << endl;
   outfile << "0.0729 11" << endl;

   outfile.close();
   cout << "Создан примерный файл конфигурации: example_config.txt" << endl;
}

int main() {
   setlocale(LC_ALL, "rus");
   // Имя входного файла
   string input_filename = "bartlett_input.txt";

   cout << "Программа для вычисления критерия Бартлета" << endl;
   cout << "==========================================" << endl;

   // Проверяем существование входного файла
   ifstream test_file(input_filename);
   if (!test_file.good()) {
       cout << "Файл " << input_filename << " не найден." << endl;
       cout << "Создайте файл " << input_filename << " в формате:" << endl;
       cout << "------------------------------------------" << endl;
       cout << "# Дисперсии и объемы выборок" << endl;
       cout << "0.0225 10" << endl;
       cout << "0.0289 12" << endl;
       cout << "0.0441 15" << endl;
       cout << "0.0625 9" << endl;
       cout << "0.0729 11" << endl;
       cout << "# Уровень значимости (необязательно)" << endl;
       cout << "alpha 0.05" << endl;
       cout << "# Выходной файл (необязательно)" << endl;
       cout << "output bartlett_results.txt" << endl;
       cout << "------------------------------------------" << endl;

       // Создаем примерный файл для справки
       create_example_config_file();

       return 1;
   }
   test_file.close();

   // Чтение конфигурации из файла
   BartlettConfig config;

   if (!read_config_from_file(input_filename, config)) {
       return 1;
   }

   // Выполнение критерия Бартлета
   cout << endl;
   bartlett_test(config);

   return 0;
}
