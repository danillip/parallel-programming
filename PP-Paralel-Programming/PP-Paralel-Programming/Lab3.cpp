#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <chrono>
#include <iomanip>
#include <sstream>

// ���� [HH:MM:SS]
std::string getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm;
    localtime_s(&now_tm, &now_time);

    std::ostringstream oss;
    oss << "[" << std::put_time(&now_tm, "%H:%M:%S") << "]";
    return oss.str();
}

// ������-�����
std::mutex print_mutex;

// ����� �������
std::vector<std::string> thread_colors = {
    "\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m",
    "\033[36m", "\033[91m", "\033[92m", "\033[93m", "\033[94m",
    "\033[95m", "\033[96m", "\033[97m"
};
const std::string reset_color = "\033[0m";

// ���������� �����
void safePrint(const std::string& message, int thread_id) {
    std::lock_guard<std::mutex> lock(print_mutex);
    std::cout << thread_colors[thread_id % thread_colors.size()] // + ����
        << message
        << reset_color; 
}

// ��� �������
class ThreadPool {
private:
    std::queue<std::function<void()>> tasks; // ������� �����
    std::mutex queue_mutex;                 // ������� ��� �������������
    std::condition_variable condition;      // �������� ����������
    std::atomic<bool> stop;                 // ���� ���������� ����
    std::vector<std::thread> workers;       // ������� ������

    void worker(int id) {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                condition.wait(lock, [this] { return stop || !tasks.empty(); });

                if (stop && tasks.empty()) {
                    safePrint(getCurrentTime() + " [Thread " + std::to_string(id) + "] ��������� ������.\n", id);
                    return;
                }

                task = std::move(tasks.front());
                tasks.pop();
            }
            safePrint(getCurrentTime() + " [Thread " + std::to_string(id) + "] �������� ���������� ������.\n", id);
            task();
            safePrint(getCurrentTime() + " [Thread " + std::to_string(id) + "] �������� ���������� ������.\n", id);
        }
    }

public:
    explicit ThreadPool(size_t threads) : stop(false) {
        for (size_t i = 0; i < threads; ++i) {
            workers.emplace_back([this, i] { worker(i + 1); });
        }
    }

    template <typename Func>
    void submit(Func&& task) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            tasks.emplace(std::forward<Func>(task));
        }
        condition.notify_one();
    }

    ~ThreadPool() {
        stop = true;
        condition.notify_all();
        for (std::thread& worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }
};

// �������� ���������� ������
void exampleTask(int id, const std::string& type) {
    std::ostringstream oss;
    oss << getCurrentTime()
        << " [Task " << id << " (" << type << ")] �������� ���������� �� ������ "
        << std::this_thread::get_id() << " (Task " << id << ").\n";
    safePrint(oss.str(), id);

    std::this_thread::sleep_for(std::chrono::seconds(2)); // +2 ��� ����

    oss.str("");
    oss << getCurrentTime()
        << " [Task " << id << " (" << type << ")] ���������.\n";
    safePrint(oss.str(), id);
}

int main() {
    setlocale(LC_ALL, "RU");
    // ������ = ����� ���������� �������
    const size_t num_threads = std::thread::hardware_concurrency();
    ThreadPool pool(num_threads);

    for (int i = 0; i < 20; ++i) {
        std::string type = (i % 2 == 0) ? "����" : "�������";
        pool.submit([i, type] { exampleTask(i, type); });
    }

    return 0;
}