#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <ctime>
#include <functional>
#include <fstream>
#include <cstdio>
#include "data_generator.h"
#include "sorting.h"

struct SortResult {
    std::string algorithm;
    std::string complexity;
    long long timeInMicroseconds;
    bool skipped;
    std::string skipReason;
};

std::string toJson(const SortResult& result) {
    std::string json = "      {\n";
    json += "        \"algorithm\": \"" + result.algorithm + "\",\n";
    json += "        \"complexity\": \"" + result.complexity + "\",\n";
    if (result.skipped) {
        json += "        \"timeInSeconds\": null,\n";
        json += "        \"skipReason\": \"" + result.skipReason + "\",\n";
        json += "        \"skipped\": true\n";
    } else {
        double timeInSeconds = result.timeInMicroseconds / 1000000.0;
        // Format with reasonable precision
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "%.6f", timeInSeconds);
        json += "        \"timeInSeconds\": " + std::string(buffer) + ",\n";
        json += "        \"skipped\": false\n";
    }
    json += "      }";
    return json;
}

int main() {
    std::srand(std::time(0));

    std::vector<int> sizes = {30, 75, 100, 1000, 10000, 100000, 1000000, 10000000};
    std::vector<DataType> types = {RANDOM, SORTED, ALMOST_SORTED, FLAT, REVERSE_SORTED};
    std::vector<std::string> typeNames = {"RANDOM", "SORTED", "ALMOST_SORTED", "FLAT", "REVERSE_SORTED"};

struct SortAlgorithm {
    std::string name;
    std::string complexity;
    std::function<void(std::vector<int>&)> func;
};

    // Define sorting functions for int with complexity
    std::vector<SortAlgorithm> intSorts;
    intSorts.push_back({"Bubble Sort", "O(n²)", static_cast<std::function<void(std::vector<int>&)>>(bubbleSort<int>)});
    intSorts.push_back({"Insertion Sort", "O(n²)", static_cast<std::function<void(std::vector<int>&)>>(insertionSort<int>)});
    intSorts.push_back({"Selection Sort", "O(n²)", static_cast<std::function<void(std::vector<int>&)>>(selectionSort<int>)});
    intSorts.push_back({"Merge Sort", "O(n log n)", [](std::vector<int>& arr){ mergeSort(arr); }});
    intSorts.push_back({"Quick Sort", "O(n log n)", [](std::vector<int>& arr){ quickSort(arr); }});
    intSorts.push_back({"Heap Sort", "O(n log n)", static_cast<std::function<void(std::vector<int>&)>>(heapSort<int>)});
    intSorts.push_back({"Timsort", "O(n log n)", static_cast<std::function<void(std::vector<int>&)>>(timSort<int>)});

    // For each dataset type, create a separate JSON file
    for (size_t typeIdx = 0; typeIdx < types.size(); ++typeIdx) {
        std::string typeName = typeNames[typeIdx];
        std::string filename = "C:\\Users\\Adi\\Desktop\\VSCode C++ Projects\\Projects\\sorting_results_" + typeName + ".json";
        
        std::ofstream jsonFile(filename);
        jsonFile << "{\n";
        jsonFile << "  \"datasetType\": \"" << typeName << "\",\n";
        jsonFile << "  \"tests\": [\n";
        
        std::vector<std::string> testResults;
        
        std::cout << "Processing " << typeName << " dataset..." << std::endl;
        
        for (size_t sizeIdx = 0; sizeIdx < sizes.size(); ++sizeIdx) {
            int size = sizes[sizeIdx];
            auto originalData = generateData<int>(size, types[typeIdx]);
            
            std::cout << "  Size: " << size << std::endl;
            
            std::string testJson = "    {\n";
            testJson += "      \"size\": " + std::to_string(size) + ",\n";
            testJson += "      \"results\": [\n";
            
            std::vector<std::string> results;
            
            for (size_t algIdx = 0; algIdx < intSorts.size(); ++algIdx) {
                auto& algo = intSorts[algIdx];
                SortResult result;
                result.algorithm = algo.name;
                result.complexity = algo.complexity;
                result.skipped = false;
                result.skipReason = "";
                
                // Skip O(n²) algorithms for sizes >= 100000
                if ((algo.name == "Bubble Sort" || algo.name == "Insertion Sort" || algo.name == "Selection Sort") && size >= 100000) {
                    result.skipped = true;
                    result.skipReason = "Would take too long - O(n²) complexity on large dataset";
                    result.timeInMicroseconds = 0;
                    std::cout << "    " << algo.name << ": Skipped (O(n²) - would take too long)" << std::endl;
                } else if (algo.name == "Quick Sort" && size >= 100000) {
                    result.skipped = true;
                    result.skipReason = "too long";
                    result.timeInMicroseconds = 0;
                    std::cout << "    " << algo.name << ": Skipped (too long)" << std::endl;
                } else {
                    auto data = originalData; // copy
                    auto start = std::chrono::high_resolution_clock::now();
                    auto timeout = std::chrono::seconds(10);
                    
                    // For problematic cases, use a shorter timeout
                    if ((algo.name == "Quick Sort") && (typeName == "SORTED" || typeName == "REVERSE_SORTED") && size >= 100000) {
                        timeout = std::chrono::seconds(5);
                    }
                    
                    algo.func(data);
                    auto end = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                    
                    // Check if it took too long
                    if (duration > timeout) {
                        result.skipped = true;
                        result.skipReason = "Test exceeded time limit";
                        result.timeInMicroseconds = 0;
                        std::cout << "    " << algo.name << ": Exceeded time limit (>10s)" << std::endl;
                    } else {
                        result.timeInMicroseconds = duration.count();
                        double timeInSeconds = result.timeInMicroseconds / 1000000.0;
                        std::cout << "    " << algo.name << ": " << timeInSeconds << " s" << std::endl;
                    }
                }
                
                results.push_back(toJson(result));
            }
            
            // Join results with commas
            for (size_t i = 0; i < results.size(); ++i) {
                testJson += results[i];
                if (i < results.size() - 1) {
                    testJson += ",";
                }
                testJson += "\n";
            }
            
            testJson += "      ]\n";
            testJson += "    }";
            
            testResults.push_back(testJson);
        }
        
        // Join all tests with commas
        for (size_t i = 0; i < testResults.size(); ++i) {
            jsonFile << testResults[i];
            if (i < testResults.size() - 1) {
                jsonFile << ",";
            }
            jsonFile << "\n";
        }
        
        jsonFile << "  ]\n";
        jsonFile << "}\n";
        jsonFile.close();
        
        std::cout << "Results saved to " << filename << std::endl << std::endl;
    }
    return 0;
}