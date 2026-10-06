#include "Tests.hpp"
#include "SharedPtr.hpp"
#include "UniquePtr.hpp"

#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <string>
#include <utility>

struct TestMetrics {
    long long time_us;
    size_t memory_bytes;
    size_t allocations;
    bool available;
};

static TestMetrics MakeMetrics(long long time_us, size_t memory_bytes, size_t allocations) {
    return {time_us, memory_bytes, allocations, true};
}

/* Результат для запрещенных операций (для UniquePtr) */
static TestMetrics UnavailableMetrics() {
    return {0, 0, 0, false};
}

/* Одна таблица используется всеми одиночными и массивными тестами */
static void PrintComparisonTable(const char* test_name, size_t N,
                                 const TestMetrics& raw,
                                 const TestMetrics& unique,
                                 const TestMetrics& shared) {
    const int type_width = 12;
    const int n_width = 10;
    const int time_width = 12;
    const int memory_width = 16;
    const int allocations_width = 12;

    auto print_row = [=](const char* type, const TestMetrics& metrics) {
        std::cout << std::left << std::setw(type_width) << type << " | "
                  << std::right << std::setw(n_width) << N << " | ";
        if (metrics.available) {
            std::cout << std::setw(time_width) << metrics.time_us << " | "
                      << std::setw(memory_width) << metrics.memory_bytes << " | "
                      << std::setw(allocations_width) << metrics.allocations;
        } else {
            std::cout << std::setw(time_width) << "-" << " | "
                      << std::setw(memory_width) << "-" << " | "
                      << std::setw(allocations_width) << "-";
        }
        std::cout << '\n';
    };

    std::cout << '\n' << test_name << '\n';
    std::cout << std::left << std::setw(type_width) << "Pointer type" << " | "
              << std::right << std::setw(n_width) << "N" << " | "
              << std::setw(time_width) << "Time (us)" << " | "
              << std::setw(memory_width) << "Memory (bytes)" << " | "
              << std::setw(allocations_width) << "Allocations" << '\n';
    std::cout << std::string(type_width, '-') << "-+-"
              << std::string(n_width, '-') << "-+-"
              << std::string(time_width, '-') << "-+-"
              << std::string(memory_width, '-') << "-+-"
              << std::string(allocations_width, '-') << '\n';
    print_row("Raw", raw);
    print_row("UniquePtr", unique);
    print_row("SharedPtr", shared);
    std::cout << '\n';
}

static auto ExpectedSum(size_t N) {
    const long long n = static_cast<long long>(N);
    return n * (n - 1) / 2;
}

/* Повторяющаяся подготовка одиночных объектов */
static int** CreateRawObjects(size_t N, size_t offset = 0) { // offset для тестов swap, reset, чтобы старые значения отличались от новых
    int** pointers = new int*[N]();
    try {
        for (size_t i = 0; i < N; ++i) {
            pointers[i] = new int(static_cast<int>(offset + i));
        }
    } catch (...) {
        for (size_t i = 0; i < N; ++i) {
            delete pointers[i];
        }
        delete[] pointers;
        throw;
    }
    return pointers;
}

static UniquePtr<int>* CreateUniqueObjects(size_t N, size_t offset = 0) {
    UniquePtr<int>* pointers = new UniquePtr<int>[N];
    try {
        for (size_t i = 0; i < N; ++i) {
            pointers[i] = UniquePtr<int>(new int(static_cast<int>(offset + i)));
        }
    } catch (...) {
        delete[] pointers;
        throw;
    }
    return pointers;
}

static SharedPtr<int>* CreateSharedObjects(size_t N, size_t offset = 0) {
    SharedPtr<int>* pointers = new SharedPtr<int>[N];
    try {
        for (size_t i = 0; i < N; ++i) {
            pointers[i] = SharedPtr<int>(new int(static_cast<int>(offset + i)));
        }
    } catch (...) {
        delete[] pointers;
        throw;
    }
    return pointers;
}

static void DeleteRawObjects(int** pointers, size_t N) {
    for (size_t i = 0; i < N; ++i) {
        delete pointers[i];
    }
    delete[] pointers;
}

/* Проверка, что указатель непустой и содержит ожидаемое значение */
template <typename Pointer>
static bool PointedValuesAreCorrect(Pointer* pointers, size_t N, size_t offset = 0) {
    for (size_t i = 0; i < N; ++i) {
        if (!pointers[i] || *pointers[i] != static_cast<int>(offset + i)) {
            return false;
        }
    }
    return true;
}

template <typename Pointer>
static bool AllPointersAreEmpty(Pointer* pointers, size_t N) {
    for (size_t i = 0; i < N; ++i) {
        if (pointers[i]) {
            return false;
        }
    }
    return true;
}

template <typename Pointer>
static long long SumPointedValues(Pointer* pointers, size_t N) {
    long long sum = 0;
    for (size_t i = 0; i < N; ++i) {
        sum += *pointers[i];
    }
    return sum;
}

template <typename Array>
static void FillArray(Array& array, size_t N, size_t offset = 0) {
    for (size_t i = 0; i < N; ++i) {
        array[i] = static_cast<int>(offset + i);
    }
}

template <typename Array>
static bool ArrayValuesAreCorrect(const Array& array, size_t N, size_t offset = 0) {
    for (size_t i = 0; i < N; ++i) {
        if (array[i] != static_cast<int>(offset + i)) {
            return false;
        }
    }
    return true;
}

