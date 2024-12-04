#include <windows.h>
#include <iostream>
#include <queue>
#include <functional>
#include <sstream>
#include <vector>
#include <atomic>

// тайм [HH:MM:SS]
std::string getCurrentTime() {
    SYSTEMTIME time;
    GetLocalTime(&time);

    std::ostringstream oss;
    oss << "[" << (time.wHour < 10 ? "0" : "") << time.wHour
        << ":" << (time.wMinute < 10 ? "0" : "") << time.wMinute
        << ":" << (time.wSecond < 10 ? "0" : "") << time.wSecond << "]";
    return oss.str();
}

// Глобаки
CRITICAL_SECTION printCriticalSection; // Критическая секция для безопасного вывода
HANDLE queueMutex;                     // Мьютекс для управления очередью
HANDLE tasksAvailable;                 // Сигнал о наличии задач
bool stopPool = false;                 // Флаг остановки пула потоков

// Очередь задач
std::queue<std::function<void()>> taskQueue;

// Каунт задач
std::atomic<int> completedTasks(0);

// Цвета потоков !!!!КОНСОЛЬНЫЕ!!!!
std::vector<WORD> threadColors = {
    FOREGROUND_RED, FOREGROUND_GREEN, FOREGROUND_BLUE, FOREGROUND_RED | FOREGROUND_GREEN,
    FOREGROUND_RED | FOREGROUND_BLUE, FOREGROUND_GREEN | FOREGROUND_BLUE, FOREGROUND_INTENSITY
};

// Сейф вывод консольный
void safePrint(const std::string& message, int threadId) {
    EnterCriticalSection(&printCriticalSection);

    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(console, threadColors[threadId % threadColors.size()]);

    std::cout << message;

    SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // Ресет цвета?:Надо ли?
    LeaveCriticalSection(&printCriticalSection);
}

// Мейн поток
//DWORD WINAPI worker(LPVOID param) { //void*
//    int threadId = reinterpret_cast<int>(param);
//
//    while (true) {
//        WaitForSingleObject(tasksAvailable, INFINITE); // 1000
//
//        if (stopPool) {
//            safePrint(getCurrentTime() + " [Thread " + std::to_string(threadId) + "] завершает работу.\n", threadId);
//            return 0;
//        }
//
//        std::function<void()> task;
//
//        //WaitForSingleObject(queueMutex, INFINITE);
//        //if (!taskQueue.empty()) {
//        //    task = std::move(taskQueue.front());
//        //    taskQueue.pop();
//        //}
//        //if (taskQueue.empty()) {
//        //    stopPool = true;
//        //    SetEvent(tasksAvailable); // Разбудить чтобы он все покинули дом и освободили ресы
//        //    ResetEvent(tasksAvailable);
//        //    //
//        //    //stopPool = true;
//        //    //SetEvent(tasksAvailable); // Разбудить чтобы он все покинули дом и освободили ресы
//        //}
//        WaitForSingleObject(queueMutex, INFINITE);
//        if (!taskQueue.empty()) {
//            task = std::move(taskQueue.front());
//            taskQueue.pop();
//        }
//        if (taskQueue.empty() && stopPool) {
//            SetEvent(tasksAvailable); // Разбудить потоки для завершения
//        }
//        ReleaseMutex(queueMutex);
//
//        if (task) {
//            safePrint(getCurrentTime() + " [Thread " + std::to_string(threadId) + "] начинает выполнение задачи.\n", threadId);
//            task();
//            ++completedTasks; // ++ счетчику задач
//            safePrint(getCurrentTime() + " [Thread " + std::to_string(threadId) + "] завершил выполнение задачи.\n", threadId);
//        }
//    }
//}

