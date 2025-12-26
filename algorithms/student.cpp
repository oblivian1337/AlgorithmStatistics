#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <string>
#include <iomanip>
#include <functional>
#include <numeric>
#include <sstream>

// Функции из Boost библиотеки для распределений
#include <boost/math/distributions/normal.hpp>
#include <boost/math/distributions/students_t.hpp>
#include <boost/math/distributions/chi_squared.hpp>
#include <boost/math/distributions/fisher_f.hpp>
#include <boost/math/distributions/non_central_t.hpp>
#include <boost/math/distributions/non_central_chi_squared.hpp>
#include <boost/math/distributions/non_central_f.hpp>
#include <boost/math/distributions/binomial.hpp>

using namespace std;
using namespace boost::math;

//############# Normal Distribution ############################
double norm_cdf(double x) {
   normal_distribution<> d(0, 1);
   return cdf(d, x);
}

double norm_ppf(double p) {
   if (p <= 0 || p >= 1) return 0;
   normal_distribution<> d(0, 1);
   return quantile(d, p);
}

double norm_pdf(double x) {
   normal_distribution<> d(0, 1);
   return pdf(d, x);
}

//############# Student Distribution ############################
double t_cdf(double x, double f) {
   students_t_distribution<> d(f);
   return cdf(d, x);
}

double t_ppf(double p, double f) {
   if (p <= 0 || p >= 1) return 0;
   students_t_distribution<> d(f);
   return quantile(d, p);
}

double t_pdf(double x, double f) {
   students_t_distribution<> d(f);
   return pdf(d, x);
}

//############# F-Distribution ############################
double f_cdf(double x, double f1, double f2) {
   fisher_f_distribution<> d(f1, f2);
   return cdf(d, x);
}

double f_ppf(double p, double f1, double f2) {
   if (p <= 0 || p >= 1) return 0;
   fisher_f_distribution<> d(f1, f2);
   return quantile(d, p);
}

// Функция для чтения данных из файла
vector<vector<double>> readDataFromFile(const string& filename) {
   vector<vector<double>> datasets;
   ifstream file(filename);

   if (!file.is_open()) {
       cerr << "Ошибка открытия файла: " << filename << endl;
       return datasets;
   }

   string line;
   while (getline(file, line)) {
       if (line.empty() || line[0] == '#') continue;

       vector<double> dataset;
       istringstream iss(line);
       double value;
       while (iss >> value) {
           dataset.push_back(value);
       }

       if (!dataset.empty()) {
           datasets.push_back(dataset);
       }
   }

   file.close();
   return datasets;
}

// Функция для вычисления выборочного среднего
double calculateMean(const vector<double>& data) {
   if (data.empty()) return 0.0;
   return accumulate(data.begin(), data.end(), 0.0) / data.size();
}

// Функция для вычисления выборочной дисперсии
double calculateVariance(const vector<double>& data, double mean) {
   if (data.size() <= 1) return 0.0;

   double sumSquares = 0.0;
   for (double value : data) {
       sumSquares += (value - mean) * (value - mean);
   }
   return sumSquares / (data.size() - 1);
}

// Функция для вычисления выборочного стандартного отклонения
double calculateStdDev(const vector<double>& data, double mean) {
   return sqrt(calculateVariance(data, mean));
}

// Функция для проверки равенства дисперсий (критерий Фишера)
bool checkEqualVariances(const vector<double>& data1, const vector<double>& data2,
   double alpha, ofstream& outputFile) {
   double var1 = calculateVariance(data1, calculateMean(data1));
   double var2 = calculateVariance(data2, calculateMean(data2));

   // Всегда помещаем большую дисперсию в числитель
   double F_statistic;
   int df1, df2;

   if (var1 >= var2) {
       F_statistic = var1 / var2;
       df1 = data1.size() - 1;
       df2 = data2.size() - 1;
   }
   else {
       F_statistic = var2 / var1;
       df1 = data2.size() - 1;
       df2 = data1.size() - 1;
   }

   double F_critical = f_ppf(1 - alpha / 2, df1, df2);

   outputFile << "ПРОВЕРКА РАВЕНСТВА ДИСПЕРСИЙ (F-критерий):" << endl;
   outputFile << "Дисперсия выборки 1: " << var1 << endl;
   outputFile << "Дисперсия выборки 2: " << var2 << endl;
   outputFile << "F-статистика: " << F_statistic << endl;
   outputFile << "Критическое значение F(" << df1 << "," << df2 << "): " << F_critical << endl;

   bool equalVariances = (F_statistic <= F_critical);

   if (equalVariances) {
       outputFile << "ВЫВОД: Дисперсии можно считать равными (принимаем H₀)" << endl;
   }
   else {
       outputFile << "ВЫВОД: Дисперсии значимо различаются (отвергаем H₀)" << endl;
   }
   outputFile << endl;

   return equalVariances;
}