template <typename Array>
static long long SumArray(const Array& array, size_t N) {
    long long sum = 0;
    for (size_t i = 0; i < N; ++i) {
        sum += array[i];
    }
    return sum;
}

/* Тесты одиночных указателей */
void TestCreation(size_t N) {
    auto start_raw = std::chrono::steady_clock::now();
    int** raw = CreateRawObjects(N);
    auto end_raw = std::chrono::steady_clock::now();
    auto time_raw = std::chrono::duration_cast<std::chrono::microseconds>(end_raw - start_raw).count();

    if (!PointedValuesAreCorrect(raw, N)) {
        std::cerr << "[ОШИБКА] Raw: создание объектов\n";
    }

    DeleteRawObjects(raw, N);

    auto start_unique = std::chrono::steady_clock::now();
    UniquePtr<int>* unique = CreateUniqueObjects(N);
    auto end_unique = std::chrono::steady_clock::now();
    auto time_unique = std::chrono::duration_cast<std::chrono::microseconds>(end_unique - start_unique).count();

    if (!PointedValuesAreCorrect(unique, N)) {
        std::cerr << "[ОШИБКА] UniquePtr: создание объектов\n";
    }

    delete[] unique;

    auto start_shared = std::chrono::steady_clock::now();
    SharedPtr<int>* shared = CreateSharedObjects(N);
    auto end_shared = std::chrono::steady_clock::now();
    auto time_shared = std::chrono::duration_cast<std::chrono::microseconds>(end_shared - start_shared).count();

    if (!PointedValuesAreCorrect(shared, N)) {
        std::cerr << "[ОШИБКА] SharedPtr: создание объектов\n";
    }

    delete[] shared;

    PrintComparisonTable("Creation", N,
        MakeMetrics(time_raw, sizeof(int**) + N * sizeof(int*) + N * sizeof(int), 1 + N),
        MakeMetrics(time_unique, sizeof(UniquePtr<int>*) + N * sizeof(UniquePtr<int>) + N * sizeof(int), 1 + N),
        MakeMetrics(time_shared, sizeof(SharedPtr<int>*) + N * sizeof(SharedPtr<int>) + 2 * N * sizeof(int), 1 + 2 * N));
}

void TestDeletion(size_t N) {
    int** raw = CreateRawObjects(N);
    if (!PointedValuesAreCorrect(raw, N)) {
        std::cerr << "[ОШИБКА] Raw: подготовка к удалению\n";
    }

    auto start_raw = std::chrono::steady_clock::now();
    DeleteRawObjects(raw, N);
    auto end_raw = std::chrono::steady_clock::now();
    auto time_raw = std::chrono::duration_cast<std::chrono::microseconds>(end_raw - start_raw).count();

    UniquePtr<int>* unique = CreateUniqueObjects(N);
    if (!PointedValuesAreCorrect(unique, N)) {
        std::cerr << "[ОШИБКА] UniquePtr: подготовка к удалению\n";
    }

    auto start_unique = std::chrono::steady_clock::now();
    delete[] unique;
    auto end_unique = std::chrono::steady_clock::now();
    auto time_unique = std::chrono::duration_cast<std::chrono::microseconds>(end_unique - start_unique).count();

    SharedPtr<int>* shared = CreateSharedObjects(N);
    if (!PointedValuesAreCorrect(shared, N)) {
        std::cerr << "[ОШИБКА] SharedPtr: подготовка к удалению\n";
    }

    auto start_shared = std::chrono::steady_clock::now();
    delete[] shared;
    auto end_shared = std::chrono::steady_clock::now();
    auto time_shared = std::chrono::duration_cast<std::chrono::microseconds>(end_shared - start_shared).count();

    PrintComparisonTable("Deletion", N,
        MakeMetrics(time_raw, sizeof(int**) + N * sizeof(int*) + N * sizeof(int), 1 + N),
        MakeMetrics(time_unique, sizeof(UniquePtr<int>*) + N * sizeof(UniquePtr<int>) + N * sizeof(int), 1 + N),
        MakeMetrics(time_shared, sizeof(SharedPtr<int>*) + N * sizeof(SharedPtr<int>) + 2 * N * sizeof(int), 1 + 2 * N));
}

void TestAccess(size_t N) {
    int** raw = CreateRawObjects(N);

    auto start_raw = std::chrono::steady_clock::now();
    long long sum_raw = SumPointedValues(raw, N);
    auto end_raw = std::chrono::steady_clock::now();
    auto time_raw = std::chrono::duration_cast<std::chrono::microseconds>(end_raw - start_raw).count();

    if (sum_raw != ExpectedSum(N)) {
        std::cerr << "[ОШИБКА] Raw: неверная контрольная сумма\n";
    }

    DeleteRawObjects(raw, N);

    UniquePtr<int>* unique = CreateUniqueObjects(N);

    auto start_unique = std::chrono::steady_clock::now();
    long long sum_unique = SumPointedValues(unique, N);
    auto end_unique = std::chrono::steady_clock::now();
    auto time_unique = std::chrono::duration_cast<std::chrono::microseconds>(end_unique - start_unique).count();

    if (sum_unique != ExpectedSum(N)) {
        std::cerr << "[ОШИБКА] UniquePtr: неверная контрольная сумма\n";
    }

    delete[] unique;

    SharedPtr<int>* shared = CreateSharedObjects(N);

    auto start_shared = std::chrono::steady_clock::now();
    long long sum_shared = SumPointedValues(shared, N);
    auto end_shared = std::chrono::steady_clock::now();
    auto time_shared = std::chrono::duration_cast<std::chrono::microseconds>(end_shared - start_shared).count();

    if (sum_shared != ExpectedSum(N)) {
        std::cerr << "[ОШИБКА] SharedPtr: неверная контрольная сумма\n";
    }

    delete[] shared;

    PrintComparisonTable("Access", N,
        MakeMetrics(time_raw, sizeof(int**) + N * sizeof(int*) + N * sizeof(int), 1 + N),
        MakeMetrics(time_unique, sizeof(UniquePtr<int>*) + N * sizeof(UniquePtr<int>) + N * sizeof(int), 1 + N),
        MakeMetrics(time_shared, sizeof(SharedPtr<int>*) + N * sizeof(SharedPtr<int>) + 2 * N * sizeof(int), 1 + 2 * N));
}

