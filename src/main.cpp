#include "aco.h"
#include "utils.h"
#include <iostream>
#include <vector>
#include <numeric>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <random>
#include <fstream>

// Função para salvar o resultado de uma execução no CSV
void saveExecutionResultToCSV(std::ofstream& csvFile, int execNumber, int bestValue, double execTime, unsigned int seed, 
                              const std::vector<int>& solution, const std::vector<Item>& items) {
    csvFile << execNumber << "," << bestValue << ","
            << std::fixed << std::setprecision(4) << execTime << ","
            << seed << ",";

    bool hasItem = false;
    for (size_t i = 0; i < solution.size(); ++i) {
        if (solution[i] == 1) {
            hasItem = true;
            break;
        }
    }

    if (hasItem) {
        csvFile << "\"";
        for (size_t i = 0; i < solution.size(); ++i) {
            if (solution[i] == 1) {
                csvFile << items[i].id << " ";
            }
        }
        csvFile.seekp(-1, std::ios_base::cur); // Remove o último espaço
        csvFile << "\"";
    } else {
        csvFile << "\"Nenhum item selecionado\"";
    }

    csvFile << "\n";
}

int main() {
    std::string instanceFilePath = "data/knapsack-instance.txt";
    std::pair<int, std::vector<Item>> knapsackData = readKnapsackInstance(instanceFilePath);
    int capacity = knapsackData.first;
    std::vector<Item> items = knapsackData.second;

    if (items.empty()) {
        std::cerr << "Erro ao ler a instância do problema '" << instanceFilePath << "'. Verifique o arquivo e o formato. Encerrando." << std::endl;
        return 1;
    }
    if (capacity <= 0) {
        std::cerr << "Erro: Capacidade da mochila inválida ou zero (" << capacity << "). Encerrando." << std::endl;
        return 1;
    }
    if (items.size() != 100) {
        std::cerr << "Atenção: O número de itens lidos (" << items.size() << ") não corresponde ao esperado (100)." << std::endl;
    }

    int numAnts = 50;
    double evaporationRate = 0.3;
    double alpha = 1.5;
    double beta = 2.5;
    int maxIterations = 20000 / numAnts;
    int numExecutions = 15;

    std::vector<int> bestValues;
    std::vector<std::vector<int>> bestSolutions;
    std::vector<double> executionTimes;
    std::vector<unsigned int> seedsUsed;

    std::random_device rd;
    std::mt19937 initial_seed_rng(rd());

    std::cout << "--- Parametros da Simulação ---" << std::endl;
    std::cout << "Instancia do Problema: " << instanceFilePath << std::endl;
    std::cout << "Capacidade da Mochila: " << capacity << std::endl;
    std::cout << "Numero de Itens: " << items.size() << std::endl;
    std::cout << "Numero de Formigas: " << numAnts << std::endl;
    std::cout << "Taxa de Evaporacao: " << evaporationRate << std::endl;
    std::cout << "Alpha (Importancia do Feromonio): " << alpha << std::endl;
    std::cout << "Beta (Importancia da Heuristica): " << beta << std::endl;
    std::cout << "Maximo de Iteracoes por Execucao: " << maxIterations << std::endl;
    std::cout << "Numero Total de Avaliacoes da Função Objetivo por Execucao: " << numAnts * maxIterations << std::endl;
    std::cout << "Numero Total de Execucoes: " << numExecutions << std::endl;
    std::cout << "---------------------------------" << std::endl;

    // Abre arquivo CSV para escrita
    std::ofstream csvFile("resultados_aco.csv");
    if (!csvFile.is_open()) {
        std::cerr << "Erro ao abrir arquivo CSV para escrita." << std::endl;
        return 1;
    }
    csvFile << "Execucao,MelhorValor,Tempo,Seed,ItensIncluidos\n";

    for (int exec = 0; exec < numExecutions; ++exec) {
        unsigned int currentSeed = initial_seed_rng();
        seedsUsed.push_back(currentSeed);

        auto start_time = std::chrono::high_resolution_clock::now();

        ACO aco(numAnts, evaporationRate, alpha, beta, capacity, items, maxIterations, currentSeed);

        std::pair<std::vector<int>, int> solveResult = aco.solve();
        std::vector<int> solution = solveResult.first;
        int value = solveResult.second;

        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end_time - start_time;
        double currentExecutionTime = elapsed.count();

        bestValues.push_back(value);
        bestSolutions.push_back(solution);
        executionTimes.push_back(currentExecutionTime);

        // Imprime no terminal
        std::cout << "Execucao " << std::setw(2) << exec + 1
                  << ": Melhor valor = " << std::setw(6) << value
                  << ", Tempo = " << std::fixed << std::setprecision(4) << currentExecutionTime << "s"
                  << ", Seed = " << currentSeed << std::endl;

        std::cout << "Itens incluídos: ";
        bool temItem = false;
        for (size_t i = 0; i < solution.size(); ++i) {
            if (solution[i] == 1) {
                std::cout << items[i].id << " ";
                temItem = true;
            }
        }
        if (!temItem) {
            std::cout << "Nenhum item selecionado";
        }
        std::cout << std::endl;

        // Salva resultado no CSV
        saveExecutionResultToCSV(csvFile, exec + 1, value, currentExecutionTime, currentSeed, solution, items);
    }

    csvFile.close();

    // --- Cálculo das métricas finais ---

    double sumValues = std::accumulate(bestValues.begin(), bestValues.end(), 0.0);
    double meanValue = sumValues / numExecutions;

    double sq_sum_values = 0.0;
    for (int val : bestValues) {
        sq_sum_values += std::pow(val - meanValue, 2);
    }
    double std_dev_values = std::sqrt(sq_sum_values / numExecutions);

    int bestOfAll = *std::max_element(bestValues.begin(), bestValues.end());
    int worstOfAll = *std::min_element(bestValues.begin(), bestValues.end());

    double sumTimes = std::accumulate(executionTimes.begin(), executionTimes.end(), 0.0);
    double meanTime = sumTimes / numExecutions;

    double sq_sum_times = 0.0;
    for (double time : executionTimes) {
        sq_sum_times += std::pow(time - meanTime, 2);
    }
    double std_dev_times = std::sqrt(sq_sum_times / numExecutions);

    std::cout << "\n--- Resultados Finais Consolidados ---" << std::endl;
    std::cout << "Media do melhor valor: " << std::fixed << std::setprecision(2) << meanValue << std::endl;
    std::cout << "Desvio padrao do valor: " << std::fixed << std::setprecision(2) << std_dev_values << std::endl;
    std::cout << "Melhor valor GLOBAL obtido: " << bestOfAll << std::endl;
    std::cout << "Pior valor GLOBAL obtido: " << worstOfAll << std::endl;
    std::cout << "---------------------------------------" << std::endl;
    std::cout << "Media do tempo de execuçao: " << std::fixed << std::setprecision(4) << meanTime << "s" << std::endl;
    std::cout << "Desvio padrao do tempo: " << std::fixed << std::setprecision(4) << std_dev_times << "s" << std::endl;
    std::cout << "---------------------------------------" << std::endl;

    // --- Detalhes da melhor solução geral ---

    std::cout << "\n--- Detalhes da Melhor Solucao Geral (entre todas as " << numExecutions << " execucoes) ---" << std::endl;

    int overallBestSolutionIndex = -1;
    for (size_t i = 0; i < bestValues.size(); ++i) {
        if (bestValues[i] == bestOfAll) {
            overallBestSolutionIndex = i;
            break;
        }
    }

    if (overallBestSolutionIndex != -1) {
        const std::vector<int>& overallBestSolution = bestSolutions[overallBestSolutionIndex];
        int overallBestWeight = 0;
        int overallBestValueCalculated = 0;

        std::cout << "Valor da Solucao: " << bestOfAll << std::endl;
        std::cout << "Itens Incluidos (ID - Valor, Peso):" << std::endl;
        for (size_t i = 0; i < overallBestSolution.size(); ++i) {
            if (overallBestSolution[i] == 1) {
                std::cout << "- Item " << items[i].id << " (Valor: " << items[i].value << ", Peso: " << items[i].weight << ")" << std::endl;
                overallBestWeight += items[i].weight;
                overallBestValueCalculated += items[i].value;
            }
        }
        std::cout << "Peso total: " << overallBestWeight << std::endl;
        std::cout << "Valor total calculado: " << overallBestValueCalculated << std::endl;
    } else {
        std::cout << "Erro ao encontrar a melhor solução geral." << std::endl;
    }

    return 0;
}
