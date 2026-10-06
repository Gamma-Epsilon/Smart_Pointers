#pragma once

#include <cstddef>

/* Тесты одиночных указателей */
void TestCreation(std::size_t N);
void TestDeletion(std::size_t N);
void TestAccess(std::size_t N);
void TestMove(std::size_t N);
void TestCopy(std::size_t N);
void TestMultipleOwnership(std::size_t N);
void TestReset(std::size_t N);
void TestSwap(std::size_t N);
void RunAllOnePointersTests(std::size_t N);

/* Тесты указателей на массив */
void TestArrayCreation(std::size_t N);
void TestArrayDeletion(std::size_t N);
void TestArrayAccess(std::size_t N);
void TestArrayMove(std::size_t N);
void TestArrayCopy(std::size_t N);
void TestArrayMultipleOwnership(std::size_t N);
void TestArrayReset(std::size_t N);
void TestArraySwap(std::size_t N);
void RunAllArrayPointerTests(std::size_t N);
