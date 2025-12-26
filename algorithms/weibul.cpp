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

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Глобальная структура
struct ne_simp {
    int n;
    vector<double> p;
    vector<double> x;
    vector<int> r;
    vector<int> nsample;
};
ne_simp nesm;

// ========== МАТЕМАТИЧЕСКИЕ ФУНКЦИИ ==========

// Гамма-функция
double tgamma(double x) {
    // Аппроксимация гамма-функции
    const double coef[] = {
        76.18009172947146,
        -86.50532032941677,
        24.01409824083091,
        -1.231739572450155,
        0.1208650973866179e-2,
        -0.5395239384953e-5
    };

    double y = x;
    double tmp = x + 5.5;
    tmp -= (x + 0.5) * log(tmp);
    double ser = 1.000000000190015;

    for (int j = 0; j <= 5; j++) {
        ser += coef[j] / (y + j + 1);
    }

    return exp(-tmp + log(2.5066282746310005 * ser / x));
}

// Функция нормального распределения (для вспомогательных вычислений)
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

// Функция распределения Вейбулла
double weibull_cdf(double x, double lambda, double k) {
    if (x <= 0) return 0;
    return 1 - exp(-pow(x / lambda, k));
}

// Плотность распределения Вейбулла
double weibull_pdf(double x, double lambda, double k) {
    if (x <= 0) return 0;
    return (k / lambda) * pow(x / lambda, k - 1) * exp(-pow(x / lambda, k));
}

// Квантиль распределения Вейбулла
double weibull_ppf(double p, double lambda, double k) {
    if (p <= 0 || p >= 1) return 0;
    return lambda * pow(-log(1 - p), 1 / k);
}

// ========== ФУНКЦИИ ММП ДЛЯ ВЕЙБУЛЛА ==========

// Функция минимизации для Вейбулла
double WeibullMinFunction(vector<double> xsimpl) {
    double s1, s2, s3, z, b, c;
    int i, k_count;
    if (xsimpl[0] <= 0) return 10000000.;
    s1 = 0; s2 = 0; s3 = 0; k_count = 0;
    b = xsimpl[0]; // параметр формы k

    for (i = 0; i < nesm.n; i++) {
        k_count += (1 - nesm.r[i]); // количество нецензурированных наблюдений
        s1 += pow(nesm.x[i], b);
    }

    // Оценка параметра масштаба lambda
    c = pow(s1 / k_count, 1 / b);

    for (i = 0; i < nesm.n; i++) {
        z = pow(nesm.x[i] / c, b);
        s3 += z * log(z);
        s2 += (1 - nesm.r[i]) * log(z);
    }

    double result = s3 - s2 - k_count;
    return result * result;
}

// Обращение матрицы
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

// Ковариационная матрица для Вейбулла
void CovMatrixMleW(int n, vector<double> x, vector<int> r, double lambda, double k, double**& v) {
    int i, k_count;
    double s1, s2, z, log_lambda, inv_k;
    log_lambda = log(lambda);
    inv_k = 1 / k;
    s1 = 0; s2 = 0; k_count = 0;

    for (i = 0; i < n; i++) {
        z = (log(x[i]) - log_lambda) * inv_k;
        s1 += (1 - r[i]) * z;
        s2 += z * z * exp(z);
        k_count += (1 - r[i]);
    }

    v[0][0] = double(k_count) / double(n);
    v[0][1] = (k_count + s1) / n;
    v[1][0] = (k_count + s1) / n;
    v[1][1] = (k_count + s2) / n;

    // Инвертируем матрицу
    double** inv = InverseMatrix(v, 2);
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            v[i][j] = inv[i][j];
        }
        delete[] inv[i];
    }
    delete[] inv;
}

// ========== ОПТИМИЗАЦИЯ И ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ==========