DWORD WINAPI worker(LPVOID param) {
    int threadId = reinterpret_cast<int>(param);

    while (true) {
        WaitForSingleObject(tasksAvailable, INFINITE); // 1000

        if (stopPool) {
            safePrint(getCurrentTime() + " [Thread " + std::to_string(threadId) + "] завершает работу.\n", threadId);
            return 0;
        }

        std::function<void()> task;

        WaitForSingleObject(queueMutex, INFINITE);
        if (!taskQueue.empty()) {
            task = std::move(taskQueue.front());
            taskQueue.pop();
        }

        bool noMoreTasks = taskQueue.empty();
        ReleaseMutex(queueMutex);

        //if (noMoreTasks) {
        //    break;
        //}

        if (task) {
            safePrint(getCurrentTime() + " [Thread " + std::to_string(threadId) + "] начинает выполнение задачи.\n", threadId);
            task();
            ++completedTasks;
            safePrint(getCurrentTime() + " [Thread " + std::to_string(threadId) + "] завершил выполнение задачи.\n", threadId);
        }

        // пусто? = сброс
        if (noMoreTasks) {
            stopPool = true;
            SetEvent(tasksAvailable);
            break;
        }
    }
    return 0;
}


// задачу в пул
void submitTask(const std::function<void()>& task) {
    WaitForSingleObject(queueMutex, INFINITE);
    taskQueue.emplace(task);
    SetEvent(tasksAvailable);
    ReleaseMutex(queueMutex);
}

// Типо работаем
void exampleTask(int id, const std::string& type) {
    std::ostringstream oss;
    oss << getCurrentTime()
        << " [Task " << id << " (" << type << ")] начинает выполнение.\n";
    safePrint(oss.str(), id);

    Sleep(2000); // Прохлаждаемся(типо работаем)

    oss.str("");
    oss << getCurrentTime()
        << " [Task " << id << " (" << type << ")] завершено.\n";
    safePrint(oss.str(), id);
}

int main() {
    setlocale(LC_ALL, "RU");

    // INIT
    InitializeCriticalSection(&printCriticalSection);
    queueMutex = CreateMutex(NULL, FALSE, NULL);
    tasksAvailable = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (queueMutex == NULL || tasksAvailable == NULL) {
        std::cerr << "Ошибка создания объектов синхронизации!" << std::endl;
        return 1;
    }

    // Все потоки проца забираем
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    const int numThreads = sysInfo.dwNumberOfProcessors;

    std::vector<HANDLE> threads(numThreads);


    // + очередь задач
    for (int i = 0; i < 70; ++i) {
        std::string type = (i % 2 == 0) ? "папа" : "ребенок";
        submitTask([i, type] { exampleTask(i, type); });
    }

    // Create Theard
    for (int i = 0; i < numThreads; ++i) {
        threads[i] = CreateThread(NULL, 0, worker, reinterpret_cast<LPVOID>(i + 1), 0, NULL);
        if (threads[i] == NULL) {
            std::cerr << "Ошибка создания потока!" << std::endl;
            return 1;
        }
    }

    //// + очередь задач
    //for (int i = 0; i < 70; ++i) {
    //    std::string type = (i % 2 == 0) ? "папа" : "ребенок";
    //    submitTask([i, type] { exampleTask(i, type); });
    //}

    // Поток имеет вермя на выполнение
    //Sleep(10000);
    // Ждем завершения всех задач
    //while (true) {
    //    WaitForSingleObject(queueMutex, INFINITE);
    //    bool allTasksProcessed = taskQueue.empty();
    //    ReleaseMutex(queueMutex);

    //    if (allTasksProcessed) {
    //        break;
    //    }
    //    Sleep(50); // Небольшая пауза перед проверкой
    //}


    // Закрываем пул потоков
    //stopPool = true;
    //SetEvent(tasksAvailable); // Разбудить чтобы он все покинули дом и освободили ресы

    // Ожидаем пока освободят ресы
    WaitForMultipleObjects(numThreads, threads.data(), TRUE, INFINITE);

    // Освобождаем ресы
    for (HANDLE thread : threads) {
        CloseHandle(thread);
    }
    CloseHandle(queueMutex);
    CloseHandle(tasksAvailable);
    DeleteCriticalSection(&printCriticalSection);

    std::cout << "Все потоки завершены. Всего задач выполнено: " << completedTasks.load() << ". Программа завершена." << std::endl;
    return 0;
}