void TestMove(size_t N) {
    int** raw_source = CreateRawObjects(N);
    int** raw_destination = new int*[N]();

    auto start_raw = std::chrono::steady_clock::now();
    for (size_t i = 0; i < N; ++i) {
        raw_destination[i] = raw_source[i];
        raw_source[i] = nullptr;
    }
    auto end_raw = std::chrono::steady_clock::now();
    auto time_raw = std::chrono::duration_cast<std::chrono::microseconds>(end_raw - start_raw).count();

    if (!AllPointersAreEmpty(raw_source, N) || !PointedValuesAreCorrect(raw_destination, N)) {
        std::cerr << "[ОШИБКА] Raw: перемещение\n";
    }

    delete[] raw_source;
    DeleteRawObjects(raw_destination, N);

    UniquePtr<int>* unique_source = CreateUniqueObjects(N);
    UniquePtr<int>* unique_destination = new UniquePtr<int>[N];

    auto start_unique = std::chrono::steady_clock::now();
    for (size_t i = 0; i < N; ++i) {
        unique_destination[i] = std::move(unique_source[i]);
    }
    auto end_unique = std::chrono::steady_clock::now();
    auto time_unique = std::chrono::duration_cast<std::chrono::microseconds>(end_unique - start_unique).count();

    if (!AllPointersAreEmpty(unique_source, N) || !PointedValuesAreCorrect(unique_destination, N)) {
        std::cerr << "[ОШИБКА] UniquePtr: перемещение\n";
    }

    delete[] unique_source;
    delete[] unique_destination;

    SharedPtr<int>* shared_source = CreateSharedObjects(N);
    SharedPtr<int>* shared_destination = new SharedPtr<int>[N];

    auto start_shared = std::chrono::steady_clock::now();
    for (size_t i = 0; i < N; ++i) {
        shared_destination[i] = std::move(shared_source[i]);
    }
    auto end_shared = std::chrono::steady_clock::now();
    auto time_shared = std::chrono::duration_cast<std::chrono::microseconds>(end_shared - start_shared).count();

    bool shared_ok = AllPointersAreEmpty(shared_source, N) && PointedValuesAreCorrect(shared_destination, N);
    for (size_t i = 0; i < N && shared_ok; ++i) {
        shared_ok = shared_destination[i].use_count() == 1;
    }
    if (!shared_ok) {
        std::cerr << "[ОШИБКА] SharedPtr: перемещение\n";
    }

    delete[] shared_source;
    delete[] shared_destination;

    PrintComparisonTable("Move", N,
        MakeMetrics(time_raw, 2 * sizeof(int**) + 2 * N * sizeof(int*) + N * sizeof(int), 2 + N),
        MakeMetrics(time_unique, 2 * sizeof(UniquePtr<int>*) + 2 * N * sizeof(UniquePtr<int>) + N * sizeof(int), 2 + N),
        MakeMetrics(time_shared, 2 * sizeof(SharedPtr<int>*) + 2 * N * sizeof(SharedPtr<int>) + 2 * N * sizeof(int), 2 + 2 * N));
}

void TestCopy(size_t N) {
    int** raw_source = CreateRawObjects(N);
    int** raw_destination = new int*[N]();

    auto start_raw = std::chrono::steady_clock::now();
    for (size_t i = 0; i < N; ++i) {
        raw_destination[i] = raw_source[i];
    }
    auto end_raw = std::chrono::steady_clock::now();
    auto time_raw = std::chrono::duration_cast<std::chrono::microseconds>(end_raw - start_raw).count();

    bool raw_ok = true;
    for (size_t i = 0; i < N && raw_ok; ++i) {
        raw_ok = raw_destination[i] == raw_source[i];
    }
    if (!raw_ok || !PointedValuesAreCorrect(raw_destination, N)) {
        std::cerr << "[ОШИБКА] Raw: копирование адресов\n";
    }

    DeleteRawObjects(raw_source, N);
    delete[] raw_destination;

    SharedPtr<int>* shared_source = CreateSharedObjects(N);
    SharedPtr<int>* shared_destination = new SharedPtr<int>[N];

    auto start_shared = std::chrono::steady_clock::now();
    for (size_t i = 0; i < N; ++i) {
        shared_destination[i] = shared_source[i];
    }
    auto end_shared = std::chrono::steady_clock::now();
    auto time_shared = std::chrono::duration_cast<std::chrono::microseconds>(end_shared - start_shared).count();

    bool shared_ok = PointedValuesAreCorrect(shared_destination, N);
    for (size_t i = 0; i < N && shared_ok; ++i) {
        shared_ok = shared_destination[i].get() == shared_source[i].get() &&
                    shared_source[i].use_count() == 2 && shared_destination[i].use_count() == 2;
    }
    if (!shared_ok) {
        std::cerr << "[ОШИБКА] SharedPtr: копирование владельцев\n";
    }

    delete[] shared_source;
    delete[] shared_destination;

    PrintComparisonTable("Copy", N,
        MakeMetrics(time_raw, 2 * sizeof(int**) + 2 * N * sizeof(int*) + N * sizeof(int), 2 + N),
        UnavailableMetrics(),
        MakeMetrics(time_shared, 2 * sizeof(SharedPtr<int>*) + 2 * N * sizeof(SharedPtr<int>) + 2 * N * sizeof(int), 2 + 2 * N));
}

