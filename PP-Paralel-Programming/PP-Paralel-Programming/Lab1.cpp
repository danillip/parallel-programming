/*
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <fstream>
#include <chrono>
#include <algorithm> 
#include <future>

// однопоточная версия
void quicksort(std::vector<int>& arr, int left, int right) {
    if (left < right) {
        int center = arr[right];
        int partition_index = left;
        for (int i = left; i < right; i++) {
            if (arr[i] < center) {
                std::swap(arr[i], arr[partition_index]);
                partition_index++;
            }
        }
        std::swap(arr[partition_index], arr[right]);

        quicksort(arr, left, partition_index - 1);
        quicksort(arr, partition_index + 1, right);
    }
}

// Многопоточная версия
void quicksort_mnogo(std::vector<int>& arr, int left, int right, int depth = 0) {
    if (left < right) {
        int center = arr[right];
        int partition_index = left;
        for (int i = left; i < right; i++) {
            if (arr[i] < center) {
                std::swap(arr[i], arr[partition_index]);
                partition_index++;
            }
        }
        std::swap(arr[partition_index], arr[right]);

        // если глубина рекурсии небольша запускаем доп потоки
        if (depth < 4) {
            auto left_task = std::async(std::launch::async, quicksort_mnogo, std::ref(arr), left, partition_index - 1, depth + 1);
            quicksort_mnogo(arr, partition_index + 1, right, depth + 1);
            left_task.get();
        }
        else {
            //quicksort_mnogo(arr, left, partition_index - 1, depth + 1);
            quicksort (arr, left, partition_index - 1);
            quicksort (arr, partition_index + 1, right);
            //quicksort_mnogo(arr, partition_index + 1, right, depth + 1);
        }
    }
}

// генератор
std::vector<int> generate_chisla(size_t size) {
    std::vector<int> arr(size);
    for (size_t i = 0; i < size; ++i) {
        arr[i] = rand() % 10000;
    }
    return arr;
}

// В файл
void WiteINFile(const std::string& filename, const std::vector<int>& arr) {
    std::ofstream file(filename);
    for (const int& num : arr) {
        file << num << " ";
    }
    file.close();
}

// Замер однопоточной
template<typename Func>
double measure_time_single(Func func, std::vector<int>& arr) {
    auto start = std::chrono::high_resolution_clock::now();
    func(arr, 0, arr.size() - 1);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    return duration.count();
}

// Замер многопоточной
template<typename Func>
double measure_time_multi(Func func, std::vector<int>& arr, int depth = 0) {
    auto start = std::chrono::high_resolution_clock::now();
    func(arr, 0, arr.size() - 1, depth);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    return duration.count();
}

int main() {
    setlocale(LC_ALL, "RU");
    srand(time(NULL));

    // (< 100 элементов)
    std::vector<int> small_array = generate_chisla(99);
    WiteINFile("small_unsorted.txt", small_array);

    std::vector<int> small_array_single = small_array;
    std::vector<int> small_array_parallel = small_array;

    // однопоточная сортировка маленького массива
    double single_thread_time_small = measure_time_single(quicksort, small_array_single);
    WiteINFile("small_sorted_single_thread.txt", small_array_single);

    // многопоточная сортировка маленького массива
    double multi_thread_time_small = measure_time_multi(quicksort_mnogo, small_array_parallel, 0);
    WiteINFile("small_sorted_multi_thread.txt", small_array_parallel);

    // (> 10000 элементов)
    std::vector<int> large_array = generate_chisla(150000);
    WiteINFile("large_unsorted.txt", large_array);

    std::vector<int> large_array_single = large_array;
    std::vector<int> large_array_parallel = large_array;

    // однопоточная сортировка крупного массива
    double single_thread_time_large = measure_time_single(quicksort, large_array_single);
    WiteINFile("large_sorted_single_thread.txt", large_array_single);

    // многопоточная сортировка крупного массива
    double multi_thread_time_large = measure_time_multi(quicksort_mnogo, large_array_parallel, 0);
    WiteINFile("large_sorted_multi_thread.txt", large_array_parallel);

    // Вывод времени выполнения
    std::cout << "Малений массив (99 элементов):\n";
    std::cout << "Однопоочное время: " << single_thread_time_small << " сек\n";
    std::cout << "Многопоточное время: " << multi_thread_time_small << " сек\n\n";

    std::cout << "Большой массив (10000+ элементов):\n";
    std::cout << "Однопоочное время: " << single_thread_time_large << " сек\n";
    std::cout << "Многопоточное время: " << multi_thread_time_large << " сек\n";

    return 0;
}
*/
