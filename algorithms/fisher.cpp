#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <string>
#include <iomanip>
#include <functional>

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

double f_pdf(double x, double f1, double f2) {
   fisher_f_distribution<> d(f1, f2);
   return pdf(d, x);
}

// Функция для чтения данных из файла для двух выборок
pair<vector<double>, vector<double>> readTwoSamplesFromFile(const string& filename) {
   vector<double> sample1, sample2;
   ifstream file(filename);

   if (!file.is_open()) {
       cerr << "Ошибка открытия файла: " << filename << endl;
       return make_pair(sample1, sample2);
   }

   string line;
   int currentSample = 1;

   while (getline(file, line)) {
       if (line.empty()) continue;

       if (line == "Sample1:") {
           currentSample = 1;
           continue;
       }
       else if (line == "Sample2:") {
           currentSample = 2;
           continue;
       }

       double value;
       try {
           value = stod(line);
           if (currentSample == 1) {
               sample1.push_back(value);
           }
           else {
               sample2.push_back(value);
           }
       }
       catch (const exception& e) {
           cerr << "Ошибка преобразования числа: " << line << endl;
       }
   }

   file.close();
   return make_pair(sample1, sample2);
}

// Функция для вычисления выборочного среднего
double calculateMean(const vector<double>& data) {
   if (data.empty()) return 0.0;
   double sum = 0.0;
   for (double value : data) {
       sum += value;
   }
   return sum / data.size();
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

// Функция для вычисления стандартного отклонения
double calculateStdDev(const vector<double>& data, double mean) {
   return sqrt(calculateVariance(data, mean));
}

// Функция для проверки равенства дисперсий (F-критерий)
bool checkEqualVariances(const vector<double>& sample1, const vector<double>& sample2,
   double alpha, ofstream& outputFile) {
   int n1 = sample1.size();
   int n2 = sample2.size();

   if (n1 < 2 || n2 < 2) {
       outputFile << "ОШИБКА: Для проверки равенства дисперсий нужны выборки объемом >= 2" << endl;
       return false;
   }

   double mean1 = calculateMean(sample1);
   double mean2 = calculateMean(sample2);
   double var1 = calculateVariance(sample1, mean1);
   double var2 = calculateVariance(sample2, mean2);

   // Вычисляем F-статистику (большая дисперсия в числителе)
   double F_statistic;
   int f1, f2;

   if (var1 >= var2) {
       F_statistic = var1 / var2;
       f1 = n1 - 1;
       f2 = n2 - 1;
   }
   else {
       F_statistic = var2 / var1;
       f1 = n2 - 1;
       f2 = n1 - 1;
   }

   // Критическое значение F-распределения
   double F_critical = f_ppf(1 - alpha / 2, f1, f2); // Двусторонний критерий

   outputFile << "ПРОВЕРКА РАВЕНСТВА ДИСПЕРСИЙ (F-критерий):" << endl;
   outputFile << "Дисперсия выборки 1: " << var1 << " (n1 = " << n1 << ", f1 = " << f1 << ")" << endl;
   outputFile << "Дисперсия выборки 2: " << var2 << " (n2 = " << n2 << ", f2 = " << f2 << ")" << endl;
   outputFile << "F-статистика: " << F_statistic << endl;
   outputFile << "Критическое значение F(" << alpha / 2 << "; " << f1 << ", " << f2 << "): " << F_critical << endl;

   bool variancesEqual = (F_statistic <= F_critical);

   if (variancesEqual) {
       outputFile << "ВЫВОД: Дисперсии СТАТИСТИЧЕСКИ НЕ РАЗЛИЧАЮТСЯ (принимаем H₀)" << endl;
       outputFile << "Гипотеза о равенстве дисперсий принимается на уровне значимости " << alpha << endl;
   }
   else {
       outputFile << "ВЫВОД: Дисперсии СТАТИСТИЧЕСКИ РАЗЛИЧАЮТСЯ (отвергаем H₀)" << endl;
       outputFile << "Гипотеза о равенстве дисперсий отвергается на уровне значимости " << alpha << endl;
   }
   outputFile << endl;

   return variancesEqual;
}

// Точный критерий Стьюдента для равных дисперсий (формула 3.5)
void performExactTTest(const vector<double>& sample1, const vector<double>& sample2,
   double alpha, ofstream& outputFile) {
   int n1 = sample1.size();
   int n2 = sample2.size();

   double mean1 = calculateMean(sample1);
   double mean2 = calculateMean(sample2);
   double var1 = calculateVariance(sample1, mean1);
   double var2 = calculateVariance(sample2, mean2);

   // Объединенная дисперсия (формула 3.6)
   double pooledVariance = ((n1 - 1) * var1 + (n2 - 1) * var2) / (n1 + n2 - 2);
   double pooledStdDev = sqrt(pooledVariance);

   // t-статистика (формула 3.5)
   double t_statistic = (mean1 - mean2) / (pooledStdDev * sqrt(1.0 / n1 + 1.0 / n2));
   int degreesOfFreedom = n1 + n2 - 2;

   // Критическое значение
   double t_critical = t_ppf(1 - alpha / 2, degreesOfFreedom); // Двусторонний критерий

   outputFile << "ТОЧНЫЙ КРИТЕРИЙ СТЬЮДЕНТА (равные дисперсии):" << endl;
   outputFile << "Среднее выборки 1: " << mean1 << endl;
   outputFile << "Среднее выборки 2: " << mean2 << endl;
   outputFile << "Разность средних: " << mean1 - mean2 << endl;
   outputFile << "Объединенная дисперсия: " << pooledVariance << endl;
   outputFile << "t-статистика: " << t_statistic << endl;
   outputFile << "Степени свободы: f = " << degreesOfFreedom << endl;
   outputFile << "Критическое значение t(" << alpha / 2 << "; " << degreesOfFreedom << "): " << t_critical << endl;

   if (abs(t_statistic) <= t_critical) {
       outputFile << "ВЫВОД: Средние значения СТАТИСТИЧЕСКИ НЕ РАЗЛИЧАЮТСЯ" << endl;
       outputFile << "Гипотеза о равенстве средних принимается на уровне значимости " << alpha << endl;
   }
   else {
       outputFile << "ВЫВОД: Средние значения СТАТИСТИЧЕСКИ РАЗЛИЧАЮТСЯ" << endl;
       outputFile << "Гипотеза о равенстве средних отвергается на уровне значимости " << alpha << endl;
   }
   outputFile << endl;
}

// Приближенный критерий Стьюдента для неравных дисперсий (формула 3.7)
void performApproximateTTest(const vector<double>& sample1, const vector<double>& sample2,
   double alpha, ofstream& outputFile) {
   int n1 = sample1.size();
   int n2 = sample2.size();

   double mean1 = calculateMean(sample1);
   double mean2 = calculateMean(sample2);
   double var1 = calculateVariance(sample1, mean1);
   double var2 = calculateVariance(sample2, mean2);

   // t-статистика для неравных дисперсий (формула 3.7)
   double t_statistic = (mean1 - mean2) / sqrt(var1 / n1 + var2 / n2);

   // Число степеней свободы по формуле Уэлча (формулы 3.8-3.9)
   double c = (var1 / n1) / (var1 / n1 + var2 / n2);
   double degreesOfFreedom = 1.0 / (c * c / (n1 - 1) + (1 - c) * (1 - c) / (n2 - 1));

   // Критическое значение
   double t_critical = t_ppf(1 - alpha / 2, degreesOfFreedom); // Двусторонний критерий

   outputFile << "ПРИБЛИЖЕННЫЙ КРИТЕРИЙ СТЬЮДЕНТА (неравные дисперсии):" << endl;
   outputFile << "Среднее выборки 1: " << mean1 << endl;
   outputFile << "Среднее выборки 2: " << mean2 << endl;
   outputFile << "Разность средних: " << mean1 - mean2 << endl;
   outputFile << "Дисперсия выборки 1: " << var1 << endl;
   outputFile << "Дисперсия выборки 2: " << var2 << endl;
   outputFile << "t-статистика: " << t_statistic << endl;
   outputFile << "Степени свободы (по Уэлчу): f = " << degreesOfFreedom << endl;
   outputFile << "Критическое значение t(" << alpha / 2 << "; " << degreesOfFreedom << "): " << t_critical << endl;

   if (abs(t_statistic) <= t_critical) {
       outputFile << "ВЫВОД: Средние значения СТАТИСТИЧЕСКИ НЕ РАЗЛИЧАЮТСЯ" << endl;
       outputFile << "Гипотеза о равенстве средних принимается на уровне значимости " << alpha << endl;
   }
   else {
       outputFile << "ВЫВОД: Средние значения СТАТИСТИЧЕСКИ РАЗЛИЧАЮТСЯ" << endl;
       outputFile << "Гипотеза о равенстве средних отвергается на уровне значимости " << alpha << endl;
   }
   outputFile << endl;
}

// Основная функция для применения критерия Фишера-Стьюдента
void performFisherStudentTest(const vector<double>& sample1, const vector<double>& sample2,
   double alpha, ofstream& outputFile) {
   int n1 = sample1.size();
   int n2 = sample2.size();

   outputFile << "==================================================" << endl;
   outputFile << "   КРИТЕРИЙ ФИШЕРА-СТЬЮДЕНТА ДЛЯ ДВУХ ВЫБОРОК" << endl;
   outputFile << "==================================================" << endl;
   outputFile << endl;

   outputFile << "ОСНОВНЫЕ ПАРАМЕТРЫ:" << endl;
   outputFile << "Объем выборки 1: n1 = " << n1 << endl;
   outputFile << "Объем выборки 2: n2 = " << n2 << endl;
   outputFile << "Уровень значимости: alpha = " << alpha << endl;
   outputFile << "Двусторонний критерий" << endl;
   outputFile << endl;

   // Шаг 1: Проверка равенства дисперсий
   bool variancesEqual = checkEqualVariances(sample1, sample2, alpha, outputFile);

   // Шаг 2: Проверка равенства средних
   outputFile << "ПРОВЕРКА РАВЕНСТВА СРЕДНИХ:" << endl;
   if (variancesEqual) {
       outputFile << "Используется ТОЧНЫЙ критерий Стьюдента (дисперсии равны)" << endl;
       performExactTTest(sample1, sample2, alpha, outputFile);
   }
   else {
       outputFile << "Используется ПРИБЛИЖЕННЫЙ критерий Стьюдента (дисперсии различны)" << endl;
       performApproximateTTest(sample1, sample2, alpha, outputFile);
   }

   // Детальная информация о выборках
   outputFile << "ДЕТАЛЬНАЯ ИНФОРМАЦИЯ О ВЫБОРКАХ:" << endl;

   vector<double> sorted1 = sample1;
   vector<double> sorted2 = sample2;
   sort(sorted1.begin(), sorted1.end());
   sort(sorted2.begin(), sorted2.end());

   outputFile << "Выборка 1 (отсортированная):" << endl;
   for (size_t i = 0; i < sorted1.size(); ++i) {
       outputFile << "  x1[" << setw(2) << i + 1 << "] = " << setw(10) << sorted1[i] << endl;
   }

   outputFile << "Выборка 2 (отсортированная):" << endl;
   for (size_t i = 0; i < sorted2.size(); ++i) {
       outputFile << "  x2[" << setw(2) << i + 1 << "] = " << setw(10) << sorted2[i] << endl;
   }

   outputFile << endl;
   outputFile << "==================================================" << endl;
   outputFile << "СТАТИСТИЧЕСКИЕ ХАРАКТЕРИСТИКИ:" << endl;
   outputFile << "Выборка 1: среднее = " << calculateMean(sample1)
       << ", ст. отклонение = " << calculateStdDev(sample1, calculateMean(sample1)) << endl;
   outputFile << "Выборка 2: среднее = " << calculateMean(sample2)
       << ", ст. отклонение = " << calculateStdDev(sample2, calculateMean(sample2)) << endl;
   outputFile << "Коэффициент вариации 1: " << calculateStdDev(sample1, calculateMean(sample1)) / calculateMean(sample1) << endl;
   outputFile << "Коэффициент вариации 2: " << calculateStdDev(sample2, calculateMean(sample2)) / calculateMean(sample2) << endl;
   outputFile << "==================================================" << endl;
}

// Функция для создания тестового файла с данными
void createTestDataFile() {
   ofstream testFile("fisher_input_data.txt");
   if (testFile.is_open()) {
       testFile << "Sample1:" << endl;
       testFile << "220" << endl;
       testFile << "223" << endl;
       testFile << "234" << endl;
       testFile << "245" << endl;
       testFile << "257" << endl;
       testFile << endl;
       testFile << "Sample2:" << endl;
       testFile << "234" << endl;
       testFile << "246" << endl;
       testFile << "259" << endl;
       testFile << "262" << endl;
       testFile << "278" << endl;
       testFile << "280" << endl;
       testFile << "285" << endl;
       testFile << "290" << endl;
       testFile.close();
       cout << "Создан тестовый файл fisher_input_data.txt с примером данных" << endl;
   }
}

int main() {
   setlocale(LC_ALL, "rus");
   // Параметры критерия
   double alpha = 0.05; // Уровень значимости

   // Имена файлов
   string inputFilename = "fisher_input_data.txt";
   string outputFilename = "fisher_test_result.txt";

   cout << "ПРОГРАММА ДЛЯ СРАВНЕНИЯ ДВУХ ВЫБОРОК" << endl;
   cout << "Метод: критерий Фишера-Стьюдента" << endl;
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
   auto samples = readTwoSamplesFromFile(inputFilename);
   vector<double> sample1 = samples.first;
   vector<double> sample2 = samples.second;

   if (sample1.empty() || sample2.empty()) {
       cerr << "ОШИБКА: Не удалось прочитать данные из файла или одна из выборок пуста" << endl;
       return 1;
   }

   cout << "Прочитано " << sample1.size() << " значений в выборке 1" << endl;
   cout << "Прочитано " << sample2.size() << " значений в выборке 2" << endl;

   // Создание выходного файла
   ofstream outputFile(outputFilename);

   if (!outputFile.is_open()) {
       cerr << "ОШИБКА: Не удалось создать выходной файл: " << outputFilename << endl;
       return 1;
   }

   // Применение критерия Фишера-Стьюдента
   cout << "Применение критерия Фишера-Стьюдента..." << endl;
   performFisherStudentTest(sample1, sample2, alpha, outputFile);

   outputFile.close();

   cout << endl;
   cout << "УСПЕШНО: Критерий Фишера-Стьюдента применен." << endl;
   cout << "Результаты сохранены в файл: " << outputFilename << endl;
   cout << endl;
   cout << "Параметры анализа:" << endl;
   cout << "  - Уровень значимости: alpha = " << alpha << endl;
   cout << "  - Объем выборки 1: n1 = " << sample1.size() << endl;
   cout << "  - Объем выборки 2: n2 = " << sample2.size() << endl;

   return 0;
}
