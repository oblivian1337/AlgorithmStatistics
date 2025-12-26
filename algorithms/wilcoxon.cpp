#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <sstream>

using namespace std;

//Вычисляет распределение статистики Манна-Уитни U (алгоритм AS62)

void udist(int M, int N, vector<double>& frequency, vector<double>& work, int& fault) {
   fault = 1;
   int minmn = min(M, N);
   if (minmn < 1) return;

   fault = 2;
   int mn1 = M * N + 1;
   if (frequency.size() < static_cast<size_t>(mn1)) return;

   int maxmn = max(M, N);
   int n1 = maxmn + 1;

   // Заполнение первых maxmn+1 элементов единицами
   for (int i = 0; i < n1; ++i) {
       frequency[i] = 1.0;
   }

   if (minmn == 1) {
       fault = 0;
       return;
   }

   fault = 3;
   if (work.size() < static_cast<size_t>((mn1 + 1) / 2 + minmn)) return;

   // Очистка остальной части frequency
   for (int i = n1; i < mn1; ++i) {
       frequency[i] = 0.0;
   }

   // Генерация распределений
   work[0] = 0.0;
   int in = maxmn;

   for (int i = 2; i <= minmn; ++i) {
       work[i - 1] = 0.0;
       in = in + maxmn;
       n1 = in + 2;
       int L = 1 + in / 2;
       int K = i;

       for (int j = 1; j <= L; ++j) {
           K = K + 1;
           n1 = n1 - 1;
           double sum = frequency[j - 1] + work[j - 1];
           frequency[j - 1] = sum;
           work[K - 1] = sum - frequency[n1 - 1];
           frequency[n1 - 1] = sum;
       }
   }

   fault = 0;
}

// Вычисляет точное p-значение для U-статистики используя AS62

double exact_p_value_from_udist(double U, int m, int n) {
   int mn1 = m * n + 1;
   int minmn = min(m, n);

   // Подготавливаем векторы как в AS62
   vector<double> frequency(mn1);
   vector<double> work((mn1 + 1) / 2 + minmn);

   int fault = 0;
   udist(m, n, frequency, work, fault);

   if (fault != 0) {
       throw runtime_error("Ошибка в udist: " + to_string(fault));
   }

   // Суммируем частоты для получения распределения
   double total = 0.0;
   for (int i = 0; i < mn1; i++) {
       total += frequency[i];
   }

   // Вычисляем точное p-значение
   double p_left = 0.0;
   double p_right = 0.0;

   int U_int = static_cast<int>(round(U));
   if (U_int < 0) U_int = 0;
   if (U_int >= mn1) U_int = mn1 - 1;

   // Левосторонний p-value: P(U ≤ U_obs)
   for (int i = 0; i <= U_int; i++) {
       p_left += frequency[i];
   }

   // Правосторонний p-value: P(U ≥ U_obs)
   for (int i = U_int; i < mn1; i++) {
       p_right += frequency[i];
   }

   // Нормализуем
   p_left /= total;
   p_right /= total;

   // Двусторонний p-value
   double p_tail = min(p_left, p_right);
   return 2.0 * p_tail;
}

// U-статистику Манна-Уитни для двух выборок
double compute_u_statistic(const vector<double>& sample1, const vector<double>& sample2) {
   double U = 0.0;

   for (size_t i = 0; i < sample1.size(); i++) {
       for (size_t j = 0; j < sample2.size(); j++) {
           if (sample1[i] > sample2[j]) {
               U += 1.0;
           }
           else if (sample1[i] == sample2[j]) {
               U += 0.5;
           }
       }
   }

   return U;
}

