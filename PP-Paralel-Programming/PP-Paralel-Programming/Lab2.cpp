#include <iostream>
#include <cmath>
#include <thread>
#include <mutex>
#include <vector>
#include <iomanip>
#include <chrono>

const double A = -25;
const double B = 5;
const int N = 4; // ���������� ������� ��� ������ ������� = ������ �������
const int STEPS = 1000; // ���������� ����� ��� ������� �������

std::mutex mut; // ��������
std::mutex cout_mut; // ������-�����

// f(x)
double f(double x) {
    if (x < -20)
        return 0.1 * x;
    else if (x < -5)
        return 0.5 * sin(0.25 * x) + 2.2 * cos(0.01 * x);
    else
        return pow(x, 5) - pow(x, 4) + pow(x, 2) - x + 1;
}

// ����� ����� � ��������
void calculate_segment(double left, double right, int steps,
    double& global_min, double& global_max, double& global_integral,
    int thread_id, bool is_main_thread) {
    auto start_time = std::chrono::high_resolution_clock::now();

    double dx = (right - left) / steps;
    double local_min = f(left);
    double local_max = f(left);
    double local_integral = 0;

    for (int i = 0; i < steps; ++i) {
        double x = left + i * dx;
        double y = f(x);
        local_min = std::min(local_min, y);
        local_max = std::max(local_max, y);
        local_integral += y * dx;
    }

    //  �������
    {
        std::lock_guard<std::mutex> guard(mut);
        global_min = std::min(global_min, local_min);
        global_max = std::max(global_max, local_max);
        global_integral += local_integral;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // ������ �����
    {
        std::lock_guard<std::mutex> cout_guard(cout_mut);
        std::cout << "================================================\n";
        std::cout << (is_main_thread ? "[Main Thread]" : "[Child Thread]")
            << " ID: " << std::this_thread::get_id() << "\n";
        std::cout << "Processed Range: [" << left << ", " << right << "]\n";
        std::cout << "Local Min = " << std::fixed << std::setprecision(6) << local_min << "\n";
        std::cout << "Local Max = " << std::fixed << std::setprecision(6) << local_max << "\n";
        std::cout << "Local Integral = " << std::fixed << std::setprecision(6) << local_integral << "\n";
        std::cout << "Execution Time: " << duration.count() << " ms\n";
        std::cout << "================================================\n";
    }
}

int main() {
    auto total_start_time = std::chrono::high_resolution_clock::now();

    // ������ ������ ��������
    double segment_length = (B - A) / N;

    // ��������
    double global_min = f(A);
    double global_max = f(A);
    double global_integral = 0;

    // ������ � �����?
    std::vector<std::thread> threads;

    std::cout << "Starting threads...\n";
    for (int i = 0; i < N - 1; ++i) {
        double left = A + i * segment_length;
        double right = left + segment_length;

        threads.emplace_back(calculate_segment, left, right, STEPS / N,
            std::ref(global_min), std::ref(global_max), std::ref(global_integral), i + 1, false);
    }

    // ������� = ��������� �������
    double left = A + (N - 1) * segment_length;
    double right = B;
    calculate_segment(left, right, STEPS / N, global_min, global_max, global_integral, N, true);

    // ���� ����������
    for (auto& t : threads) {
        t.join();
    }

    auto total_end_time = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(total_end_time - total_start_time);

    // ����� ��������
    std::cout << "\n==================== GLOBAL RESULTS ====================\n";
    std::cout << "Global Min = " << std::fixed << std::setprecision(6) << global_min << "\n";
    std::cout << "Global Max = " << std::fixed << std::setprecision(6) << global_max << "\n";
    std::cout << "Global Integral = " << std::fixed << std::setprecision(6) << global_integral << "\n";
    std::cout << "Total Execution Time: " << total_duration.count() << " ms\n";
    std::cout << "========================================================\n";

    return 0;
}