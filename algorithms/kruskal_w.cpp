#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <boost/math/distributions/chi_squared.hpp>
#include <boost/math/distributions/fisher_f.hpp>

using namespace std;
using namespace boost::math;


struct KruskalWallisConfig {
   vector<vector<double>> samples; // Вектор выборок 
   double alpha = 0.05;
   string output_filename = "kruskal_wallis_results.txt";
};

//Функция для чтения всех данных из файла
bool read_config_from_file(const string& filename, KruskalWallisConfig& config) {
   ifstream infile(filename);
   if (!infile.is_open()) {
       cout << "Ошибка: не удалось открыть файл " << filename << endl;
       return false;
   }
   
   config.samples.clear();
   config.alpha = 0.05;
   config.output_filename = "kruskal_wallis_results.txt";
   
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
       else if (line == "[SAMPLE]") {
           // Начало новой выборки
           config.samples.push_back(vector<double>());
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
           // Если выборок еще нет, создаем первую
           if (config.samples.empty()) {
               config.samples.push_back(vector<double>());
           }
           
           // Пробуем прочитать как значения выборки
           istringstream iss(line);
           double value;
           bool has_data = false;
           
           // Читаем все числа в строке
           while (iss >> value) {
               config.samples.back().push_back(value);
               has_data = true;
           }
           
           // Если не удалось прочитать как данные, пробуем как параметр
           if (!has_data) {
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
   
   // Удаляем пустые выборки
   config.samples.erase(
       remove_if(config.samples.begin(), config.samples.end(),
                 [](const vector<double>& sample) { return sample.empty(); }),
       config.samples.end()
   );
   
   if (config.samples.empty()) {
       cout << "Ошибка: файл не содержит данных" << endl;
       return false;
   }
   
   cout << "Прочитано " << config.samples.size() << " выборок из файла " << filename << endl;
   
   // Выводим информацию о выборках
   for (size_t i = 0; i < config.samples.size(); i++) {
       cout << "  Выборка " << i+1 << ": " << config.samples[i].size() << " наблюдений" << endl;
   }
   
   cout << "Уровень значимости alpha: " << config.alpha << endl;
   cout << "Выходной файл: " << config.output_filename << endl;
   
   return true;
}

//Функция для вычисления рангов 

vector<double> calculate_ranks(const vector<double>& all_values) {
   int n = all_values.size();
   vector<double> ranks(n, 0.0);
   
   // Создаем копию значений с индексами
   vector<pair<double, int>> indexed_values;
   for (int i = 0; i < n; i++) {
       indexed_values.push_back({all_values[i], i});
   }
   
   // Сортируем по значениям
   sort(indexed_values.begin(), indexed_values.end(),
        [](const pair<double, int>& a, const pair<double, int>& b) {
            return a.first < b.first;
        });
   
   // Присваиваем ранги с учетом связей
   int i = 0;
   while (i < n) {
       int j = i;
       // Находим группу одинаковых значений
       while (j < n && indexed_values[j].first == indexed_values[i].first) {
           j++;
       }
       
       // Вычисляем средний ранг для группы
       double avg_rank = (i + j + 1) / 2.0;
       
       // Присваиваем ранги всем элементам группы
       for (int k = i; k < j; k++) {
           ranks[indexed_values[k].second] = avg_rank;
       }
       
       i = j;
   }
   
   return ranks;
}

//Функция для вычисления статистики H критерия Краскела-Уоллиса
//samples вектор выборок
//H_stat статистика H (возвращается по ссылке)
//H1_stat скорректированная статистика H1 (возвращается по ссылке)
//true если вычисление успешно

bool calculate_kruskal_wallis_stat(const vector<vector<double>>& samples, 
                                 double& H_stat, double& H1_stat) {
   
   int k = samples.size(); // количество выборок
   
   if (k < 2) {
       cout << "Ошибка: необходимо хотя бы 2 выборки" << endl;
       return false;
   }
   
   // Проверка, что каждая выборка не пуста
   for (int i = 0; i < k; i++) {
       if (samples[i].empty()) {
           cout << "Ошибка: выборка " << i+1 << " пуста" << endl;
           return false;
       }
       if (samples[i].size() < 3) {
           cout << "Предупреждение: выборка " << i+1 << " имеет менее 3 наблюдений" << endl;
       }
   }
   
   // Объединяем все значения в один вектор
   vector<double> all_values;
   vector<int> sample_indices; // Индекс выборки для каждого значения
   
   for (int i = 0; i < k; i++) {
       for (double value : samples[i]) {
           all_values.push_back(value);
           sample_indices.push_back(i);
       }
   }
   
   int N = all_values.size(); // общее количество наблюдений
   
   // Вычисляем ранги для всех значений
   vector<double> ranks = calculate_ranks(all_values);
   
   // Вычисляем суммы рангов для каждой выборки (формула (3.36))
   vector<double> R(k, 0.0);
   vector<int> n_i(k, 0);
   
   for (int idx = 0; idx < N; idx++) {
       int sample_idx = sample_indices[idx];
       R[sample_idx] += ranks[idx];
       n_i[sample_idx]++;
   }
   
   // Вычисляем статистику H (формула (3.36))
   double sum_R2_n = 0.0;
   for (int i = 0; i < k; i++) {
       sum_R2_n += (R[i] * R[i]) / n_i[i];
   }
   
   H_stat = (12.0 / (N * (N + 1.0))) * sum_R2_n - 3.0 * (N + 1.0);
   
   // Вычисляем поправку H1 для связей (формула (3.37))
   // Сначала вычисляем количество связей
   int T = 0;
   vector<double> sorted_values = all_values;
   sort(sorted_values.begin(), sorted_values.end());
   
   int i = 0;
   while (i < N) {
       int j = i;
       while (j < N && sorted_values[j] == sorted_values[i]) {
           j++;
       }
       int tie_size = j - i;
       if (tie_size > 1) {
           T += (tie_size * tie_size * tie_size - tie_size);
       }
       i = j;
   }
   
   // Поправка для связей
   double correction = 1.0 - (static_cast<double>(T) / (N * N * N - N));
   
   // Корректируем статистику H
   if (correction > 0) {
       H_stat /= correction;
   }
   
   // Вычисляем H1_stat (формула (3.37))
   H1_stat = (H_stat / 2.0) * (1.0 + (N - k) / (N - 1.0 - H_stat));
   
   return true;
}

/**
* Функция для вычисления критического значения
*/
double calculate_critical_value(int k, int N, double alpha) {
   // Вычисляем по формуле (3.38)
   int df1 = k - 1;
   int df2 = N - k;
   
   // Квантиль F-распределения
   double F_quantile = quantile(fisher_f_distribution<double>(df1, df2), 1.0 - alpha);
   
   // Квантиль хи-квадрат распределения
   double chi2_quantile = quantile(chi_squared_distribution<double>(df1), 1.0 - alpha);
   
   // Критическое значение H_alpha (формула (3.38))
   double H_alpha = 0.5 * ((k - 1) * F_quantile + chi2_quantile);
   
   return H_alpha;
}

void write_results_to_file(const KruskalWallisConfig& config,
                         double H_stat,
                         double H1_stat,
                         double H_alpha,
                         bool hypothesis_accepted) {
   
   ofstream outfile(config.output_filename);
   if (!outfile.is_open()) {
       cout << "Ошибка: не удалось создать файл " << config.output_filename << endl;
       return;
   }
   
   outfile << fixed << setprecision(6);
   
   outfile << "=== РЕЗУЛЬТАТЫ КРИТЕРИЯ КРАСКЕЛА-УОЛЛИСА ===" << endl << endl;
   
   // Исходные данные
   outfile << "ИСХОДНЫЕ ДАННЫЕ:" << endl;
   outfile << "Входной файл: kruskal_wallis_input.txt" << endl;
   outfile << "Уровень значимости (alpha): " << config.alpha << endl;
   outfile << "Количество выборок (k): " << config.samples.size() << endl << endl;
   
   // Выводим данные по выборкам
   for (size_t i = 0; i < config.samples.size(); i++) {
       outfile << "Выборка " << i+1 << " (n" << i+1 << " = " << config.samples[i].size() << "):" << endl;
       outfile << "  Значения: ";
       
       for (size_t j = 0; j < config.samples[i].size(); j++) {
           outfile << config.samples[i][j];
           if (j < config.samples[i].size() - 1) {
               outfile << ", ";
           }
           if ((j + 1) % 5 == 0 && j < config.samples[i].size() - 1) {
               outfile << endl << "           ";
           }
       }
       outfile << endl << endl;
   }
   
   // Вычисляем общее количество наблюдений
   int N = 0;
   for (const auto& sample : config.samples) {
       N += sample.size();
   }
   outfile << "Общее количество наблюдений (N): " << N << endl << endl;
   
   // Результаты вычислений
   outfile << "РЕЗУЛЬТАТЫ ВЫЧИСЛЕНИЙ:" << endl;
   outfile << "Статистика H: " << H_stat << endl;
   outfile << "Скорректированная статистика H1: " << H1_stat << endl;
   outfile << "Критическое значение H_alpha: " << H_alpha << endl;
   outfile << "Уровень значимости (alpha): " << config.alpha << endl << endl;
   
   // Вывод о гипотезе
   outfile << "ВЫВОД:" << endl;
   outfile << "Нулевая гипотеза H0: theta_1 = theta_2 = ... = theta_k (все выборки имеют одинаковые медианы)" << endl;
   outfile << "Альтернативная гипотеза H1: не все θ равны" << endl << endl;
   
   if (hypothesis_accepted) {
       outfile << "Нулевая гипотеза ПРИНЯТА." << endl;
       outfile << "(H1 = " << H1_stat << " <= " << H_alpha << ")" << endl;
       outfile << "Нет статистически значимых различий между медианами выборок." << endl;
   } else {
       outfile << "Нулевая гипотеза ОТВЕРГНУТА." << endl;
       outfile << "(H1 = " << H1_stat << " > " << H_alpha << ")" << endl;
       outfile << "Существуют статистически значимые различия между медианами выборок." << endl;
       outfile << "Рекомендуется провести попарное сравнение выборок." << endl;
   }
   
   outfile << endl << "==============================================" << endl;
   outfile << "Критерий Краскела-Уоллиса выполнен успешно." << endl;
   
   outfile.close();
   cout << "Результаты сохранены в файл: " << config.output_filename << endl;
}

bool kruskal_wallis_test(const KruskalWallisConfig& config) {
   
   // Проверка уровня значимости
   if (config.alpha <= 0 || config.alpha >= 1) {
       cout << "Ошибка: уровень значимости должен быть в диапазоне (0, 1)" << endl;
       return false;
   }
   
   // Вычисляем статистики
   double H_stat, H1_stat;
   if (!calculate_kruskal_wallis_stat(config.samples, H_stat, H1_stat)) {
       return false;
   }
   
   // Вычисляем общее количество наблюдений
   int N = 0;
   for (const auto& sample : config.samples) {
       N += sample.size();
   }
   
   int k = config.samples.size();
   
   // Вычисляем критическое значение
   double H_alpha = calculate_critical_value(k, N, config.alpha);
   
   // Проверка гипотезы
   bool hypothesis_accepted = (H1_stat <= H_alpha);
   
   // Вывод в консоль
   cout << fixed << setprecision(6);
   cout << "=== Результаты критерия Краскела-Уоллиса ===" << endl;
   cout << "Количество выборок (k): " << k << endl;
   cout << "Общее количество наблюдений (N): " << N << endl;
   cout << "Статистика H: " << H_stat << endl;
   cout << "Скорректированная статистика H1: " << H1_stat << endl;
   cout << "Критическое значение H_alpha: " << H_alpha << endl;
   cout << "Уровень значимости (alpha): " << config.alpha << endl;
   
   if (hypothesis_accepted) {
       cout << "Вывод: Нулевая гипотеза ПРИНЯТА (медианы выборок не различаются)" << endl;
   } else {
       cout << "Вывод: Нулевая гипотеза ОТВЕРГНУТА (медианы выборок различаются)" << endl;
   }
   
   // Запись в файл
   write_results_to_file(config, H_stat, H1_stat, H_alpha, hypothesis_accepted);
   
   return hypothesis_accepted;
}

void create_example_config_file() {
   ofstream outfile("example_kruskal_config.txt");
   if (!outfile.is_open()) {
       cout << "Ошибка: не удалось создать примерный файл" << endl;
       return;
   }
   
   outfile << "# Пример файла конфигурации для критерия Краскела-Уоллиса" << endl;
   outfile << "# Все параметры можно указывать в любом порядке" << endl;
   outfile << "# Пустые строки и строки, начинающиеся с #, игнорируются" << endl;
   outfile << endl;
   
   outfile << "# Параметры теста (необязательные, значения по умолчанию указаны)" << endl;
   outfile << "alpha 0.05           # уровень значимости" << endl;
   outfile << "output kruskal_results.txt   # имя выходного файла" << endl;
   outfile << endl;
   
   outfile << "# Данные: каждая строка содержит значения одной выборки" << endl;
   outfile << "# Можно указывать несколько значений в строке через пробел" << endl;
   outfile << "# Для новой выборки можно использовать метку [SAMPLE]" << endl;
   outfile << endl;
   
   outfile << "# Пример данных: 3 выборки" << endl;
   outfile << "# Выборка 1" << endl;
   outfile << "7.1 7.4 7.5 7.6 7.9" << endl;
   outfile << endl;
   
   outfile << "# Выборка 2" << endl;
   outfile << "6.8 7.0 7.2 7.3 7.4 7.6" << endl;
   outfile << endl;
   
   outfile << "# Выборка 3" << endl;
   outfile << "6.5 6.7 6.9 7.0 7.1 7.2 7.3" << endl;
   
   outfile.close();
   cout << "Создан примерный файл конфигурации: example_kruskal_config.txt" << endl;
}

int main() {
   setlocale(LC_ALL, "rus");
   // Имя входного файла
   string input_filename = "kruskal_wallis_input.txt";
   
   cout << "Программа для вычисления критерия Краскела-Уоллиса" << endl;
   cout << "==================================================" << endl;
   
   // Проверяем существование входного файла
   ifstream test_file(input_filename);
   if (!test_file.good()) {
       cout << "Файл " << input_filename << " не найден." << endl;
       cout << "Создайте файл " << input_filename << " в формате:" << endl;
       cout << "------------------------------------------" << endl;
       cout << "# Пример данных для критерия Краскела-Уоллиса" << endl;
       cout << "alpha 0.05" << endl;
       cout << "output kruskal_results.txt" << endl;
       cout << endl;
       cout << "# Выборка 1" << endl;
       cout << "7.1 7.4 7.5 7.6 7.9" << endl;
       cout << endl;
       cout << "# Выборка 2" << endl;
       cout << "6.8 7.0 7.2 7.3 7.4 7.6" << endl;
       cout << endl;
       cout << "# Выборка 3" << endl;
       cout << "6.5 6.7 6.9 7.0 7.1 7.2 7.3" << endl;
       cout << "------------------------------------------" << endl;
       
       // Создаем примерный файл для справки
       create_example_config_file();
       
       return 1;
   }
   test_file.close();
   
   // Чтение конфигурации из файла
   KruskalWallisConfig config;
   
   if (!read_config_from_file(input_filename, config)) {
       return 1;
   }
   
   // Выполнение критерия Краскела-Уоллиса
   cout << endl;
   kruskal_wallis_test(config);
   
   return 0;
}