// Точный критерий Стьюдента для равных дисперсий
void performExactTTest(const vector<double>& data1, const vector<double>& data2,
   double alpha, bool twoSided, ofstream& outputFile) {
   double mean1 = calculateMean(data1);
   double mean2 = calculateMean(data2);
   double var1 = calculateVariance(data1, mean1);
   double var2 = calculateVariance(data2, mean2);

   int n1 = data1.size();
   int n2 = data2.size();
   int df = n1 + n2 - 2;

   // Объединенная дисперсия
   double pooledVariance = ((n1 - 1) * var1 + (n2 - 1) * var2) / df;
   double pooledStdDev = sqrt(pooledVariance);

   // t-статистика
   double t_statistic = (mean1 - mean2) / (pooledStdDev * sqrt(1.0 / n1 + 1.0 / n2));

   // Критическое значение
   double t_critical = twoSided ? t_ppf(1 - alpha / 2, df) : t_ppf(1 - alpha, df);

   // p-value
   double p_value = twoSided ? 2 * (1 - t_cdf(fabs(t_statistic), df)) :
       (1 - t_cdf(fabs(t_statistic), df));

   outputFile << "ТОЧНЫЙ КРИТЕРИЙ СТЬЮДЕНТА (равные дисперсии):" << endl;
   outputFile << "Среднее выборки 1: " << mean1 << " (n=" << n1 << ")" << endl;
   outputFile << "Среднее выборки 2: " << mean2 << " (n=" << n2 << ")" << endl;
   outputFile << "Разность средних: " << mean1 - mean2 << endl;
   outputFile << "Объединенная дисперсия: " << pooledVariance << endl;
   outputFile << "t-статистика: " << t_statistic << " (df=" << df << ")" << endl;
   outputFile << "Критическое значение t: " << t_critical << endl;
   outputFile << "p-value: " << p_value << endl;

   bool significant;
   if (twoSided) {
       significant = (fabs(t_statistic) > t_critical);
       outputFile << "Гипотеза H₀: mu₁ = mu₂" << endl;
       outputFile << "Гипотеза H₁: mu₁ ≠ mu₂" << endl;
   }
   else {
       significant = (t_statistic > t_critical);
       outputFile << "Гипотеза H₀: mu₁ ≤ mu₂" << endl;
       outputFile << "Гипотеза H₁: mu₁ > mu₂" << endl;
   }

   if (significant) {
       outputFile << "СТАТИСТИЧЕСКИЙ ВЫВОД: Различия СТАТИСТИЧЕСКИ ЗНАЧИМЫ" << endl;
       outputFile << "Отвергаем нулевую гипотезу H₀ на уровне значимости " << alpha << endl;
   }
   else {
       outputFile << "СТАТИСТИЧЕСКИЙ ВЫВОД: Различия НЕ значимы" << endl;
       outputFile << "Принимаем нулевую гипотезу H₀" << endl;
   }
   outputFile << endl;
}