// Функция Нелдера-Мида
int neldermead(vector<double>& x0, double eps, double(*func)(vector<double>)) {
    int n = x0.size();
    int max_iter = 1000;

    vector<vector<double>> simplex(n + 1, vector<double>(n));
    vector<double> fvals(n + 1);

    // Инициализация симплекса
    simplex[0] = x0;
    fvals[0] = func(x0);

    for (int i = 1; i <= n; i++) {
        simplex[i] = x0;
        simplex[i][i - 1] += (simplex[i][i - 1] == 0) ? 0.05 : simplex[i][i - 1] * 0.05;
        fvals[i] = func(simplex[i]);
    }

    double alpha = 1.0, gamma = 2.0, rho = 0.5, sigma = 0.5;
    int iter = 0;

    while (iter < max_iter) {
        iter++;

        // Находим индексы лучшей, худшей и второй худшей точек
        int best_idx = 0, worst_idx = 0, second_worst_idx = 0;
        for (int i = 1; i <= n; i++) {
            if (fvals[i] < fvals[best_idx]) best_idx = i;
            if (fvals[i] > fvals[worst_idx]) worst_idx = i;
        }

        second_worst_idx = (worst_idx == 0) ? 1 : 0;
        for (int i = 0; i <= n; i++) {
            if (i != worst_idx && fvals[i] > fvals[second_worst_idx]) {
                second_worst_idx = i;
            }
        }

        // Проверка сходимости
        double range = 0.0;
        for (int i = 0; i <= n; i++) {
            range += pow(fvals[i] - fvals[best_idx], 2);
        }
        range = sqrt(range / (n + 1));

        if (range < eps) break;

        // Вычисляем центр масс
        vector<double> centroid(n, 0.0);
        for (int i = 0; i <= n; i++) {
            if (i != worst_idx) {
                for (int j = 0; j < n; j++) {
                    centroid[j] += simplex[i][j];
                }
            }
        }
        for (int j = 0; j < n; j++) {
            centroid[j] /= n;
        }

        // Отражение
        vector<double> reflected(n);
        for (int j = 0; j < n; j++) {
            reflected[j] = centroid[j] + alpha * (centroid[j] - simplex[worst_idx][j]);
        }
        double f_reflected = func(reflected);

        if (f_reflected < fvals[best_idx]) {
            // Расширение
            vector<double> expanded(n);
            for (int j = 0; j < n; j++) {
                expanded[j] = centroid[j] + gamma * (reflected[j] - centroid[j]);
            }
            double f_expanded = func(expanded);

            if (f_expanded < f_reflected) {
                simplex[worst_idx] = expanded;
                fvals[worst_idx] = f_expanded;
            }
            else {
                simplex[worst_idx] = reflected;
                fvals[worst_idx] = f_reflected;
            }
        }
        else if (f_reflected < fvals[second_worst_idx]) {
            simplex[worst_idx] = reflected;
            fvals[worst_idx] = f_reflected;
        }
        else {
            // Сжатие
            vector<double> contracted(n);
            if (f_reflected < fvals[worst_idx]) {
                for (int j = 0; j < n; j++) {
                    contracted[j] = centroid[j] + rho * (reflected[j] - centroid[j]);
                }
            }
            else {
                for (int j = 0; j < n; j++) {
                    contracted[j] = centroid[j] + rho * (simplex[worst_idx][j] - centroid[j]);
                }
            }

            double f_contracted = func(contracted);

            if (f_contracted < fvals[worst_idx]) {
                simplex[worst_idx] = contracted;
                fvals[worst_idx] = f_contracted;
            }
            else {
                // Уменьшение
                for (int i = 0; i <= n; i++) {
                    if (i != best_idx) {
                        for (int j = 0; j < n; j++) {
                            simplex[i][j] = simplex[best_idx][j] + sigma * (simplex[i][j] - simplex[best_idx][j]);
                        }
                        fvals[i] = func(simplex[i]);
                    }
                }
            }
        }
    }

    // Возвращаем лучшую точку
    int best_idx = 0;
    for (int i = 1; i <= n; i++) {
        if (fvals[i] < fvals[best_idx]) best_idx = i;
    }
    x0 = simplex[best_idx];

    return iter;
}

// Функция чтения данных
vector<vector<double>> readCensoredData(const string& filename) {
    setlocale(LC_ALL, "rus");
    vector<vector<double>> data;
    ifstream inp(filename);

    if (!inp.is_open()) {
        cout << "Не удалось открыть файл: " << filename << endl;
        return data;
    }

    string line;
    vector<double> values;
    vector<int> censored;

    while (getline(inp, line)) {
        if (line.empty() || line[0] == '#') continue;

        istringstream iss(line);
        double value;
        int censor;
        char comma;

        if (iss >> value >> comma >> censor) {
            values.push_back(value);
            censored.push_back(censor);
        }
        else {
            // Альтернативный формат
            iss.clear();
            iss.str(line);
            if (iss >> value >> censor) {
                values.push_back(value);
                censored.push_back(censor);
            }
        }
    }

    inp.close();

    data.push_back(values);
    data.push_back(vector<double>(censored.begin(), censored.end()));
    return data;
}

