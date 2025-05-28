#include <algorithm>
#include <iostream>
#include <vector>
#include <numeric>
#include <cmath>
#include <chrono>
#include <fstream> // Para salvar no CSV
#include "utils.h"
#include "aco.h"

int main() {
    auto [capacity, items] = readKnapsackInstance("data/knapsack-instance.txt");
    if (items.empty()) {
        std::cerr << "Erro: não foi possível ler os itens." << std::endl;
        return 1;
    }

    int numAnts = 50;
    double evaporationRate = 0.3;
    double alpha = 1.5;
    double beta = 2.5;
    int maxIterations = 2000;
    unsigned int seed = 1234;

    std::vector<int> bestValues;
    std::vector<std::vector<int>> bestSolutions;
    std::vector<double> executionTimes;

    std::ofstream csv("resultados.csv");
    if (!csv.is_open()) {
        std::cerr << "Erro ao criar o arquivo resultados.csv" << std::endl;
        return 1;
    }

    // Cabeçalho do CSV
    csv << "Execucao,MelhorValor,TempoSegundos,ItensSelecionados\n";

    auto totalStart = std::chrono::high_resolution_clock::now();

    for (int run = 0; run < 20; ++run) {
        auto start = std::chrono::high_resolution_clock::now();

        ACO aco(numAnts, evaporationRate, alpha, beta, capacity, items, maxIterations, seed + run);
        auto [bestSolution, bestValue] = aco.solve();

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        executionTimes.push_back(duration.count());

        bestValues.push_back(bestValue);
        bestSolutions.push_back(bestSolution);

        std::cout << "Execução " << run + 1 << ": Melhor valor = " << bestValue << " (Tempo: " 
                  << duration.count() << "s)" << std::endl;

        std::cout << "Itens selecionados: ";
        csv << run + 1 << "," << bestValue << "," << duration.count() << ",";

        for (size_t i = 0; i < bestSolution.size(); ++i) {
            if (bestSolution[i] == 1) {
                std::cout << (i + 1) << " ";
                csv << (i + 1) << " ";
            }
        }
        std::cout << "\n" << std::endl;
        csv << "\n";
    }

    auto totalEnd = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> totalDuration = totalEnd - totalStart;

    // Estatísticas
    double mean = std::accumulate(bestValues.begin(), bestValues.end(), 0.0) / bestValues.size();
    double sq_sum = 0;
    for (auto v : bestValues)
        sq_sum += (v - mean) * (v - mean);
    double stddev = std::sqrt(sq_sum / bestValues.size());
    int bestOverall = *std::max_element(bestValues.begin(), bestValues.end());
    int worstOverall = *std::min_element(bestValues.begin(), bestValues.end());

    std::cout << "Resultados após 20 execuções:\n";
    std::cout << "Média: " << mean << std::endl;
    std::cout << "Desvio Padrão: " << stddev << std::endl;
    std::cout << "Melhor valor obtido: " << bestOverall << std::endl;
    std::cout << "Pior valor obtido: " << worstOverall << std::endl;
    std::cout << "\nTempo total de execução: " << totalDuration.count() << " segundos\n";

    csv.close();
    return 0;
}
