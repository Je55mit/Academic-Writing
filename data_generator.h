#ifndef DATA_GENERATOR_H
#define DATA_GENERATOR_H

#include <vector>
#include <cstdlib>
#include <algorithm>

enum DataType { RANDOM, SORTED, ALMOST_SORTED, FLAT, REVERSE_SORTED };

template<typename T>
std::vector<T> generateData(int size, DataType type) {
    std::vector<T> data(size);
    if (type == RANDOM) {
        for (auto& val : data) {
            val = static_cast<T>(rand() % 1000);
        }
    } else if (type == SORTED) {
        for (int i = 0; i < size; ++i) {
            data[i] = static_cast<T>(i);
        }
    } else if (type == REVERSE_SORTED) {
        for (int i = 0; i < size; ++i) {
            data[i] = static_cast<T>(size - i - 1);
        }
    } else if (type == ALMOST_SORTED) {
        for (int i = 0; i < size; ++i) {
            data[i] = static_cast<T>(i);
        }
        // Introduce some disorder
        int swaps = size / 10;
        for (int i = 0; i < swaps; ++i) {
            int a = rand() % size;
            int b = rand() % size;
            std::swap(data[a], data[b]);
        }
    } else if (type == FLAT) {
        T value = static_cast<T>(42);
        for (auto& val : data) {
            val = value;
        }
    }
    return data;
}

#endif // DATA_GENERATOR_H