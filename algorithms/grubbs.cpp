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

// Функция для чтения данных из файла
vector<double> readDataFromFile(const string& filename) {
   vector<double> data;
   ifstream file(filename);

   if (!file.is_open()) {
       cerr << "Ошибка открытия файла: " << filename << endl;
       return data;
   }

   double value;
   while (file >> value) {
       data.push_back(value);
   }

   file.close();
   return data;
}

// Функция для вычисления выборочного среднего
double calculateMean(const vector<double>& data) {
   double sum = 0.0;
   for (double value : data) {
       sum += value;
   }
   return sum / data.size();
}

// Функция для вычисления выборочного стандартного отклонения
double calculateStdDev(const vector<double>& data, double mean) {
   if (data.size() <= 1) return 0.0;

   double sumSquares = 0.0;
   for (double value : data) {
       sumSquares += (value - mean) * (value - mean);
   }
   return sqrt(sumSquares / (data.size() - 1));
}

// Функция для вычисления статистики Граббса
double calculateGrubbsStatistic(const vector<double>& data, double mean, double stdDev, bool testMax) {
   if (testMax) {
       double maxVal = *max_element(data.begin(), data.end());
       return abs(maxVal - mean) / stdDev;
   }
   else {
       double minVal = *min_element(data.begin(), data.end());
       return abs(minVal - mean) / stdDev;
   }
}

// Функция для вычисления критического значения критерия Граббса
double calculateGrubbsCriticalValue(int n, double alpha, bool twoSided) {
   double adjustedAlpha = twoSided ? alpha / (2 * n) : alpha / n;
   double t_quantile = t_ppf(1 - adjustedAlpha, n - 2);

   double numerator = (n - 1);
   double denominator = sqrt(n * (n - 2 + t_quantile * t_quantile));

   return numerator / denominator;
}

// Функция для проверки нормальности данных (упрощенная версия)
bool checkNormality(const vector<double>& data, double mean, double stdDev, ofstream& outputFile) {
   int n = data.size();
   if (n < 8) {
       outputFile << "Предупреждение: объем выборки слишком мал для надежной проверки нормальности" << endl;
       return true; // Принимаем нормальность для малых выборок
   }

   // Простая проверка на основе коэффициента вариации
   double coefficientOfVariation = stdDev / mean;
   if (coefficientOfVariation > 0.5) {
       outputFile << "Предупреждение: высокий коэффициент вариации (" << coefficientOfVariation
           << ") может указывать на ненормальность данных" << endl;
   }

   return true;
}