// Приближенный критерий Стьюдента для неравных дисперсий
void performApproximateTTest(const vector<double>& data1, const vector<double>& data2,
   double alpha, bool twoSided, ofstream& outputFile) {
   double mean1 = calculateMean(data1);
   double mean2 = calculateMean(data2);
   double var1 = calculateVariance(data1, mean1);
   double var2 = calculateVariance(data2, mean2);

   int n1 = data1.size();
   int n2 = data2.size();

   // t-статистика Уэлча
   double t_statistic = (mean1 - mean2) / sqrt(var1 / n1 + var2 / n2);

   // Степени свободы по формуле Уэлча-Саттертуэйта
   double numerator = pow(var1 / n1 + var2 / n2, 2);
   double denominator = pow(var1 / n1, 2) / (n1 - 1) + pow(var2 / n2, 2) / (n2 - 1);
   double df = numerator / denominator;

   // Критическое значение
   double t_critical = twoSided ? t_ppf(1 - alpha / 2, df) : t_ppf(1 - alpha, df);

   // p-value
   double p_value = twoSided ? 2 * (1 - t_cdf(fabs(t_statistic), df)) :
       (1 - t_cdf(fabs(t_statistic), df));

   outputFile << "ПРИБЛИЖЕННЫЙ КРИТЕРИЙ СТЬЮДЕНТА (Уэлча, неравные дисперсии):" << endl;
   outputFile << "Среднее выборки 1: " << mean1 << " (n=" << n1 << ")" << endl;
   outputFile << "Среднее выборки 2: " << mean2 << " (n=" << n2 << ")" << endl;
   outputFile << "Разность средних: " << mean1 - mean2 << endl;
   outputFile << "Дисперсия выборки 1: " << var1 << endl;
   outputFile << "Дисперсия выборки 2: " << var2 << endl;
   outputFile << "t-статистика: " << t_statistic << " (df=" << df << ")" << endl;
   outputFile << "Критическое значение t: " << t_critical << endl;
   outputFile << "p-value: " << p_value << endl;

   bool significant;
   if (twoSided) {
       significant = (fabs(t_statistic) > t_critical);
       outputFile << "Гипотеза H₀: mu₁ = mu₂" << endl;
       outputFile << "Гипотеза H₁: mu₁ ≠ mu₂" << endl;
   }
   else {
       significant = (t_statistic > t_critical);
       outputFile << "Гипотеза H₀: mu₁ ≤ mu₂" << endl;
       outputFile << "Гипотеза H₁: mu₁ > mu₂" << endl;
   }

   if (significant) {
       outputFile << "СТАТИСТИЧЕСКИЙ ВЫВОД: Различия СТАТИСТИЧЕСКИ ЗНАЧИМЫ" << endl;
       outputFile << "Отвергаем нулевую гипотезу H₀ на уровне значимости " << alpha << endl;
   }
   else {
       outputFile << "СТАТИСТИЧЕСКИЙ ВЫВОД: Различия НЕ значимы" << endl;
       outputFile << "Принимаем нулевую гипотезу H₀" << endl;
   }
   outputFile << endl;
}

// Вспомогательная функция для нахождения минимума двух чисел
size_t min_size(size_t a, size_t b) {
   return (a < b) ? a : b;
}

// Основная функция для применения критерия Стьюдента
void performTTest(const vector<double>& data1, const vector<double>& data2,
   double alpha, bool twoSided, ofstream& outputFile) {
   int n1 = data1.size();
   int n2 = data2.size();

   outputFile << "==================================================" << endl;
   outputFile << "         КРИТЕРИЙ СТЬЮДЕНТА ДЛЯ ДВУХ ВЫБОРОК" << endl;
   outputFile << "==================================================" << endl;
   outputFile << endl;

   outputFile << "ОСНОВНЫЕ ПАРАМЕТРЫ:" << endl;
   outputFile << "Объем выборки 1: n₁ = " << n1 << endl;
   outputFile << "Объем выборки 2: n₂ = " << n2 << endl;
   outputFile << "Уровень значимости: alpha = " << alpha << endl;
   outputFile << "Тип критерия: " << (twoSided ? "Двусторонний" : "Односторонний") << endl;
   outputFile << endl;

   // Проверка минимального объема выборок
   if (n1 < 2 || n2 < 2) {
       outputFile << "ОШИБКА: Объем каждой выборки должен быть не менее 2" << endl;
       return;
   }

   // Вычисляем основные статистики
   double mean1 = calculateMean(data1);
   double mean2 = calculateMean(data2);
   double stdDev1 = calculateStdDev(data1, mean1);
   double stdDev2 = calculateStdDev(data2, mean2);

   outputFile << "ОПИСАТЕЛЬНАЯ СТАТИСТИКА:" << endl;
   outputFile << "Выборка 1: среднее = " << mean1 << ", ст. отклонение = " << stdDev1 << endl;
   outputFile << "Выборка 2: среднее = " << mean2 << ", ст. отклонение = " << stdDev2 << endl;
   outputFile << "Разность средних: " << mean1 - mean2 << endl;
   outputFile << endl;

   // Проверяем равенство дисперсий
   bool equalVariances = checkEqualVariances(data1, data2, alpha, outputFile);

   // Выполняем соответствующий t-тест
   if (equalVariances) {
       performExactTTest(data1, data2, alpha, twoSided, outputFile);
   }
   else {
       outputFile << "ИСПОЛЬЗУЕТСЯ ПРИБЛИЖЕННЫЙ КРИТЕРИЙ (Уэлча)" << endl;
       outputFile << "в связи с неравенством дисперсий" << endl;
       outputFile << endl;
       performApproximateTTest(data1, data2, alpha, twoSided, outputFile);
   }

   // Дополнительная информация
   outputFile << "ДОПОЛНИТЕЛЬНАЯ ИНФОРМАЦИЯ:" << endl;
   outputFile << "Выборка 1 (первые значения): ";
   size_t displayCount = min_size(data1.size(), 10);
   for (size_t i = 0; i < displayCount; ++i) {
       outputFile << data1[i] << " ";
   }
   if (data1.size() > 10) outputFile << "...";
   outputFile << endl;

   outputFile << "Выборка 2 (первые значения): ";
   displayCount = min_size(data2.size(), 10);
   for (size_t i = 0; i < displayCount; ++i) {
       outputFile << data2[i] << " ";
   }
   if (data2.size() > 10) outputFile << "...";
   outputFile << endl;

   outputFile << "==================================================" << endl;
}