void TestMultipleOwnership(size_t N) {
    int** raw = new int*[N];
    raw[0] = new int(42);

    auto start_raw = std::chrono::steady_clock::now();
    for (size_t i = 1; i < N; ++i) {
        raw[i] = raw[0];
    }
    auto end_raw = std::chrono::steady_clock::now();
    auto time_raw = std::chrono::duration_cast<std::chrono::microseconds>(end_raw - start_raw).count();

    bool raw_ok = true;
    for (size_t i = 0; i < N && raw_ok; ++i) {
        raw_ok = raw[i] == raw[0] && *raw[i] == 42;
    }
    if (!raw_ok) {
        std::cerr << "[ОШИБКА] Raw: множественные ссылки\n";
    }

    delete raw[0];
    delete[] raw;

    SharedPtr<int>* shared = new SharedPtr<int>[N];
    shared[0] = SharedPtr<int>(new int(42));

    auto start_shared = std::chrono::steady_clock::now();
    for (size_t i = 1; i < N; ++i) {
        shared[i] = shared[0];
    }
    auto end_shared = std::chrono::steady_clock::now();
    auto time_shared = std::chrono::duration_cast<std::chrono::microseconds>(end_shared - start_shared).count();

    bool shared_ok = true;
    for (size_t i = 0; i < N && shared_ok; ++i) {
        shared_ok = shared[i].get() == shared[0].get() &&
                    static_cast<size_t>(shared[i].use_count()) == N;
    }
    if (!shared_ok) {
        std::cerr << "[ОШИБКА] SharedPtr: множественное владение\n";
    }

    delete[] shared;

    PrintComparisonTable("Multiple ownership", N,
        MakeMetrics(time_raw, sizeof(int**) + N * sizeof(int*) + sizeof(int), 2),
        UnavailableMetrics(),
        MakeMetrics(time_shared, sizeof(SharedPtr<int>*) + N * sizeof(SharedPtr<int>) + 2 * sizeof(int), 3));
}

void TestReset(size_t N) {
    int** raw = CreateRawObjects(N);

    auto start_raw = std::chrono::steady_clock::now();
    for (size_t i = 0; i < N; ++i) {
        delete raw[i];
        raw[i] = new int(static_cast<int>(N + i));
    }
    auto end_raw = std::chrono::steady_clock::now();
    auto time_raw = std::chrono::duration_cast<std::chrono::microseconds>(end_raw - start_raw).count();

    if (!PointedValuesAreCorrect(raw, N, N)) {
        std::cerr << "[ОШИБКА] Raw: reset\n";
    }

    DeleteRawObjects(raw, N);

    UniquePtr<int>* unique = CreateUniqueObjects(N);

    auto start_unique = std::chrono::steady_clock::now();
    for (size_t i = 0; i < N; ++i) {
        unique[i].reset(new int(static_cast<int>(N + i)));
    }
    auto end_unique = std::chrono::steady_clock::now();
    auto time_unique = std::chrono::duration_cast<std::chrono::microseconds>(end_unique - start_unique).count();

    if (!PointedValuesAreCorrect(unique, N, N)) {
        std::cerr << "[ОШИБКА] UniquePtr: reset\n";
    }

    delete[] unique;

    SharedPtr<int>* shared = CreateSharedObjects(N);

    auto start_shared = std::chrono::steady_clock::now();
    for (size_t i = 0; i < N; ++i) {
        shared[i].reset(new int(static_cast<int>(N + i)));
    }
    auto end_shared = std::chrono::steady_clock::now();
    auto time_shared = std::chrono::duration_cast<std::chrono::microseconds>(end_shared - start_shared).count();

    bool shared_ok = PointedValuesAreCorrect(shared, N, N);
    for (size_t i = 0; i < N && shared_ok; ++i) {
        shared_ok = shared[i].use_count() == 1;
    }
    if (!shared_ok) {
        std::cerr << "[ОШИБКА] SharedPtr: reset\n";
    }

    delete[] shared;

    PrintComparisonTable("Reset", N,
        MakeMetrics(time_raw, sizeof(int**) + N * sizeof(int*) + N * sizeof(int), 1 + 2 * N),
        MakeMetrics(time_unique, sizeof(UniquePtr<int>*) + N * sizeof(UniquePtr<int>) + N * sizeof(int), 1 + 2 * N),
        MakeMetrics(time_shared, sizeof(SharedPtr<int>*) + N * sizeof(SharedPtr<int>) + 2 * N * sizeof(int), 1 + 4 * N));
}