bool read_data_from_file(const string& filename,
   vector<double>& sample1,
   vector<double>& sample2) {
   ifstream infile(filename);
   if (!infile) {
       cerr << "Ошибка: не удалось открыть файл " << filename << endl;
       return false;
   }

   string line;
   int line_num = 0;

   while (getline(infile, line)) {
       line_num++;

       if (line.empty()) continue;

       // Удаляем начальные и конечные пробелы
       size_t start = line.find_first_not_of(" \t");
       if (start == string::npos) continue;

       size_t end = line.find_last_not_of(" \t");
       string trimmed_line = line.substr(start, end - start + 1);

       // Ищем разделитель из 5 пробелов
       string delimiter = "     ";  // 5 пробелов

       size_t delim_pos = trimmed_line.find(delimiter);
       if (delim_pos == string::npos) {
           for (int spaces = 2; spaces <= 10; spaces++) {
               delimiter = string(spaces, ' ');
               delim_pos = trimmed_line.find(delimiter);
               if (delim_pos != string::npos) {
                   break;
               }
           }

           if (delim_pos == string::npos) {
               cerr << "Ошибка в строке " << line_num << endl;
               cerr << "Ожидается формат: значение1" << string(5, ' ') << "значение2" << endl;
               return false;
           }
       }

       // Извлекаем значения из двух столбцов
       string value1_str = trimmed_line.substr(0, delim_pos);
       string value2_str = trimmed_line.substr(delim_pos + delimiter.length());

       // Убираем возможные пробелы вокруг чисел
       value1_str.erase(0, value1_str.find_first_not_of(" \t"));
       value1_str.erase(value1_str.find_last_not_of(" \t") + 1);

       value2_str.erase(0, value2_str.find_first_not_of(" \t"));
       value2_str.erase(value2_str.find_last_not_of(" \t") + 1);

       try {
           double value1 = stod(value1_str);
           double value2 = stod(value2_str);

           sample1.push_back(value1);
           sample2.push_back(value2);

       }
       catch (const exception& e) {
           cerr << "Ошибка преобразования чисел в строке " << line_num << ": " << line << endl;
           cerr << "  value1: '" << value1_str << "', value2: '" << value2_str << "'" << endl;
           return false;
       }
   }

   infile.close();

   if (sample1.empty() || sample2.empty()) {
       cerr << "Ошибка: одна или обе выборки пусты" << endl;
       return false;
   }

   return true;
}