// Функция для создания тестового файла с данными
void createTestDataFile() {
   ofstream testFile("input_data_t_test.txt");
   if (testFile.is_open()) {
       testFile << "# Тестовые данные для критерия Стьюдента" << endl;
       testFile << "# Первая строка - первая выборка, вторая строка - вторая выборка" << endl;
       testFile << endl;
       testFile << "220 223 234 245 257" << endl;
       testFile << "234 246 259 262 278 280 285 290" << endl;
       testFile.close();
       cout << "Создан тестовый файл input_data_t_test.txt с примером данных" << endl;
   }
}

int main() {
   setlocale(LC_ALL, "rus");
   // Параметры критерия
   double alpha = 0.05; // Уровень значимости
   bool twoSided = true; // Двусторонний критерий

   // Имена файлов
   string inputFilename = "input_data_t_test.txt";
   string outputFilename = "student_test_result.txt";

   cout << "ПРОГРАММА ДЛЯ СРАВНЕНИЯ СРЕДНИХ ДВУХ ВЫБОРОК" << endl;
   cout << "Метод: критерий Стьюдента" << endl;
   cout << "=============================================" << endl;

   // Проверяем существование входного файла
   ifstream testFile(inputFilename);
   if (!testFile.good()) {
       cout << "Входной файл не найден. Создаю тестовый файл..." << endl;
       createTestDataFile();
   }
   testFile.close();

   // Чтение данных из файла
   cout << "Чтение данных из файла: " << inputFilename << endl;
   vector<vector<double>> datasets = readDataFromFile(inputFilename);

   if (datasets.size() < 2) {
       cerr << "ОШИБКА: Необходимо как минимум 2 выборки для сравнения" << endl;
       cerr << "Прочитано выборок: " << datasets.size() << endl;
       return 1;
   }

   cout << "Прочитано " << datasets.size() << " выборок" << endl;
   cout << "Объем выборки 1: " << datasets[0].size() << " значений" << endl;
   cout << "Объем выборки 2: " << datasets[1].size() << " значений" << endl;

   // Создание выходного файла
   ofstream outputFile(outputFilename);

   if (!outputFile.is_open()) {
       cerr << "ОШИБКА: Не удалось создать выходной файл: " << outputFilename << endl;
       return 1;
   }

   // Применение критерия Стьюдента
   cout << "Применение критерия Стьюдента..." << endl;
   performTTest(datasets[0], datasets[1], alpha, twoSided, outputFile);

   outputFile.close();

   cout << endl;
   cout << "УСПЕШНО: Критерий Стьюдента применен." << endl;
   cout << "Результаты сохранены в файл: " << outputFilename << endl;
   cout << endl;
   cout << "Параметры анализа:" << endl;
   cout << "  - Уровень значимости: alpha = " << alpha << endl;
   cout << "  - Тип критерия: " << (twoSided ? "двусторонний" : "односторонний") << endl;
   cout << "  - Объем выборки 1: n = " << datasets[0].size() << endl;
   cout << "  - Объем выборки 2: n = " << datasets[1].size() << endl;

   return 0;
}
