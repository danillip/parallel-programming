# Лабораторные работы

## Лабораторная работа 1: Быстрая сортировка

### Задание
1. Реализовать алгоритм быстрой сортировки.
2. Написать многопоточную версию алгоритма.
3. Провести сравнение времени выполнения:
   - На малом массиве (< 100 элементов).
   - На крупном массиве (> 10,000 элементов).
4. Сделать выводы о производительности.

---

## Лабораторная работа 2: Анализ функции и численный интеграл

### Задание
Для функции:

f(x) =
- 0.1 * x, если x < -20
- 0.5 * sin(0.25 * x) + 2.2 * cos(0.01 * x), если -20 ≤ x < -5
- x^5 - x^4 + x^2 - x + 1, если x ≥ -5

на промежутке \[A, B\], где A = -25, B = 5, требуется:

1. Найти приближенные минимальное и максимальное значения.
2. Вычислить приближенное значение интеграла численным методом (например, методом прямоугольников, трапеций и т. д.).

### Условия
- Разбить отрезок на N подотрезков.
- Запустить N потоков.
- Для каждого потока вычислить локальные:
  - минимум,
  - максимум,
  - интеграл.
- Глобальные минимум, максимум и интеграл изменять внутри потоков.

---

## Лабораторные работы 3 и 4: Реализация Thread Pool

### Задание 3
Реализовать программу, использующую паттерн **Thread Pool**.  
#### Требования:
1. Один объект управляет задачами и потоками.
2. Задачи реализовать в виде функций или классов.
3. **Thread Pool**:
   - Принимает задачи, помещает их в очередь.
   - Выделяет потоки для выполнения задач при их наличии.
4. При старте программы запускаются все доступные потоки (в зависимости от числа доступных процессоров).
5. Потоки:
   - Выполняют задачи, после завершения сообщают пулу о своей готовности.
6. Рекомендуется использовать `std::mutex`/`std::lock_guard`.
7. Управление очередью задач можно перенести внутрь потоков, чтобы они сами забирали задачи до полного завершения очереди.

---

### Задание 4
Реализовать **Thread Pool**, но управление потоками выполнить через **WinAPI** вместо стандартной библиотеки C++.

---

## Выводы
Для каждой лабораторной работы необходимо сделать выводы, проанализировав производительность и особенности реализации.
