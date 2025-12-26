//здесь есть метода маскимального правдоподобия, но нет импорта бустовских функций, они просто переписаны из boost.cpp
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
#include <limits>

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

// ========== ФУНКЦИИ ИЗ boost.cpp ==========

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

// ========== ФУНКЦИИ ИЗ mle_normal.cpp ==========

double NormalMinFunction(vector<double> xsimpl) {
    double s1, s2, s3, s4, z, psi, p, d, c1, c2;
    int i, kx;
    s1 = 0; s2 = 0; s3 = 0; s4 = 0; kx = 0;
    if (xsimpl[0] <= 0) return 10000;
    if (xsimpl[1] <= 0) return 10000;

    for (i = 0; i < nesm.n; i++) {
        z = (nesm.x[i] - xsimpl[0]) / xsimpl[1];
        d = norm_pdf(z);
        p = norm_cdf(z);
        psi = d / (1. - p);
        s1 += (1. - nesm.r[i]) * (nesm.x[i] - xsimpl[0]);
        s2 += (1. - nesm.r[i]) * pow(nesm.x[i] - xsimpl[0], 2);
        s3 += nesm.r[i] * psi;
        s4 += nesm.r[i] * psi * z;
        kx += 1 - nesm.r[i];
    }
    c1 = s1 + xsimpl[1] * s3;
    c2 = s2 + pow(xsimpl[1], 2) * (s4 - kx);
    z = c1 * c1 + c2 * c2;
    return z;
}

