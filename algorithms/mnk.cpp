#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include <numeric>
#include <random>

using namespace std;

// Определяем M_PI если не определен
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Прототипы функций
double** InverseMatrix(double** a, int n);
double** MultiplyMatrix(int rowsa, int colsa, int rowsb, int colsb, double** a, double** b);
double** TransMatrix(int m, int n, double** a);
void MleastSquare(int n, int k, double** x, double** y, double**& db, double**& b, double*& yr);

// Реализации функций распределения (из boost.cpp)
double norm_cdf(double x) {
   // Аппроксимация функции нормального распределения
   double t = 1.0 / (1.0 + 0.2316419 * fabs(x));
   double d = 0.3989423 * exp(-x * x / 2.0);
   double p = d * t * (0.3193815 + t * (-0.3565638 + t * (1.781478 + t * (-1.821256 + t * 1.330274))));

   if (x > 0) {
       return 1.0 - p;
   }
   else {
       return p;
   }
}

double norm_ppf(double p) {
   if (p <= 0 || p >= 1) return 0;

   // Аппроксимация обратной функции нормального распределения
   const double a[] = {
       2.50662823884, -18.61500062529, 41.39119773534, -25.44106049637
   };
   const double b[] = {
       -8.47351093090, 23.08336743743, -21.06224101826, 3.13082909833
   };
   const double c[] = {
       0.3374754822726147, 0.9761690190917186, 0.1607979714918209,
       0.0276438810333863, 0.0038405729373609, 0.0003951896511919,
       0.0000321767881768, 0.0000002888167364, 0.0000003960315187
   };

   double x = p - 0.5;
   double r;

   if (fabs(x) < 0.42) {
       r = x * x;
       r = x * (((a[3] * r + a[2]) * r + a[1]) * r + a[0]) /
           ((((b[3] * r + b[2]) * r + b[1]) * r + b[0]) * r + 1.0);
   }
   else {
       r = p;
       if (x > 0) r = 1.0 - p;
       r = log(-log(r));
       r = c[0] + r * (c[1] + r * (c[2] + r * (c[3] + r *
           (c[4] + r * (c[5] + r * (c[6] + r * (c[7] + r * c[8])))))));
       if (x < 0) r = -r;
   }

   return r;
}

double norm_pdf(double x) {
   return (1.0 / sqrt(2.0 * M_PI)) * exp(-0.5 * x * x);
}

// Реализация обращения матрицы (из boost.cpp)
double** InverseMatrix(double** a, int n) {
   double temp;
   int i, j, k;

   double** e = new double* [n];
   for (i = 0; i < n; i++) {
       e[i] = new double[n];
       for (j = 0; j < n; j++) {
           e[i][j] = 0;
           if (i == j)
               e[i][j] = 1;
       }
   }

   for (k = 0; k < n; k++) {
       temp = a[k][k];
       for (j = 0; j < n; j++) {
           a[k][j] /= temp;
           e[k][j] /= temp;
       }

       for (i = k + 1; i < n; i++) {
           temp = a[i][k];
           for (j = 0; j < n; j++) {
               a[i][j] -= a[k][j] * temp;
               e[i][j] -= e[k][j] * temp;
           }
       }
   }

   for (k = n - 1; k > 0; k--) {
       for (i = k - 1; i >= 0; i--) {
           temp = a[i][k];
           for (j = 0; j < n; j++) {
               a[i][j] -= a[k][j] * temp;
               e[i][j] -= e[k][j] * temp;
           }
       }
   }
   return e;
}

// Умножение матриц (из boost.cpp)
double** MultiplyMatrix(int rowsa, int colsa, int rowsb, int colsb, double** a, double** b) {
   int i, j, k;
   double t;
   double** c;
   c = new double* [rowsa];
   for (i = 0; i < rowsa; i++) c[i] = new double[colsb];

   if (colsa != rowsb) return 0;
   for (k = 0; k < colsb; k++) {
       for (i = 0; i < rowsa; i++) {
           t = 0;
           for (j = 0; j < rowsb; j++) t += a[i][j] * b[j][k];
           c[i][k] = t;
       }
   }
   return c;
}

// Транспонирование матрицы (из boost.cpp)
double** TransMatrix(int m, int n, double** a) {
   int i, j;
   double** b;
   b = new double* [n];
   for (i = 0; i < n; i++) b[i] = new double[m];
   for (i = 0; i < n; i++) {
       for (j = 0; j < m; j++) b[i][j] = a[j][i];
   }
   return b;
}

// Метод наименьших квадратов (из mls.cpp)
void MleastSquare(int n, int k, double** x, double** y, double**& db, double**& b, double*& yr) {
   int i, j;
   double s;

   db = InverseMatrix(MultiplyMatrix(k, n, n, k, TransMatrix(n, k, x), x), k);  // covariance matrix factors (k x k)
   b = MultiplyMatrix(k, n, n, 1, MultiplyMatrix(k, k, k, n, db, TransMatrix(n, k, x)), y); // coef

   for (i = 0; i < n; i++) {
       s = 0;
       for (j = 0; j < k; j++) s += b[j][0] * x[i][j];
       yr[i] = s;
   }
}