void TestSwap(size_t N) {
    int** raw_a = CreateRawObjects(N);
    int** raw_b = CreateRawObjects(N, N);

    auto start_raw = std::chrono::steady_clock::now();
    for (size_t i = 0; i < N; ++i) {
        int* temporary = raw_a[i];
        raw_a[i] = raw_b[i];
        raw_b[i] = temporary;
    }
    auto end_raw = std::chrono::steady_clock::now();
    auto time_raw = std::chrono::duration_cast<std::chrono::microseconds>(end_raw - start_raw).count();

    if (!PointedValuesAreCorrect(raw_a, N, N) || !PointedValuesAreCorrect(raw_b, N)) {
        std::cerr << "[ОШИБКА] Raw: swap\n";
    }

    DeleteRawObjects(raw_a, N);
    DeleteRawObjects(raw_b, N);

    UniquePtr<int>* unique_a = CreateUniqueObjects(N);
    UniquePtr<int>* unique_b = CreateUniqueObjects(N, N);

    auto start_unique = std::chrono::steady_clock::now();
    for (size_t i = 0; i < N; ++i) {
        unique_a[i].swap(unique_b[i]);
    }
    auto end_unique = std::chrono::steady_clock::now();
    auto time_unique = std::chrono::duration_cast<std::chrono::microseconds>(end_unique - start_unique).count();

    if (!PointedValuesAreCorrect(unique_a, N, N) || !PointedValuesAreCorrect(unique_b, N)) {
        std::cerr << "[ОШИБКА] UniquePtr: swap\n";
    }

    delete[] unique_a;
    delete[] unique_b;

    SharedPtr<int>* shared_a = CreateSharedObjects(N);
    SharedPtr<int>* shared_b = CreateSharedObjects(N, N);

    auto start_shared = std::chrono::steady_clock::now();
    for (size_t i = 0; i < N; ++i) {
        shared_a[i].swap(shared_b[i]);
    }
    auto end_shared = std::chrono::steady_clock::now();
    auto time_shared = std::chrono::duration_cast<std::chrono::microseconds>(end_shared - start_shared).count();

    bool shared_ok = PointedValuesAreCorrect(shared_a, N, N) && PointedValuesAreCorrect(shared_b, N);
    for (size_t i = 0; i < N && shared_ok; ++i) {
        shared_ok = shared_a[i].use_count() == 1 && shared_b[i].use_count() == 1;
    }
    if (!shared_ok) {
        std::cerr << "[ОШИБКА] SharedPtr: swap\n";
    }

    delete[] shared_a;
    delete[] shared_b;

    PrintComparisonTable("Swap", N,
        MakeMetrics(time_raw, 2 * sizeof(int**) + 2 * N * sizeof(int*) + 2 * N * sizeof(int), 2 + 2 * N),
        MakeMetrics(time_unique, 2 * sizeof(UniquePtr<int>*) + 2 * N * sizeof(UniquePtr<int>) + 2 * N * sizeof(int), 2 + 2 * N),
        MakeMetrics(time_shared, 2 * sizeof(SharedPtr<int>*) + 2 * N * sizeof(SharedPtr<int>) + 4 * N * sizeof(int), 2 + 4 * N));
}

void RunAllOnePointersTests(size_t N) {
    TestCreation(N);
    TestDeletion(N);
    TestAccess(N);
    TestMove(N);
    TestCopy(N);
    TestMultipleOwnership(N);
    TestReset(N);
    TestSwap(N);
}

/* Тесты одного умного указателя, управляющего динамическим массивом */
void TestArrayCreation(size_t N) {
    auto start_raw = std::chrono::steady_clock::now();
    int* raw = new int[N];
    FillArray(raw, N);
    auto end_raw = std::chrono::steady_clock::now();
    auto time_raw = std::chrono::duration_cast<std::chrono::microseconds>(end_raw - start_raw).count();

    if (!ArrayValuesAreCorrect(raw, N)) {
        std::cerr << "[ОШИБКА] Raw array: создание\n";
    }

    delete[] raw;

    long long time_unique = 0;
    {
        auto start_unique = std::chrono::steady_clock::now();
        UniquePtr<int[]> unique(new int[N]);
        FillArray(unique, N);
        auto end_unique = std::chrono::steady_clock::now();
        time_unique = std::chrono::duration_cast<std::chrono::microseconds>(end_unique - start_unique).count();

        if (!ArrayValuesAreCorrect(unique, N)) {
            std::cerr << "[ОШИБКА] UniquePtr<int[]>: создание\n";
        }
    }

    long long time_shared = 0;
    {
        auto start_shared = std::chrono::steady_clock::now();
        SharedPtr<int[]> shared(new int[N]);
        FillArray(shared, N);
        auto end_shared = std::chrono::steady_clock::now();
        time_shared = std::chrono::duration_cast<std::chrono::microseconds>(end_shared - start_shared).count();

        if (!ArrayValuesAreCorrect(shared, N) || shared.use_count() != 1) {
            std::cerr << "[ОШИБКА] SharedPtr<int[]>: создание\n";
        }
    }

    PrintComparisonTable("Array creation", N,
        MakeMetrics(time_raw, sizeof(int*) + N * sizeof(int), 1),
        MakeMetrics(time_unique, sizeof(UniquePtr<int[]>) + N * sizeof(int), 1),
        MakeMetrics(time_shared, sizeof(SharedPtr<int[]>) + N * sizeof(int) + sizeof(int), 2));
}