void CovMatrixMleN(int n, vector<double> x, vector<int> r, double a, double s, double**& v) {
    double z, p_val, d, s1, s2, s3, psi;
    int j, k;
    s1 = 0; s2 = 0; s3 = 0; k = 0;

    for (j = 0; j < n; j++) {
        z = (x[j] - a) / s;
        p_val = norm_cdf(z);
        d = norm_pdf(z);
        psi = d / (1 - p_val);
        s1 += r[j] * psi * (psi - z);
        s2 += r[j] * psi * z * (z * (psi - z) - 1);
        s3 += r[j] * psi * (z * (psi - z) - 1);
        k += (1 - r[j]);
    }

    v[0][0] = (k + s1) / n;
    v[0][1] = s3 / n;
    v[1][0] = s3 / n;
    v[1][1] = (2 * k + s2) / n;

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

// ========== ОСНОВНЫЕ ФУНКЦИИ ПРОГРАММЫ ==========

// Функция Нелдера-Мида для оптимизации
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

// Метод максимального правдоподобия для нормального распределения
void estimateNormalMLE(const vector<double>& values, const vector<int>& censored,
    double& mu_mle, double& sigma_mle, double**& cov_matrix) {
    int n = values.size();

    // Инициализация глобальной структуры для MLE функций
    nesm.n = n;
    nesm.x = values;
    nesm.r = censored;

    // Начальные оценки (метод моментов)
    double sum = 0.0;
    int count = 0;

    for (int i = 0; i < n; i++) {
        if (censored[i] == 0) {
            sum += values[i];
            count++;
        }
    }

    vector<double> initialParams(2);
    if (count > 0) {
        initialParams[0] = sum / count;
    }
    else {
        initialParams[0] = 0.0;
    }

    double variance = 0.0;
    for (int i = 0; i < n; i++) {
        if (censored[i] == 0) {
            variance += pow(values[i] - initialParams[0], 2);
        }
    }

    if (count > 1) {
        initialParams[1] = sqrt(variance / (count - 1));
    }
    else {
        initialParams[1] = 1.0;
    }

    cout << "Начальные оценки MLE: mu = " << initialParams[0]
        << ", sigma = " << initialParams[1] << endl;

    // Минимизация функции правдоподобия методом Нелдера-Мида
    double epsilon = 1e-6;
    int iterations = neldermead(initialParams, epsilon, NormalMinFunction);

    mu_mle = initialParams[0];
    sigma_mle = initialParams[1];

    cout << "MLE оценки: mu = " << mu_mle << ", sigma = " << sigma_mle << endl;
    cout << "Итераций метода Нелдера-Мида: " << iterations << endl;
    cout << "Значение минимизируемой функции: " << NormalMinFunction(initialParams) << endl;

    // Вычисление ковариационной матрицы оценок
    cov_matrix = new double* [2];
    for (int i = 0; i < 2; i++) {
        cov_matrix[i] = new double[2];
        for (int j = 0; j < 2; j++) {
            cov_matrix[i][j] = 0.0;
        }
    }

    CovMatrixMleN(n, values, censored, mu_mle, sigma_mle, cov_matrix);
}

// Основная функция оценки параметров
void estimateNormalParameters(const string& inputFile, const string& outputFile) {
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

    // Подсчет статистики
    int uncensored_count = 0;
    for (int c : censored) {
        if (c == 0) uncensored_count++;
    }

    cout << "Размер выборки: " << n << endl;
    cout << "Нецензурированных наблюдений: " << uncensored_count << endl;
    cout << "Цензурированных наблюдений: " << n - uncensored_count << endl;

    // 2. Оценка параметров методом максимального правдоподобия
    double mu_mle, sigma_mle;
    double** cov_matrix = nullptr;

    estimateNormalMLE(values, censored, mu_mle, sigma_mle, cov_matrix);

    // 3. Запись результатов
    ofstream out(outputFile);
    if (!out.is_open()) {
        cout << "Не удалось создать файл: " << outputFile << endl;
        return;
    }

    out << "ОЦЕНКА ПАРАМЕТРОВ НОРМАЛЬНОГО РАСПРЕДЕЛЕНИЯ" << endl;
    out << "Метод максимального правдоподобия (MLE)" << endl;
    out << "==============================================" << endl << endl;

    out << "ХАРАКТЕРИСТИКИ ВЫБОРКИ:" << endl;
    out << "Размер выборки: " << n << endl;
    out << "Нецензурированных наблюдений: " << uncensored_count << endl;
    out << "Цензурированных наблюдений: " << n - uncensored_count << endl << endl;

    out << "ОЦЕНКИ ПАРАМЕТРОВ MLE:" << endl;
    out << "Среднее (μ): " << fixed << setprecision(6) << mu_mle << endl;
    out << "Стандартное отклонение (σ): " << fixed << setprecision(6) << sigma_mle << endl << endl;

    out << "КОВАРИАЦИОННАЯ МАТРИЦА ОЦЕНОК:" << endl;
    out << "[ " << setw(12) << cov_matrix[0][0] << "  " << setw(12) << cov_matrix[0][1] << " ]" << endl;
    out << "[ " << setw(12) << cov_matrix[1][0] << "  " << setw(12) << cov_matrix[1][1] << " ]" << endl << endl;

    out << "ДЕТАЛИ КОВАРИАЦИОННОЙ МАТРИЦЫ:" << endl;
    out << "Var(μ) = " << fixed << setprecision(8) << cov_matrix[0][0] << endl;
    out << "Cov(μ,σ) = " << fixed << setprecision(8) << cov_matrix[0][1] << endl;
    out << "Cov(σ,μ) = " << fixed << setprecision(8) << cov_matrix[1][0] << endl;
    out << "Var(σ) = " << fixed << setprecision(8) << cov_matrix[1][1] << endl << endl;

    // Стандартные ошибки и корреляция
    double se_mu = sqrt(cov_matrix[0][0]);
    double se_sigma = sqrt(cov_matrix[1][1]);
    double correlation = cov_matrix[0][1] / (se_mu * se_sigma);

    out << "СТАТИСТИЧЕСКИЕ ХАРАКТЕРИСТИКИ ОЦЕНОК:" << endl;
    out << "Стандартная ошибка μ: " << fixed << setprecision(6) << se_mu << endl;
    out << "Стандартная ошибка σ: " << fixed << setprecision(6) << se_sigma << endl;
    out << "Корреляция между оценками μ и σ: " << fixed << setprecision(6) << correlation << endl << endl;

    // Доверительные интервалы (приближенные, 95%)
    double z_95 = norm_ppf(0.975); // 1.96 для 95% доверительного интервала
    out << "ПРИБЛИЖЕННЫЕ 95% ДОВЕРИТЕЛЬНЫЕ ИНТЕРВАЛЫ:" << endl;
    out << "μ: [" << mu_mle - z_95 * se_mu << ", " << mu_mle + z_95 * se_mu << "]" << endl;
    out << "σ: [" << max(0.0, sigma_mle - z_95 * se_sigma) << ", " << sigma_mle + z_95 * se_sigma << "]" << endl;

    out.close();

    // Освобождение памяти
    if (cov_matrix != nullptr) {
        for (int i = 0; i < 2; i++) {
            delete[] cov_matrix[i];
        }
        delete[] cov_matrix;
    }

    cout << "Результаты MLE оценки записаны в: " << outputFile << endl;
}

int main() {
    string inputFile = "data.txt";
    string outputFile = "results_mle.txt";

    estimateNormalParameters(inputFile, outputFile);

    return 0;
}
