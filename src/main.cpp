#include "Tests.hpp"

#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>

/* CRT Debug Heap доступен только в MSVC */
#ifdef _MSC_VER
/* Учёт утечек включается только в Debug-сборке и не влияет на обычные замеры и ASan */
#ifdef _DEBUG
#include <crtdbg.h>
#endif
#endif

/* Очистка экрана для консоли */
static void ClearScreen(){
    std::cout.flush();
    std::system("cls");
}

/* Сброс остатка ввода до конца строки */
static void ClearInputBuffer(){
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

/* Ожидание enter */
static void WaitingForEnter(){
    std::cout << "\nНажмите ENTER для продолжения...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    ClearScreen();
}

int main() {
#ifdef _MSC_VER
#ifdef _DEBUG
    /*
    _CRTDBG_REPORT_FLAG - получить текущие настройки CRT.
    _CRTDBG_ALLOC_MEM_DF - учитывать динамические выделения.
    _CRTDBG_LEAK_CHECK_DF - автоматически проверить утечки при завершении.
    _CrtSetDbgFlag() - применить эти настройки.
    */
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) |
                   _CRTDBG_ALLOC_MEM_DF |
                   _CRTDBG_LEAK_CHECK_DF);
#endif
#endif

    ClearScreen();

    while (true) {
        std::cout << "Главное меню:\n"
                  << "1) Работа с одиночными указателями\n"
                  << "2) Работа с указателем на массив\n"
                  << "0) Выход\n"
                  << "Выберите пункт: ";

        int main_choice;
        if (!(std::cin >> main_choice)) {
            ClearInputBuffer();
            std::cout << "Неверный ввод.\n";
            WaitingForEnter();
            continue;
        }
        ClearInputBuffer();

        switch (main_choice) {
        case 0:
            return 0;

        case 1: {
            ClearScreen();

            while (true) {
                std::cout << "Одиночные указатели:\n"
                          << "1) Тест создания\n"
                          << "2) Тест удаления\n"
                          << "3) Тест доступа к данным\n"
                          << "4) Тест перемещения\n"
                          << "5) Тест копирования\n"
                          << "6) Тест множественного владения\n"
                          << "7) Тест reset\n"
                          << "8) Тест swap\n"
                          << "9) Полный набор тестов\n"
                          << "0) Назад\n"
                          << "Выберите пункт: ";

                int test_choice;
                if (!(std::cin >> test_choice)) {
                    ClearInputBuffer();
                    std::cout << "Неверный ввод.\n";
                    WaitingForEnter();
                    continue;
                }
                ClearInputBuffer();

                if (test_choice == 0) {
                    ClearScreen();
                    break;
                }

                if (test_choice < 1 || test_choice > 9) {
                    std::cout << "Неверный пункт меню.\n";
                    WaitingForEnter();
                    continue;
                }

                size_t N = 0;
                while (N == 0) {
                    std::cout << "Введите количество объектов N: ";
                    long input_N;
                    if (!(std::cin >> input_N) || input_N <= 0) {
                        ClearInputBuffer();
                        std::cout << "N должно быть больше нуля.\n";
                    } else {
                        N = static_cast<size_t>(input_N);
                        ClearInputBuffer();
                    }
                }

                ClearScreen();
                try {
                    switch (test_choice) {
                    case 1:
                        std::cout << "Тест создания\n";
                        TestCreation(N);
                        break;
                    case 2:
                        std::cout << "Тест удаления\n";
                        TestDeletion(N);
                        break;
                    case 3:
                        std::cout << "Тест доступа к данным\n";
                        TestAccess(N);
                        break;
                    case 4:
                        std::cout << "Тест перемещения\n";
                        TestMove(N);
                        break;
                    case 5:
                        std::cout << "Тест копирования\n";
                        TestCopy(N);
                        break;
                    case 6:
                        std::cout << "Тест множественного владения\n";
                        TestMultipleOwnership(N);
                        break;
                    case 7:
                        std::cout << "Тест reset\n";
                        TestReset(N);
                        break;
                    case 8:
                        std::cout << "Тест swap\n";
                        TestSwap(N);
                        break;
                    case 9:
                        std::cout << "Полный набор тестов\n";
                        RunAllOnePointersTests(N);
                        break;
                    }
                } catch (const std::bad_alloc&) {
                    std::cerr << "[ОШИБКА] Недостаточно памяти для N = " << N << '\n';
                }

                WaitingForEnter();
            }
            break;
        }

        case 2: {
            ClearScreen();

            while (true) {
                std::cout << "Указатель на массив:\n"
                          << "1) Тест создания\n"
                          << "2) Тест удаления\n"
                          << "3) Тест доступа к данным\n"
                          << "4) Тест перемещения\n"
                          << "5) Тест копирования\n"
                          << "6) Тест множественного владения\n"
                          << "7) Тест reset\n"
                          << "8) Тест swap\n"
                          << "9) Полный набор тестов\n"
                          << "0) Назад\n"
                          << "Выберите пункт: ";

                int test_choice;
                if (!(std::cin >> test_choice)) {
                    ClearInputBuffer();
                    std::cout << "Неверный ввод.\n";
                    WaitingForEnter();
                    continue;
                }
                ClearInputBuffer();

                if (test_choice == 0) {
                    ClearScreen();
                    break;
                }

                if (test_choice < 1 || test_choice > 9) {
                    std::cout << "Неверный пункт меню.\n";
                    WaitingForEnter();
                    continue;
                }

                size_t N = 0;
                while (N == 0) {
                    std::cout << "Введите размер массива N: ";
                    long input_N;
                    if (!(std::cin >> input_N) || input_N <= 0) {
                        ClearInputBuffer();
                        std::cout << "N должен быть больше нуля.\n";
                    } else {
                        N = static_cast<size_t>(input_N);
                        ClearInputBuffer();
                    }
                }

                ClearScreen();
                try {
                    switch (test_choice) {
                    case 1:
                        std::cout << "Тест создания массива\n";
                        TestArrayCreation(N);
                        break;
                    case 2:
                        std::cout << "Тест удаления массива\n";
                        TestArrayDeletion(N);
                        break;
                    case 3:
                        std::cout << "Тест доступа к элементам массива\n";
                        TestArrayAccess(N);
                        break;
                    case 4:
                        std::cout << "Тест перемещения массива\n";
                        TestArrayMove(N);
                        break;
                    case 5:
                        std::cout << "Тест копирования массива\n";
                        TestArrayCopy(N);
                        break;
                    case 6:
                        std::cout << "Тест множественного владения массивом\n";
                        TestArrayMultipleOwnership(N);
                        break;
                    case 7:
                        std::cout << "Тест reset для массива\n";
                        TestArrayReset(N);
                        break;
                    case 8:
                        std::cout << "Тест swap для массива\n";
                        TestArraySwap(N);
                        break;
                    case 9:
                        std::cout << "Полный набор тестов указателей на массив\n";
                        RunAllArrayPointerTests(N);
                        break;
                    }
                } catch (const std::bad_alloc&) {
                    std::cerr << "[ОШИБКА] Недостаточно памяти для N = " << N << '\n';
                }

                WaitingForEnter();
            }
            break;
        }

        default:
            std::cout << "Неверный пункт меню.\n";
            WaitingForEnter();
            break;
        }
    }
}