// Основная функция для применения критерия Граббса
void applyGrubbsTest(const vector<double>& data, double alpha, bool twoSided,
   ofstream& outputFile) {
   int n = data.size();

   if (n < 3) {
       outputFile << "ОШИБКА: Объем выборки слишком мал для применения критерия Граббса (n < 3)" << endl;
       outputFile << "Минимальный требуемый объем выборки: 3 наблюдения" << endl;
       return;
   }

   if (n > 50) {
       outputFile << "обычно не проводят, поскольку они не оказывают заметного влияния на точность оценок" << endl;
   }

   // Вычисляем выборочные характеристики
   double mean = calculateMean(data);
   double stdDev = calculateStdDev(data, mean);

   outputFile << "==================================================" << endl;
   outputFile << "         КРИТЕРИЙ ГРАББСА ДЛЯ ВЫБРОСОВ" << endl;
   outputFile << "==================================================" << endl;
   outputFile << endl;
   outputFile << "ОСНОВНЫЕ ПАРАМЕТРЫ:" << endl;
   outputFile << "Объем выборки: n = " << n << endl;
   outputFile << "Выборочное среднее: " << fixed << setprecision(6) << mean << endl;
   outputFile << "Выборочное стандартное отклонение: " << stdDev << endl;
   outputFile << "Уровень значимости: alpha = " << alpha << endl;
   outputFile << "Тип критерия: " << (twoSided ? "Двусторонний" : "Односторонний") << endl;
   outputFile << endl;

   // Проверка предположения о нормальности
   outputFile << "ПРОВЕРКА ПРЕДПОСЫЛОК:" << endl;
   bool isNormal = checkNormality(data, mean, stdDev, outputFile);
   if (!isNormal) {
       outputFile << "ВНИМАНИЕ: Данные могут не подчиняться нормальному распределению!" << endl;
       outputFile << "Результаты критерия Граббса могут быть ненадежными." << endl;
   }
   outputFile << endl;

   // Проверяем максимальное значение
   double maxValue = *max_element(data.begin(), data.end());
   double grubbsMax = calculateGrubbsStatistic(data, mean, stdDev, true);
   double criticalValue = calculateGrubbsCriticalValue(n, alpha, twoSided);

   outputFile << "ПРОВЕРКА МАКСИМАЛЬНОГО ЗНАЧЕНИЯ:" << endl;
   outputFile << "Максимальное значение: " << maxValue << endl;
   outputFile << "Статистика Граббса: mu = " << grubbsMax << endl;
   outputFile << "Критическое значение: mu_alpha = " << criticalValue << endl;

   if (grubbsMax > criticalValue) {
       outputFile << "СТАТИСТИЧЕСКИЙ ВЫВОД: Максимальное значение является АНОМАЛЬНЫМ" << endl;
       outputFile << "Гипотеза H₀ отвергается на уровне значимости " << alpha << endl;
   }
   else {
       outputFile << "СТАТИСТИЧЕСКИЙ ВЫВОД: Максимальное значение НЕ является аномальным" << endl;
       outputFile << "Гипотеза H₀ принимается" << endl;
   }
   outputFile << endl;

   // Проверяем минимальное значение
   double minValue = *min_element(data.begin(), data.end());
   double grubbsMin = calculateGrubbsStatistic(data, mean, stdDev, false);

   outputFile << "ПРОВЕРКА МИНИМАЛЬНОГО ЗНАЧЕНИЯ:" << endl;
   outputFile << "Минимальное значение: " << minValue << endl;
   outputFile << "Статистика Граббса: mu = " << grubbsMin << endl;
   outputFile << "Критическое значение: mu_alpha = " << criticalValue << endl;

   if (grubbsMin > criticalValue) {
       outputFile << "СТАТИСТИЧЕСКИЙ ВЫВОД: Минимальное значение является АНОМАЛЬНЫМ" << endl;
       outputFile << "Гипотеза H₀ отвергается на уровне значимости " << alpha << endl;
   }
   else {
       outputFile << "СТАТИСТИЧЕСКИЙ ВЫВОД: Минимальное значение НЕ является аномальным" << endl;
       outputFile << "Гипотеза H₀ принимается" << endl;
   }
   outputFile << endl;

   // Детальная информация о выборке
   outputFile << "ДЕТАЛЬНЫЙ АНАЛИЗ ВЫБОРКИ:" << endl;
   outputFile << "Все значения в порядке возрастания:" << endl;

   vector<double> sortedData = data;
   sort(sortedData.begin(), sortedData.end());

   for (size_t i = 0; i < sortedData.size(); ++i) {
       outputFile << "  x[" << setw(2) << i + 1 << "] = " << setw(10) << sortedData[i];
       if (sortedData[i] == maxValue && grubbsMax > criticalValue) {
           outputFile << "  <-- АНОМАЛЬНОЕ (максимум)";
       }
       else if (sortedData[i] == minValue && grubbsMin > criticalValue) {
           outputFile << "  <-- АНОМАЛЬНОЕ (минимум)";
       }
       else if (i == 0 || i == sortedData.size() - 1) {
           outputFile << "  <-- крайнее значение";
       }
       outputFile << endl;
   }

   outputFile << endl;
   outputFile << "==================================================" << endl;
   outputFile << "ЗАКЛЮЧЕНИЕ:" << endl;

   int anomaliesCount = 0;
   if (grubbsMax > criticalValue) anomaliesCount++;
   if (grubbsMin > criticalValue) anomaliesCount++;

   if (anomaliesCount == 0) {
       outputFile << "Аномальных выбросов в данных не обнаружено." << endl;
   }
   else {
       outputFile << "Обнаружено аномальных выбросов: " << anomaliesCount << endl;
       if (grubbsMax > criticalValue) {
           outputFile << "  - Максимальное значение " << maxValue << " является выбросом" << endl;
       }
       if (grubbsMin > criticalValue) {
           outputFile << "  - Минимальное значение " << minValue << " является выбросом" << endl;
       }
   }
   outputFile << "==================================================" << endl;
}

// Функция для создания тестового файла с данными
void createTestDataFile() {
   ofstream testFile("input_data.txt");
   if (testFile.is_open()) {
       testFile << "4.12" << endl;
       testFile << "4.99" << endl;
       testFile << "5.12" << endl;
       testFile << "5.32" << endl;
       testFile << "5.55" << endl;
       testFile << "5.76" << endl;
       testFile << "5.87" << endl;
       testFile << "5.98" << endl;
       testFile << "6.03" << endl;
       testFile << "6.10" << endl;
       testFile.close();
       cout << "Создан тестовый файл input_data.txt с примером данных" << endl;
   }
}

int main() {
   setlocale(LC_ALL, "rus");
   // Параметры критерия
   double alpha = 0.05; // Уровень значимости
   bool twoSided = true; // Двусторонний критерий

   // Имена файлов
   string inputFilename = "input_data.txt";
   string outputFilename = "grubbs_test_result.txt";

   cout << "ПРОГРАММА ДЛЯ ВЫЯВЛЕНИЯ АНОМАЛЬНЫХ ВЫБРОСОВ" << endl;
   cout << "Метод: критерий Граббса" << endl;
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
   vector<double> data = readDataFromFile(inputFilename);

   if (data.empty()) {
       cerr << "ОШИБКА: Не удалось прочитать данные из файла или файл пуст" << endl;
       return 1;
   }

   cout << "Прочитано " << data.size() << " значений" << endl;

   // Создание выходного файла
   ofstream outputFile(outputFilename);

   if (!outputFile.is_open()) {
       cerr << "ОШИБКА: Не удалось создать выходной файл: " << outputFilename << endl;
       return 1;
   }

   // Применение критерия Граббса
   cout << "Применение критерия Граббса..." << endl;
   applyGrubbsTest(data, alpha, twoSided, outputFile);

   outputFile.close();

   cout << endl;
   cout << "УСПЕШНО: Критерий Граббса применен." << endl;
   cout << "Результаты сохранены в файл: " << outputFilename << endl;
   cout << endl;
   cout << "Параметры анализа:" << endl;
   cout << "  - Уровень значимости: alpha = " << alpha << endl;
   cout << "  - Объем выборки: n = " << data.size() << endl;
   cout << "  - Тип критерия: " << (twoSided ? "двусторонний" : "односторонний") << endl;

   return 0;
}