int main() {
   setlocale(LC_ALL, "rus");
   cout << "Двухвыборочный критерий Уилкоксона-Манна-Уитни (алгоритм AS62)" << endl;
   cout << "==============================================================" << endl;

   // Чтение данных из файла
   vector<double> sample1, sample2;
   string filename = "wilcoxon_input.txt";

   if (!read_data_from_file(filename, sample1, sample2)) {
       cerr << "\nТребуемый формат файла " << filename << ":" << endl;
       cerr << "1.0" << string(5, ' ') << "2.5" << endl;
       cerr << "1.5" << string(5, ' ') << "3.0" << endl;
       cerr << "2.0" << string(5, ' ') << "3.5" << endl;
       cerr << "2.5" << string(5, ' ') << "4.0" << endl;
       cerr << "3.0" << string(5, ' ') << "4.5" << endl;
       cerr << "\nГде:" << endl;
       cerr << "- Первый столбец: значения первой выборки" << endl;
       cerr << "- Второй столбец: значения второй выборки" << endl;
       cerr << "- Значения разделены пятью пробелами" << endl;
       cerr << "- Количество строк определяет размер выборок" << endl;
       return 1;
   }

   cout << "\nДанные успешно загружены:" << endl;
   cout << "Размер выборки 1: " << sample1.size() << endl;
   cout << "Размер выборки 2: " << sample2.size() << endl;
   cout << "Произведение размеров (m x n): " << sample1.size() * sample2.size() << endl;

   // Проверка на слишком большие выборки
   int m = sample1.size();
   int n = sample2.size();
   int mn_product = m * n;

   if (mn_product > 10000) {
       cout << "\nВНИМАНИЕ: Произведение размеров выборок велико (" << mn_product << ")" << endl;
       cout << "Алгоритм AS62 может работать медленно или требовать много памяти." << endl;
       cout << "Рекомендуется использовать выборки меньшего размера." << endl;
   }

   // Вычисление U-статистики
   cout << "\nВычисление U-статистики..." << endl;
   double U_stat = compute_u_statistic(sample1, sample2);
   cout << "U-статистика: " << U_stat << endl;

   // Вычисление точного p-значения через AS62
   cout << "\nВычисление точного p-значения (алгоритм AS62)..." << endl;
   try {
       double p_value = exact_p_value_from_udist(U_stat, m, n);
       cout << "Точное p-значение: " << p_value << endl;

       // Статистический вывод
       double alpha = 0.05;
       cout << "\nСТАТИСТИЧЕСКИЙ ВЫВОД (alpha = " << alpha << "):" << endl;
       cout << "Нулевая гипотеза H0: распределения одинаковы" << endl;
       cout << "Альтернативная гипотеза H1: распределения различаются" << endl;

       if (p_value < alpha) {
           cout << "ОТКЛОНИТЬ H0: существует статистически значимое различие" << endl;
           cout << "(p = " << p_value << " < " << alpha << ")" << endl;
       }
       else {
           cout << "НЕ ОТВЕРГАТЬ H0: статистически значимого различия не обнаружено" << endl;
           cout << "(p = " << p_value << " >= " << alpha << ")" << endl;
       }

       // Сохранение результатов в файл
       ofstream outfile("wilcoxon_output.txt");
       if (!outfile) {
           cerr << "Ошибка: не удалось создать файл результатов" << endl;
           return 1;
       }

       outfile << "РЕЗУЛЬТАТЫ КРИТЕРИЯ УИЛКОКСОНА-МАННА-УИТНИ" << endl;
       outfile << "Использован алгоритм AS62" << endl;
       outfile << "===========================================" << endl << endl;

       outfile << "ИСХОДНЫЕ ДАННЫЕ:" << endl;
       outfile << "Файл: " << filename << endl;
       outfile << "Размер выборки 1: " << m << endl;
       outfile << "Размер выборки 2: " << n << endl;
       outfile << "m x n = " << mn_product << endl << endl;

       outfile << "РАСЧЕТНЫЕ ВЕЛИЧИНЫ:" << endl;
       outfile << "U-статистика Манна-Уитни: " << U_stat << endl;
       outfile << "Ожидаемое значение U при H0: " << (mn_product / 2.0) << endl;
       outfile << "Точное p-значение (двустороннее): " << p_value << endl << endl;

       outfile << "СТАТИСТИЧЕСКИЙ ВЫВОД:" << endl;
       outfile << "Уровень значимости alpha = " << alpha << endl;
       outfile << "Нулевая гипотеза H0: выборки из одинаковых распределений" << endl;
       outfile << "Альтернативная гипотеза H1: выборки из разных распределений" << endl << endl;

       if (p_value < alpha) {
           outfile << "РЕЗУЛЬТАТ: ОТКЛОНИТЬ H0" << endl;
           outfile << "Существует статистически значимое различие между выборками." << endl;
           outfile << "Вероятность наблюсти такие или более крайние различия" << endl;
           outfile << "при условии истинности H0 составляет " << p_value << "." << endl;
       }
       else {
           outfile << "РЕЗУЛЬТАТ: НЕ ОТВЕРГАТЬ H0" << endl;
           outfile << "Статистически значимого различия не обнаружено." << endl;
           outfile << "Нет оснований утверждать, что выборки происходят" << endl;
           outfile << "из разных распределений." << endl;
       }

       outfile.close();
       cout << "\nРезультаты сохранены в файл: wilcoxon_output.txt" << endl;

   }
   catch (const exception& e) {
       cerr << "\nОшибка при вычислении p-значения: " << e.what() << endl;
       cerr << "Возможно, размеры выборок слишком велики для алгоритма AS62." << endl;
       return 1;
   }

   return 0;
}