void TestArrayDeletion(size_t N) {
    int* raw = new int[N];
    FillArray(raw, N);
    if (!ArrayValuesAreCorrect(raw, N)) {
        std::cerr << "[ОШИБКА] Raw array: подготовка к удалению\n";
    }

    auto start_raw = std::chrono::steady_clock::now();
    delete[] raw;
    auto end_raw = std::chrono::steady_clock::now();
    auto time_raw = std::chrono::duration_cast<std::chrono::microseconds>(end_raw - start_raw).count();

    UniquePtr<int[]> unique(new int[N]);
    FillArray(unique, N);
    if (!ArrayValuesAreCorrect(unique, N)) {
        std::cerr << "[ОШИБКА] UniquePtr<int[]>: подготовка к удалению\n";
    }

    auto start_unique = std::chrono::steady_clock::now();
    {
        UniquePtr<int[]> deleted_unique(std::move(unique));
    }
    auto end_unique = std::chrono::steady_clock::now();
    auto time_unique = std::chrono::duration_cast<std::chrono::microseconds>(end_unique - start_unique).count();

    if (unique) {
        std::cerr << "[ОШИБКА] UniquePtr<int[]>: источник не пуст после удаления\n";
    }

    SharedPtr<int[]> shared(new int[N]);
    FillArray(shared, N);
    if (!ArrayValuesAreCorrect(shared, N) || shared.use_count() != 1) {
        std::cerr << "[ОШИБКА] SharedPtr<int[]>: подготовка к удалению\n";
    }

    auto start_shared = std::chrono::steady_clock::now();
    {
        SharedPtr<int[]> deleted_shared(std::move(shared));
    }
    auto end_shared = std::chrono::steady_clock::now();
    auto time_shared = std::chrono::duration_cast<std::chrono::microseconds>(end_shared - start_shared).count();

    if (shared) {
        std::cerr << "[ОШИБКА] SharedPtr<int[]>: источник не пуст после удаления\n";
    }

    PrintComparisonTable("Array deletion", N,
        MakeMetrics(time_raw, sizeof(int*) + N * sizeof(int), 1),
        MakeMetrics(time_unique, sizeof(UniquePtr<int[]>) + N * sizeof(int), 1),
        MakeMetrics(time_shared, sizeof(SharedPtr<int[]>) + N * sizeof(int) + sizeof(int), 2));
}

void TestArrayAccess(size_t N) {
    int* raw = new int[N];
    FillArray(raw, N);

    auto start_raw = std::chrono::steady_clock::now();
    long long sum_raw = SumArray(raw, N);
    auto end_raw = std::chrono::steady_clock::now();
    auto time_raw = std::chrono::duration_cast<std::chrono::microseconds>(end_raw - start_raw).count();

    if (sum_raw != ExpectedSum(N)) {
        std::cerr << "[ОШИБКА] Raw array: неверная контрольная сумма\n";
    }

    delete[] raw;

    UniquePtr<int[]> unique(new int[N]);
    FillArray(unique, N);

    auto start_unique = std::chrono::steady_clock::now();
    long long sum_unique = SumArray(unique, N);
    auto end_unique = std::chrono::steady_clock::now();
    auto time_unique = std::chrono::duration_cast<std::chrono::microseconds>(end_unique - start_unique).count();

    if (sum_unique != ExpectedSum(N)) {
        std::cerr << "[ОШИБКА] UniquePtr<int[]>: неверная контрольная сумма\n";
    }

    SharedPtr<int[]> shared(new int[N]);
    FillArray(shared, N);

    auto start_shared = std::chrono::steady_clock::now();
    long long sum_shared = SumArray(shared, N);
    auto end_shared = std::chrono::steady_clock::now();
    auto time_shared = std::chrono::duration_cast<std::chrono::microseconds>(end_shared - start_shared).count();

    if (sum_shared != ExpectedSum(N)) {
        std::cerr << "[ОШИБКА] SharedPtr<int[]>: неверная контрольная сумма\n";
    }

    PrintComparisonTable("Array access", N,
        MakeMetrics(time_raw, sizeof(int*) + N * sizeof(int), 1),
        MakeMetrics(time_unique, sizeof(UniquePtr<int[]>) + N * sizeof(int), 1),
        MakeMetrics(time_shared, sizeof(SharedPtr<int[]>) + N * sizeof(int) + sizeof(int), 2));
}