// Функция для вычисления математических ожиданий нормальных порядковых статистик (из order.cpp)
vector<double> calculateNormalOrderStatisticsExpectations(int n) {
   vector<double> expectations(n);

   for (int i = 1; i <= n; i++) {
       double p = static_cast<double>(i) / (n + 1);
       double u_p = norm_ppf(p);

       double f_u = exp(-u_p * u_p / 2.0) / sqrt(2.0 * M_PI);
       double f_prime_u = -u_p * f_u;

       // Основной член
       expectations[i - 1] = u_p;

       // Первая поправка
       expectations[i - 1] += (p * (1.0 - p)) / (2.0 * (n + 2.0)) * (f_prime_u / (f_u * f_u));

       // Вторая поправка (для большей точности)
       double term2 = (p * (1.0 - p)) / ((n + 2.0) * (n + 2.0)) *
           ((1.0 - 2.0 * p) * f_prime_u * f_prime_u / (f_u * f_u * f_u) +
               (p * (1.0 - p)) * (f_prime_u * f_prime_u * f_prime_u -
                   f_u * f_u * (-3.0 * u_p * f_u - u_p * u_p * f_prime_u)) /
               (6.0 * f_u * f_u * f_u * f_u));

       expectations[i - 1] += term2;
   }

   return expectations;
}

// Функция чтения данных (только значения, без цензурирования)
vector<double> readData(const string& filename) {
   setlocale(LC_ALL, "rus");
   vector<double> data;
   ifstream inp(filename);

   if (!inp.is_open()) {
       cout << "Не удалось открыть файл: " << filename << endl;
       return data;
   }

   string line;
   while (getline(inp, line)) {
       if (line.empty() || line[0] == '#') continue;

       istringstream iss(line);
       double value;
       char comma;
       int censor;

       // Пытаемся прочитать в формате "значение,цензурирование"
       if (iss >> value >> comma >> censor) {
           // Используем только нецензурированные данные (censor == 0)
           if (censor == 0) {
               data.push_back(value);
           }
       }
       else {
           // Если формат другой, пытаемся прочитать просто значение
           iss.clear();
           iss.str(line);
           if (iss >> value) {
               data.push_back(value);
           }
       }
   }

   inp.close();
   return data;
}

