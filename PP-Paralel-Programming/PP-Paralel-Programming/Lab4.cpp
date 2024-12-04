#include <windows.h>
#include <iostream>
#include <queue>
#include <functional>
#include <sstream>
#include <vector>
#include <atomic>

// ���� [HH:MM:SS]
std::string getCurrentTime() {
    SYSTEMTIME time;
    GetLocalTime(&time);

    std::ostringstream oss;
    oss << "[" << (time.wHour < 10 ? "0" : "") << time.wHour
        << ":" << (time.wMinute < 10 ? "0" : "") << time.wMinute
        << ":" << (time.wSecond < 10 ? "0" : "") << time.wSecond << "]";
    return oss.str();
}

// �������
CRITICAL_SECTION printCriticalSection; // ����������� ������ ��� ����������� ������
HANDLE queueMutex;                     // ������� ��� ���������� ��������
HANDLE tasksAvailable;                 // ������ � ������� �����
bool stopPool = false;                 // ���� ��������� ���� �������

// ������� �����
std::queue<std::function<void()>> taskQueue;

// ����� �����
std::atomic<int> completedTasks(0);

// ����� ������� !!!!����������!!!!
std::vector<WORD> threadColors = {
    FOREGROUND_RED, FOREGROUND_GREEN, FOREGROUND_BLUE, FOREGROUND_RED | FOREGROUND_GREEN,
    FOREGROUND_RED | FOREGROUND_BLUE, FOREGROUND_GREEN | FOREGROUND_BLUE, FOREGROUND_INTENSITY
};

// ���� ����� ����������
void safePrint(const std::string& message, int threadId) {
    EnterCriticalSection(&printCriticalSection);

    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(console, threadColors[threadId % threadColors.size()]);

    std::cout << message;

    SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // ����� �����?:���� ��?
    LeaveCriticalSection(&printCriticalSection);
}

// ���� �����
//DWORD WINAPI worker(LPVOID param) { //void*
//    int threadId = reinterpret_cast<int>(param);
//
//    while (true) {
//        WaitForSingleObject(tasksAvailable, INFINITE); // 1000
//
//        if (stopPool) {
//            safePrint(getCurrentTime() + " [Thread " + std::to_string(threadId) + "] ��������� ������.\n", threadId);
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
//        //    SetEvent(tasksAvailable); // ��������� ����� �� ��� �������� ��� � ���������� ����
//        //    ResetEvent(tasksAvailable);
//        //    //
//        //    //stopPool = true;
//        //    //SetEvent(tasksAvailable); // ��������� ����� �� ��� �������� ��� � ���������� ����
//        //}
//        WaitForSingleObject(queueMutex, INFINITE);
//        if (!taskQueue.empty()) {
//            task = std::move(taskQueue.front());
//            taskQueue.pop();
//        }
//        if (taskQueue.empty() && stopPool) {
//            SetEvent(tasksAvailable); // ��������� ������ ��� ����������
//        }
//        ReleaseMutex(queueMutex);
//
//        if (task) {
//            safePrint(getCurrentTime() + " [Thread " + std::to_string(threadId) + "] �������� ���������� ������.\n", threadId);
//            task();
//            ++completedTasks; // ++ �������� �����
//            safePrint(getCurrentTime() + " [Thread " + std::to_string(threadId) + "] �������� ���������� ������.\n", threadId);
//        }
//    }
//}

DWORD WINAPI worker(LPVOID param) {
    int threadId = reinterpret_cast<int>(param);

    while (true) {
        WaitForSingleObject(tasksAvailable, INFINITE); // 1000

        if (stopPool) {
            safePrint(getCurrentTime() + " [Thread " + std::to_string(threadId) + "] ��������� ������.\n", threadId);
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
            safePrint(getCurrentTime() + " [Thread " + std::to_string(threadId) + "] �������� ���������� ������.\n", threadId);
            task();
            ++completedTasks;
            safePrint(getCurrentTime() + " [Thread " + std::to_string(threadId) + "] �������� ���������� ������.\n", threadId);
        }

        // �����? = �����
        if (noMoreTasks) {
            stopPool = true;
            SetEvent(tasksAvailable);
            break;
        }
    }
    return 0;
}


// ������ � ���
void submitTask(const std::function<void()>& task) {
    WaitForSingleObject(queueMutex, INFINITE);
    taskQueue.emplace(task);
    SetEvent(tasksAvailable);
    ReleaseMutex(queueMutex);
}

// ���� ��������
void exampleTask(int id, const std::string& type) {
    std::ostringstream oss;
    oss << getCurrentTime()
        << " [Task " << id << " (" << type << ")] �������� ����������.\n";
    safePrint(oss.str(), id);

    Sleep(2000); // �������������(���� ��������)

    oss.str("");
    oss << getCurrentTime()
        << " [Task " << id << " (" << type << ")] ���������.\n";
    safePrint(oss.str(), id);
}

int main() {
    setlocale(LC_ALL, "RU");

    // INIT
    InitializeCriticalSection(&printCriticalSection);
    queueMutex = CreateMutex(NULL, FALSE, NULL);
    tasksAvailable = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (queueMutex == NULL || tasksAvailable == NULL) {
        std::cerr << "������ �������� �������� �������������!" << std::endl;
        return 1;
    }

    // ��� ������ ����� ��������
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    const int numThreads = sysInfo.dwNumberOfProcessors;

    std::vector<HANDLE> threads(numThreads);


    // + ������� �����
    for (int i = 0; i < 70; ++i) {
        std::string type = (i % 2 == 0) ? "����" : "�������";
        submitTask([i, type] { exampleTask(i, type); });
    }

    // Create Theard
    for (int i = 0; i < numThreads; ++i) {
        threads[i] = CreateThread(NULL, 0, worker, reinterpret_cast<LPVOID>(i + 1), 0, NULL);
        if (threads[i] == NULL) {
            std::cerr << "������ �������� ������!" << std::endl;
            return 1;
        }
    }

    //// + ������� �����
    //for (int i = 0; i < 70; ++i) {
    //    std::string type = (i % 2 == 0) ? "����" : "�������";
    //    submitTask([i, type] { exampleTask(i, type); });
    //}

    // ����� ����� ����� �� ����������
    //Sleep(10000);
    // ���� ���������� ���� �����
    //while (true) {
    //    WaitForSingleObject(queueMutex, INFINITE);
    //    bool allTasksProcessed = taskQueue.empty();
    //    ReleaseMutex(queueMutex);

    //    if (allTasksProcessed) {
    //        break;
    //    }
    //    Sleep(50); // ��������� ����� ����� ���������
    //}


    // ��������� ��� �������
    //stopPool = true;
    //SetEvent(tasksAvailable); // ��������� ����� �� ��� �������� ��� � ���������� ����

    // ������� ���� ��������� ����
    WaitForMultipleObjects(numThreads, threads.data(), TRUE, INFINITE);

    // ����������� ����
    for (HANDLE thread : threads) {
        CloseHandle(thread);
    }
    CloseHandle(queueMutex);
    CloseHandle(tasksAvailable);
    DeleteCriticalSection(&printCriticalSection);

    std::cout << "��� ������ ���������. ����� ����� ���������: " << completedTasks.load() << ". ��������� ���������." << std::endl;
    return 0;
}