void TestArrayMove(size_t N) {
    int* raw_source = new int[N];
    FillArray(raw_source, N);
    int* raw_original = raw_source;
    int* raw_destination = nullptr;

    auto start_raw = std::chrono::steady_clock::now();
    raw_destination = raw_source;
    raw_source = nullptr;
    auto end_raw = std::chrono::steady_clock::now();
    auto time_raw = std::chrono::duration_cast<std::chrono::microseconds>(end_raw - start_raw).count();

    if (raw_source != nullptr || raw_destination != raw_original || !ArrayValuesAreCorrect(raw_destination, N)) {
        std::cerr << "[ОШИБКА] Raw array: перемещение\n";
    }

    delete[] raw_destination;

    UniquePtr<int[]> unique_source(new int[N]);
    FillArray(unique_source, N);
    int* unique_original = unique_source.get();
    UniquePtr<int[]> unique_destination;

    auto start_unique = std::chrono::steady_clock::now();
    unique_destination = std::move(unique_source);
    auto end_unique = std::chrono::steady_clock::now();
    auto time_unique = std::chrono::duration_cast<std::chrono::microseconds>(end_unique - start_unique).count();

    if (unique_source || unique_destination.get() != unique_original ||
        !ArrayValuesAreCorrect(unique_destination, N)) {
        std::cerr << "[ОШИБКА] UniquePtr<int[]>: перемещение\n";
    }

    SharedPtr<int[]> shared_source(new int[N]);
    FillArray(shared_source, N);
    int* shared_original = shared_source.get();
    SharedPtr<int[]> shared_destination;

    auto start_shared = std::chrono::steady_clock::now();
    shared_destination = std::move(shared_source);
    auto end_shared = std::chrono::steady_clock::now();
    auto time_shared = std::chrono::duration_cast<std::chrono::microseconds>(end_shared - start_shared).count();

    if (shared_source || shared_destination.get() != shared_original ||
        shared_destination.use_count() != 1 || !ArrayValuesAreCorrect(shared_destination, N)) {
        std::cerr << "[ОШИБКА] SharedPtr<int[]>: перемещение\n";
    }

    PrintComparisonTable("Array move", N,
        MakeMetrics(time_raw, 2 * sizeof(int*) + N * sizeof(int), 1),
        MakeMetrics(time_unique, 2 * sizeof(UniquePtr<int[]>) + N * sizeof(int), 1),
        MakeMetrics(time_shared, 2 * sizeof(SharedPtr<int[]>) + N * sizeof(int) + sizeof(int), 2));
}

void TestArrayCopy(size_t N) {
    int* raw_source = new int[N];
    FillArray(raw_source, N);
    int* raw_destination = nullptr;

    auto start_raw = std::chrono::steady_clock::now();
    raw_destination = raw_source;
    auto end_raw = std::chrono::steady_clock::now();
    auto time_raw = std::chrono::duration_cast<std::chrono::microseconds>(end_raw - start_raw).count();

    if (raw_destination != raw_source || !ArrayValuesAreCorrect(raw_destination, N)) {
        std::cerr << "[ОШИБКА] Raw array: копирование адреса\n";
    }

    delete[] raw_source;

    SharedPtr<int[]> shared_source(new int[N]);
    FillArray(shared_source, N);
    SharedPtr<int[]> shared_destination;

    auto start_shared = std::chrono::steady_clock::now();
    shared_destination = shared_source;
    auto end_shared = std::chrono::steady_clock::now();
    auto time_shared = std::chrono::duration_cast<std::chrono::microseconds>(end_shared - start_shared).count();

    if (shared_destination.get() != shared_source.get() ||
        shared_source.use_count() != 2 || shared_destination.use_count() != 2 ||
        !ArrayValuesAreCorrect(shared_destination, N)) {
        std::cerr << "[ОШИБКА] SharedPtr<int[]>: копирование владельца\n";
    }

    PrintComparisonTable("Array copy", N,
        MakeMetrics(time_raw, 2 * sizeof(int*) + N * sizeof(int), 1),
        UnavailableMetrics(),
        MakeMetrics(time_shared, 2 * sizeof(SharedPtr<int[]>) + N * sizeof(int) + sizeof(int), 2));
}

void TestArrayMultipleOwnership(size_t N) {
    const size_t owner_count = 5;

    int** raw_owners = new int*[owner_count];
    raw_owners[0] = new int[N];
    FillArray(raw_owners[0], N);

    auto start_raw = std::chrono::steady_clock::now();
    for (size_t i = 1; i < owner_count; ++i) {
        raw_owners[i] = raw_owners[0];
    }
    auto end_raw = std::chrono::steady_clock::now();
    auto time_raw = std::chrono::duration_cast<std::chrono::microseconds>(end_raw - start_raw).count();

    bool raw_ok = ArrayValuesAreCorrect(raw_owners[owner_count - 1], N);
    for (size_t i = 0; i < owner_count && raw_ok; ++i) {
        raw_ok = raw_owners[i] == raw_owners[0];
    }
    if (!raw_ok) {
        std::cerr << "[ОШИБКА] Raw array: множественные ссылки\n";
    }

    delete[] raw_owners[0];
    delete[] raw_owners;

    SharedPtr<int[]>* shared_owners = new SharedPtr<int[]>[owner_count];
    shared_owners[0] = SharedPtr<int[]>(new int[N]);
    FillArray(shared_owners[0], N);

    auto start_shared = std::chrono::steady_clock::now();
    for (size_t i = 1; i < owner_count; ++i) {
        shared_owners[i] = shared_owners[0];
    }
    auto end_shared = std::chrono::steady_clock::now();
    auto time_shared = std::chrono::duration_cast<std::chrono::microseconds>(end_shared - start_shared).count();

    bool shared_ok = ArrayValuesAreCorrect(shared_owners[owner_count - 1], N);
    for (size_t i = 0; i < owner_count && shared_ok; ++i) {
        shared_ok = shared_owners[i].get() == shared_owners[0].get() &&
                    shared_owners[i].use_count() == static_cast<int>(owner_count);
    }
    if (!shared_ok) {
        std::cerr << "[ОШИБКА] SharedPtr<int[]>: множественное владение\n";
    }

    delete[] shared_owners;

    PrintComparisonTable("Array multiple ownership", N,
        MakeMetrics(time_raw, sizeof(int**) + owner_count * sizeof(int*) + N * sizeof(int), 2),
        UnavailableMetrics(),
        MakeMetrics(time_shared, sizeof(SharedPtr<int[]>*) + owner_count * sizeof(SharedPtr<int[]>) + N * sizeof(int) + sizeof(int), 3));
}