// Функция для вычисления начальных оценок параметров Вейбулла
void initialWeibullEstimates(const vector<double>& values, const vector<int>& censored,
    double& lambda_init, double& k_init) {
    int n = values.size();
    int count = 0;
    double sum_log = 0.0;
    double sum_log_x = 0.0;
    double sum_x = 0.0;

    // Вычисляем по нецензурированным данным
    for (int i = 0; i < n; i++) {
        if (censored[i] == 0) {
            sum_log += log(values[i]);
            sum_log_x += log(values[i]) * values[i];
            sum_x += values[i];
            count++;
        }
    }

    if (count == 0) {
        // Если все данные цензурированы, используем разумные начальные значения
        lambda_init = 1.0;
        k_init = 1.5;
        return;
    }

    double mean_log = sum_log / count;

    // Метод моментов для начальных оценок
    double mean = sum_x / count;
    double variance = 0.0;
    for (int i = 0; i < n; i++) {
        if (censored[i] == 0) {
            variance += pow(values[i] - mean, 2);
        }
    }
    variance /= (count - 1);

    // Начальные оценки на основе метода моментов
    if (variance > 0) {
        double cv = sqrt(variance) / mean; // коэффициент вариации
        if (cv < 1.0) {
            k_init = pow(cv, -1.086);
        }
        else {
            k_init = 1.0;
        }
        lambda_init = mean / tgamma(1 + 1 / k_init);
    }
    else {
        k_init = 1.5;
        lambda_init = mean;
    }

    // Ограничиваем значения параметров
    k_init = max(0.1, min(10.0, k_init));
    lambda_init = max(0.1, lambda_init);
}

