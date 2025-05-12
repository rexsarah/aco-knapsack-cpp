#include "aco.h"
#include "utils.h"
#include <iostream>
#include <vector>
#include <numeric>
#include <iomanip>
#include <cmath>
#include <algorithm>

int main() {
    std::pair<int, std::vector<Item>> knapsackData = readKnapsackInstance("data/knapsack-instance.txt");
    int capacity = knapsackData.first;
    std::vector<Item> items = knapsackData.second;

    if (items.empty()) {
        return 1;
    }

    int numAnts = 50;
    double evaporationRate = 0.5;
    double alpha = 1.0;
    double beta = 2.0;
    int maxIterations = 20000 / numAnts; // Adjust iterations based on total evaluations

    std::vector<int> bestValues;
    std::vector<std::vector<int>> bestSolutions;

    int numExecutions = 20;
    for (int exec = 0; exec < numExecutions; ++exec) {
        ACO aco(numAnts, evaporationRate, alpha, beta, capacity, items, maxIterations);
        std::pair<std::vector<int>, int> solveResult = aco.solve();
        std::vector<int> solution = solveResult.first;
        int value = solveResult.second;
        bestValues.push_back(value);
        bestSolutions.push_back(solution);
        std::cout << "Execução " << exec + 1 << ": Melhor valor encontrado = " << value << std::endl;
    }

    // Calculate metrics
    double sum = std::accumulate(bestValues.begin(), bestValues.end(), 0.0);
    double mean = sum / numExecutions;

    double sq_sum = 0.0;
    for (int val : bestValues) {
        sq_sum += std::pow(val - mean, 2);
    }
    double std_dev = std::sqrt(sq_sum / numExecutions);

    int bestOfAll = *std::max_element(bestValues.begin(), bestValues.end());
    int worstOfAll = *std::min_element(bestValues.begin(), bestValues.end());

    std::cout << "\n--- Resultados Finais ---" << std::endl;
    std::cout << "Média do melhor valor: " << std::fixed << std::setprecision(2) << mean << std::endl;
    std::cout << "Desvio padrão: " << std::fixed << std::setprecision(2) << std_dev << std::endl;
    std::cout << "Melhor valor obtido em todas as execuções: " << bestOfAll << std::endl;
    std::cout << "Pior valor obtido em todas as execuções: " << worstOfAll << std::endl;

    return 0;
}