// Основная функция оценки параметров нормального распределения методом наименьших квадратов
void estimateNormalParametersMLS(const string& inputFile, const string& outputFile) {
   setlocale(LC_ALL, "rus");

   // 1. Чтение данных
   vector<double> data = readData(inputFile);
   if (data.empty()) {
       cout << "Ошибка: не удалось прочитать данные или данные отсутствуют" << endl;
       return;
   }

   int n = data.size();
   cout << "Прочитано " << n << " наблюдений" << endl;

   // 2. Сортировка данных для порядковых статистик
   vector<double> sorted_data = data;
   sort(sorted_data.begin(), sorted_data.end());

   // 3. Вычисление математических ожиданий порядковых статистик
   auto expectations = calculateNormalOrderStatisticsExpectations(n);

   // 4. Подготовка матриц для метода наименьших квадратов
   int k = 2; // число параметров (μ, σ)

   // Матрица регрессоров X (n x 2)
   double** x = new double* [n];
   for (int i = 0; i < n; i++) {
       x[i] = new double[k];
       x[i][0] = 1.0;                   // константа для μ
       x[i][1] = expectations[i];       // математическое ожидание порядковой статистики для σ
   }

   // Вектор наблюдений Y (n x 1)
   double** y = new double* [n];
   for (int i = 0; i < n; i++) {
       y[i] = new double[1];
       y[i][0] = sorted_data[i];        // отсортированные наблюдения
   }

   // Матрицы для результатов
   double** db = new double* [k];       // ковариационная матрица коэффициентов
   double** b = new double* [k];        // коэффициенты регрессии
   double* yr = new double[n];          // предсказанные значения

   for (int i = 0; i < k; i++) {
       db[i] = new double[k];
       b[i] = new double[1];
       for (int j = 0; j < k; j++) {
           db[i][j] = 0.0;
       }
       b[i][0] = 0.0;
   }

   // 5. Применение метода наименьших квадратов
   MleastSquare(n, k, x, y, db, b, yr);

   // Параметры нормального распределения
   double mu = b[0][0];     // μ = intercept
   double sigma = b[1][0];  // σ = slope

   cout << "Оценки параметров методом наименьших квадратов:" << endl;
   cout << "Среднее (mu): " << mu << endl;
   cout << "Стандартное отклонение (sigma): " << sigma << endl;

   // 6. Вычисление статистик качества оценки
   double sse = 0.0; // сумма квадратов ошибок
   double sst = 0.0; // общая сумма квадратов
   double y_mean = accumulate(sorted_data.begin(), sorted_data.end(), 0.0) / n;

   for (int i = 0; i < n; i++) {
       sse += pow(sorted_data[i] - yr[i], 2);
       sst += pow(sorted_data[i] - y_mean, 2);
   }

   double r_squared = 1.0 - sse / sst;
   double mse = sse / (n - k);

   cout << "Качество оценки:" << endl;
   cout << "R-square = " << r_squared << endl;
   cout << "MSE = " << mse << endl;

   // 7. Запись результатов
   ofstream out(outputFile);
   if (!out.is_open()) {
       cout << "Не удалось создать файл: " << outputFile << endl;
       return;
   }

   out << "Оценка параметров нормального распределения методом наименьших квадратов" << endl;
   out << "=========================================================================" << endl << endl;

   out << "Размер выборки: " << n << endl << endl;

   out << "Оцененные параметры:" << endl;
   out << "Среднее (mu): " << fixed << setprecision(6) << mu << endl;
   out << "Стандартное отклонение (sigma): " << fixed << setprecision(6) << sigma << endl << endl;

   out << "Ковариационная матрица оценок:" << endl;
   out  << setw(12) << db[0][0]  << setw(12) << db[0][1] << endl;
   out  << setw(12) << db[1][0]  << setw(12) << db[1][1] << endl;


   out << "Элементы ковариационной матрицы:" << endl;
   out << "Var(mu)  = " << scientific << setprecision(6) << db[0][0] << endl;
   out << "Cov(mu,sigma) = " << scientific << setprecision(6) << db[0][1] << endl;
   out << "Cov(sigma,mu) = " << scientific << setprecision(6) << db[1][0] << endl;
   out << "Var(sigma)  = " << scientific << setprecision(6) << db[1][1] << endl << endl;

   double correlation = db[0][1] / sqrt(db[0][0] * db[1][1]);
   out << "Корреляция между оценками mu и sigma: " << fixed << setprecision(6) << correlation << endl;

   // Стандартные ошибки
   double se_mu = sqrt(db[0][0]);
   double se_sigma = sqrt(db[1][1]);
   out << "Стандартная ошибка mu: " << fixed << setprecision(6) << se_mu << endl;
   out << "Стандартная ошибка sigma: " << fixed << setprecision(6) << se_sigma << endl << endl;

   // Доверительные интервалы (приблизительные, 95%)
   double t_value = 1.96; // для больших выборок
   double ci_mu_low = mu - t_value * se_mu;
   double ci_mu_high = mu + t_value * se_mu;
   double ci_sigma_low = sigma - t_value * se_sigma;
   double ci_sigma_high = sigma + t_value * se_sigma;

   out << "Приблизительные 95% доверительные интервалы:" << endl;
   out << "mu:  [" << fixed << setprecision(6) << ci_mu_low << ", " << ci_mu_high << "]" << endl;
   out << "sigma:  [" << fixed << setprecision(6) << ci_sigma_low << ", " << ci_sigma_high << "]" << endl << endl;

   // Статистики качества
   out << "Статистики качества оценки:" << endl;
   out << "R² (коэффициент детерминации): " << fixed << setprecision(6) << r_squared << endl;
   out << "Сумма квадратов ошибок (SSE): " << fixed << setprecision(6) << sse << endl;
   out << "Среднеквадратическая ошибка (MSE): " << fixed << setprecision(6) << mse << endl;
   out << "Среднеквадратическое отклонение (RMSE): " << fixed << setprecision(6) << sqrt(mse) << endl << endl;

   // Сравнение с выборочными оценками
   double sample_mean = accumulate(data.begin(), data.end(), 0.0) / n;
   double sample_variance = 0.0;
   for (double val : data) {
       sample_variance += pow(val - sample_mean, 2);
   }
   sample_variance /= (n - 1);
   double sample_std = sqrt(sample_variance);

   out << "Сравнение с выборочными оценками:" << endl;
   out << "Выборочное среднее: " << fixed << setprecision(6) << sample_mean << endl;
   out << "Выборочное стандартное отклонение: " << fixed << setprecision(6) << sample_std << endl;
   out << "Разница по mu: " << fixed << setprecision(6) << (mu - sample_mean) << endl;
   out << "Разница по sigma: " << fixed << setprecision(6) << (sigma - sample_std) << endl;

   out.close();

   // Освобождение памяти
   for (int i = 0; i < n; i++) {
       delete[] x[i];
       delete[] y[i];
   }
   for (int i = 0; i < k; i++) {
       delete[] db[i];
       delete[] b[i];
   }
   delete[] x;
   delete[] y;
   delete[] db;
   delete[] b;
   delete[] yr;

   cout << "Результаты записаны в: " << outputFile << endl;
}

int main() {
   string inputFile = "data.txt";
   string outputFile = "results.txt";

   estimateNormalParametersMLS(inputFile, outputFile);

   return 0;
}