// Основная функция оценки параметров Вейбулла
void estimateWeibullParameters(const string& inputFile, const string& outputFile) {
    setlocale(LC_ALL, "rus");

    // 1. Чтение данных
    auto data = readCensoredData(inputFile);
    if (data.empty() || data[0].empty()) {
        cout << "Ошибка чтения данных" << endl;
        return;
    }

    vector<double> values = data[0];
    vector<int> censored(data[1].begin(), data[1].end());
    int n = values.size();

    // Инициализация глобальной структуры
    nesm.n = n;
    nesm.x = values;
    nesm.r = censored;

    // 2. Начальные оценки параметров Вейбулла
    vector<double> initialParams(2); // [k, lambda]
    double lambda_init, k_init;
    initialWeibullEstimates(values, censored, lambda_init, k_init);

    // Для Вейбулла: initialParams[0] = k (форма), initialParams[1] = lambda (масштаб)
    initialParams[0] = k_init;
    initialParams[1] = lambda_init;

    int uncensored_count = 0;
    for (int i = 0; i < n; i++) {
        if (censored[i] == 0) uncensored_count++;
    }

    cout << "ОЦЕНКА ПАРАМЕТРОВ РАСПРЕДЕЛЕНИЯ ВЕЙБУЛЛА" << endl;
    cout << "========================================" << endl;
    cout << "Размер выборки: " << n << endl;
    cout << "Нецензурированных наблюдений: " << uncensored_count << endl;
    cout << "Цензурированных наблюдений: " << n - uncensored_count << endl;
    cout << "Начальные параметры Вейбулла: k = " << initialParams[0]
        << ", lambda = " << initialParams[1] << endl;

    // 3. Минимизация функции правдоподобия
    double epsilon = 1e-6;
    int iterations = neldermead(initialParams, epsilon, WeibullMinFunction);

    double k = initialParams[0];    // параметр формы
    double lambda = initialParams[1]; // параметр масштаба

    cout << "Оптимальные параметры Вейбулла: k = " << k << ", lambda = " << lambda << endl;
    cout << "Итераций метода Нелдера-Мида: " << iterations << endl;
    cout << "Значение функции правдоподобия: " << WeibullMinFunction(initialParams) << endl;

    // 4. Вычисление ковариационной матрицы
    double** cov_matrix = new double* [2];
    for (int i = 0; i < 2; i++) {
        cov_matrix[i] = new double[2];
        for (int j = 0; j < 2; j++) {
            cov_matrix[i][j] = 0.0;
        }
    }

    CovMatrixMleW(n, values, censored, lambda, k, cov_matrix);

    // 5. Запись результатов
    ofstream out(outputFile);
    if (!out.is_open()) {
        cout << "Не удалось создать файл: " << outputFile << endl;
        return;
    }

    out << "ОЦЕНКА ПАРАМЕТРОВ РАСПРЕДЕЛЕНИЯ ВЕЙБУЛЛА" << endl;
    out << "Метод максимального правдоподобия (MLE)" << endl;
    out << "========================================" << endl << endl;

    out << "ХАРАКТЕРИСТИКИ ВЫБОРКИ:" << endl;
    out << "Размер выборки: " << n << endl;
    out << "Нецензурированных наблюдений: " << uncensored_count << endl;
    out << "Цензурированных наблюдений: " << n - uncensored_count << endl << endl;

    out << "ОЦЕНКИ ПАРАМЕТРОВ MLE:" << endl;
    out << "Параметр формы (k): " << fixed << setprecision(6) << k << endl;
    out << "Параметр масштаба (λ): " << fixed << setprecision(6) << lambda << endl << endl;

    // Характеристики распределения
    double mean_weibull = lambda * tgamma(1 + 1 / k);
    double variance_weibull = lambda * lambda * (tgamma(1 + 2 / k) - pow(tgamma(1 + 1 / k), 2));
    double median_weibull = lambda * pow(log(2), 1 / k);
    double mode_weibull = (k > 1) ? lambda * pow((k - 1) / k, 1 / k) : 0;

    out << "ХАРАКТЕРИСТИКИ РАСПРЕДЕЛЕНИЯ ВЕЙБУЛЛА:" << endl;
    out << "Среднее: " << fixed << setprecision(6) << mean_weibull << endl;
    out << "Медиана: " << fixed << setprecision(6) << median_weibull << endl;
    if (k > 1) {
        out << "Мода: " << fixed << setprecision(6) << mode_weibull << endl;
    }
    else {
        out << "Мода: 0 (распределение J-образное)" << endl;
    }
    out << "Дисперсия: " << fixed << setprecision(6) << variance_weibull << endl;
    out << "Стандартное отклонение: " << fixed << setprecision(6) << sqrt(variance_weibull) << endl << endl;

    out << "КОВАРИАЦИОННАЯ МАТРИЦА ОЦЕНОК:" << endl;
    out << "[ " << setw(15) << cov_matrix[0][0] << "  " << setw(15) << cov_matrix[0][1] << " ]" << endl;
    out << "[ " << setw(15) << cov_matrix[1][0] << "  " << setw(15) << cov_matrix[1][1] << " ]" << endl << endl;

    out << "ДЕТАЛИ КОВАРИАЦИОННОЙ МАТРИЦЫ:" << endl;
    out << "Var(k) = " << scientific << setprecision(6) << cov_matrix[0][0] << endl;
    out << "Cov(k,λ) = " << scientific << setprecision(6) << cov_matrix[0][1] << endl;
    out << "Cov(λ,k) = " << scientific << setprecision(6) << cov_matrix[1][0] << endl;
    out << "Var(λ) = " << scientific << setprecision(6) << cov_matrix[1][1] << endl << endl;

    // Стандартные ошибки и корреляция
    double se_k = sqrt(cov_matrix[0][0]);
    double se_lambda = sqrt(cov_matrix[1][1]);
    double correlation = cov_matrix[0][1] / (se_k * se_lambda);

    out << "СТАТИСТИЧЕСКИЕ ХАРАКТЕРИСТИКИ ОЦЕНОК:" << endl;
    out << "Стандартная ошибка k: " << fixed << setprecision(6) << se_k << endl;
    out << "Стандартная ошибка λ: " << fixed << setprecision(6) << se_lambda << endl;
    out << "Корреляция между оценками k и λ: " << fixed << setprecision(6) << correlation << endl << endl;

    // Доверительные интервалы (приближенные, 95%)
    double z_95 = norm_ppf(0.975);
    out << "ПРИБЛИЖЕННЫЕ 95% ДОВЕРИТЕЛЬНЫЕ ИНТЕРВАЛЫ:" << endl;
    out << "k: [" << max(0.0, k - z_95 * se_k) << ", " << k + z_95 * se_k << "]" << endl;
    out << "λ: [" << max(0.0, lambda - z_95 * se_lambda) << ", " << lambda + z_95 * se_lambda << "]" << endl << endl;

    // Квантили распределения
    out << "КВАНТИЛИ РАСПРЕДЕЛЕНИЯ ВЕЙБУЛЛА:" << endl;
    vector<double> probabilities = { 0.01, 0.05, 0.1, 0.25, 0.5, 0.75, 0.9, 0.95, 0.99 };
    out << "Вероятность\tКвантиль" << endl;
    out << fixed << setprecision(6);
    for (double p : probabilities) {
        double quantile = weibull_ppf(p, lambda, k);
        out << p << "\t\t" << quantile << endl;
    }

    out.close();

    // Освобождение памяти
    for (int i = 0; i < 2; i++) {
        delete[] cov_matrix[i];
    }
    delete[] cov_matrix;

    cout << "Результаты MLE оценки распределения Вейбулла записаны в: " << outputFile << endl;
}

int main() {
    string inputFile = "data.txt";
    string outputFile = "results_weibull.txt";

    estimateWeibullParameters(inputFile, outputFile);

    return 0;
}