void TestArrayReset(size_t N) {
    int* raw = new int[N];
    FillArray(raw, N);

    auto start_raw = std::chrono::steady_clock::now();
    delete[] raw;
    raw = new int[N];
    auto end_raw = std::chrono::steady_clock::now();
    auto time_raw = std::chrono::duration_cast<std::chrono::microseconds>(end_raw - start_raw).count();

    FillArray(raw, N, N);
    if (!ArrayValuesAreCorrect(raw, N, N)) {
        std::cerr << "[ОШИБКА] Raw array: reset\n";
    }

    delete[] raw;

    UniquePtr<int[]> unique(new int[N]);
    FillArray(unique, N);

    auto start_unique = std::chrono::steady_clock::now();
    unique.reset(new int[N]);
    auto end_unique = std::chrono::steady_clock::now();
    auto time_unique = std::chrono::duration_cast<std::chrono::microseconds>(end_unique - start_unique).count();

    FillArray(unique, N, N);
    if (!ArrayValuesAreCorrect(unique, N, N)) {
        std::cerr << "[ОШИБКА] UniquePtr<int[]>: reset\n";
    }

    SharedPtr<int[]> shared(new int[N]);
    FillArray(shared, N);

    auto start_shared = std::chrono::steady_clock::now();
    shared.reset(new int[N]);
    auto end_shared = std::chrono::steady_clock::now();
    auto time_shared = std::chrono::duration_cast<std::chrono::microseconds>(end_shared - start_shared).count();

    FillArray(shared, N, N);
    if (!ArrayValuesAreCorrect(shared, N, N) || shared.use_count() != 1) {
        std::cerr << "[ОШИБКА] SharedPtr<int[]>: reset\n";
    }

    PrintComparisonTable("Array reset", N,
        MakeMetrics(time_raw, sizeof(int*) + N * sizeof(int), 2),
        MakeMetrics(time_unique, sizeof(UniquePtr<int[]>) + N * sizeof(int), 2),
        MakeMetrics(time_shared, sizeof(SharedPtr<int[]>) + N * sizeof(int) + sizeof(int), 4));
}

void TestArraySwap(size_t N) {
    int* raw_a = new int[N];
    int* raw_b = new int[N];
    FillArray(raw_a, N);
    FillArray(raw_b, N, N);

    auto start_raw = std::chrono::steady_clock::now();
    int* raw_temporary = raw_a;
    raw_a = raw_b;
    raw_b = raw_temporary;
    auto end_raw = std::chrono::steady_clock::now();
    auto time_raw = std::chrono::duration_cast<std::chrono::microseconds>(end_raw - start_raw).count();

    if (!ArrayValuesAreCorrect(raw_a, N, N) || !ArrayValuesAreCorrect(raw_b, N)) {
        std::cerr << "[ОШИБКА] Raw array: swap\n";
    }

    delete[] raw_a;
    delete[] raw_b;

    UniquePtr<int[]> unique_a(new int[N]);
    UniquePtr<int[]> unique_b(new int[N]);
    FillArray(unique_a, N);
    FillArray(unique_b, N, N);

    auto start_unique = std::chrono::steady_clock::now();
    unique_a.swap(unique_b);
    auto end_unique = std::chrono::steady_clock::now();
    auto time_unique = std::chrono::duration_cast<std::chrono::microseconds>(end_unique - start_unique).count();

    if (!ArrayValuesAreCorrect(unique_a, N, N) || !ArrayValuesAreCorrect(unique_b, N)) {
        std::cerr << "[ОШИБКА] UniquePtr<int[]>: swap\n";
    }

    SharedPtr<int[]> shared_a(new int[N]);
    SharedPtr<int[]> shared_b(new int[N]);
    FillArray(shared_a, N);
    FillArray(shared_b, N, N);

    auto start_shared = std::chrono::steady_clock::now();
    shared_a.swap(shared_b);
    auto end_shared = std::chrono::steady_clock::now();
    auto time_shared = std::chrono::duration_cast<std::chrono::microseconds>(end_shared - start_shared).count();

    if (!ArrayValuesAreCorrect(shared_a, N, N) || !ArrayValuesAreCorrect(shared_b, N) ||
        shared_a.use_count() != 1 || shared_b.use_count() != 1) {
        std::cerr << "[ОШИБКА] SharedPtr<int[]>: swap\n";
    }

    PrintComparisonTable("Array swap", N,
        MakeMetrics(time_raw, 2 * sizeof(int*) + 2 * N * sizeof(int), 2),
        MakeMetrics(time_unique, 2 * sizeof(UniquePtr<int[]>) + 2 * N * sizeof(int), 2),
        MakeMetrics(time_shared, 2 * sizeof(SharedPtr<int[]>) + 2 * N * sizeof(int) + 2 * sizeof(int), 4));
}

void RunAllArrayPointerTests(size_t N) {
    TestArrayCreation(N);
    TestArrayDeletion(N);
    TestArrayAccess(N);
    TestArrayMove(N);
    TestArrayCopy(N);
    TestArrayMultipleOwnership(N);
    TestArrayReset(N);
    TestArraySwap(N);
}
