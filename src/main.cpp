#include "aco.h"         // Inclui a declaração da classe ACO (Ant Colony Optimization)
#include "utils.h"       // Inclui funções utilitárias, como a de leitura do arquivo da instância
#include <iostream>      // Para operações de entrada e saída (como imprimir na tela)
#include <vector>        // Para usar o contêiner std::vector (arrays dinâmicos)
#include <numeric>       // Para funções numéricas como std::accumulate (para somar elementos de um vetor)
#include <iomanip>       // Para manipular a formatação da saída (como std::fixed e std::setprecision)
#include <cmath>         // Para funções matemáticas como std::pow e std::sqrt
#include <algorithm>     // Para algoritmos como std::max_element e std::min_element
#include <chrono>        // Para medir o tempo de execução
#include <random>        // Para std::random_device e std::mt19937
#include <fstream>       // Para escrita em arquivos (para salvar resultados)

int main() {
    // --- 1. Leitura da Instância do Problema da Mochila ---
    // A função readKnapsackInstance retorna um par: a capacidade da mochila (int)
    // e um vetor de itens (std::vector<Item>), onde cada Item tem id, valor e peso.
    std::string instanceFilePath = "data/knapsack-instance.txt";
    std::pair<int, std::vector<Item>> knapsackData = readKnapsackInstance(instanceFilePath);
    int capacity = knapsackData.first;   // Extrai a capacidade da mochila do par retornado
    std::vector<Item> items = knapsackData.second; // Extrai o vetor de itens do par retornado

    // --- 2. Verificação de Robustez da Leitura ---
    if (items.empty()) {
        std::cerr << "Erro ao ler a instância do problema '" << instanceFilePath << "'. Verifique o arquivo e o formato. Encerrando." << std::endl;
        return 1; // Retorna um código de erro para indicar que o programa não terminou normalmente
    }
    if (capacity <= 0) {
        std::cerr << "Erro: Capacidade da mochila inválida ou zero (" << capacity << "). Encerrando." << std::endl;
        return 1;
    }
    // O problema possui 100 itens [cite: 13]
    if (items.size() != 100) {
        std::cerr << "Atenção: O número de itens lidos (" << items.size() << ") não corresponde ao esperado (100)." << std::endl;
    }

    // --- 3. Definição dos Parâmetros do Algoritmo ACO ---
    // Valores ajustados para tentar melhorar a otimização
    int numAnts = 50;              // Número de formigas na colônia. [cite: 5]
    double evaporationRate = 0.3;   // Taxa de evaporação do feromônio (reduzida para persistência maior)
    double alpha = 1.5;            // Importância relativa do feromônio (aumentada para dar mais peso ao feromônio)
    double beta = 2.5;             // Importância relativa da heurística (ligeiramente aumentada, mas menos que alpha)

    // O algoritmo deve realizar 20.000 verificações da função objetivo [cite: 4]
    // Se cada formiga constrói uma solução por iteração, então:
    int maxIterations = 20000 / numAnts; // Número máximo de iterações do algoritmo

    int numExecutions = 15; // O grupo deve realizar 20 execuções separadamente [cite: 7]

    // --- 4. Armazenamento de Resultados para Análise ---
    std::vector<int> bestValues;
    std::vector<std::vector<int>> bestSolutions; // Armazena a solução binária para cada 'bestValue'
    std::vector<double> executionTimes; // Armazena o tempo de execução de cada rodada
    std::vector<unsigned int> seedsUsed; // Armazena o seed usado para cada rodada

    // Gerador de sementes para reprodução (usa random_device para um seed "verdadeiramente" aleatório inicial)
    std::random_device rd;
    std::mt19937 initial_seed_rng(rd()); // Gerador para obter seeds para cada execução

    // --- 5. Impressão dos Parâmetros da Simulação ---
    std::cout << "--- Parametros da Simulação ---" << std::endl;
    std::cout << "Instancia do Problema: " << instanceFilePath << std::endl;
    std::cout << "Capacidade da Mochila: " << capacity << std::endl;
    std::cout << "Numero de Itens: " << items.size() << std::endl;
    std::cout << "Numero de Formigas: " << numAnts << std::endl;
    std::cout << "Taxa de Evaporacao: " << evaporationRate << std::endl;
    std::cout << "Alpha (Importancia do Feromonio): " << alpha << std::endl;
    std::cout << "Beta (Importancia da Heuristica): " << beta << std::endl;
    std::cout << "Maximo de Iteracoes por Execucao: " << maxIterations << std::endl;
    std::cout << "Numero Total de Avaliacoes da Função Objetivo por Execução: " << numAnts * maxIterations << std::endl;
    std::cout << "Numero Total de Execucoes: " << numExecutions << std::endl;
    std::cout << "---------------------------------" << std::endl;

    // --- 6. Loop de Múltiplas Execuções ---
    for (int exec = 0; exec < numExecutions; ++exec) {
        unsigned int currentSeed = initial_seed_rng(); // Obtém um novo seed para esta execução
        seedsUsed.push_back(currentSeed); // Armazena o seed

        // Mede o tempo de execução de cada instância do ACO
        auto start_time = std::chrono::high_resolution_clock::now();

        // Cria uma instância da classe ACO para esta execução, passando o seed
        ACO aco(numAnts, evaporationRate, alpha, beta, capacity, items, maxIterations, currentSeed);

        // Executa o algoritmo ACO e obtém a melhor solução e seu valor
        std::pair<std::vector<int>, int> solveResult = aco.solve();
        std::vector<int> solution = solveResult.first;
        int value = solveResult.second;

        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end_time - start_time;
        double currentExecutionTime = elapsed.count(); // Tempo em segundos

        // Armazena os resultados desta execução
        bestValues.push_back(value);
        bestSolutions.push_back(solution);
        executionTimes.push_back(currentExecutionTime);

        // Imprime o resumo da execução atual
        std::cout << "Execucao " << std::setw(2) << exec + 1
                  << ": Melhor valor = " << std::setw(6) << value
                  << ", Tempo = " << std::fixed << std::setprecision(4) << currentExecutionTime << "s"
                  << ", Seed = " << currentSeed << std::endl;
    }

    // --- 7. Cálculo das Métricas de Desempenho Finais ---
    // Calcular média, desvio padrão, melhor e pior valor obtido [cite: 7, 8]
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

    // --- 8. Impressão dos Resultados Finais Consolidados ---
    std::cout << "\n--- Resultados Finais Consolidados ---" << std::endl;
    std::cout << "Media do melhor valor: " << std::fixed << std::setprecision(2) << meanValue << std::endl;
    std::cout << "Desvio padrao do valor: " << std::fixed << std::setprecision(2) << std_dev_values << std::endl;
    std::cout << "Melhor valor GLOBAL obtido: " << bestOfAll << std::endl;
    std::cout << "Pior valor GLOBAL obtido: " << worstOfAll << std::endl;
    std::cout << "---------------------------------------" << std::endl;
    std::cout << "Media do tempo de execuçao: " << std::fixed << std::setprecision(4) << meanTime << "s" << std::endl;
    std::cout << "Desvio padrao do tempo: " << std::fixed << std::setprecision(4) << std_dev_times << "s" << std::endl;
    std::cout << "---------------------------------------" << std::endl;

    // --- 9. Detalhes da Melhor Solução Geral Encontrada ---
    std::cout << "\n--- Detalhes da Melhor Solucao Geral (entre todas as " << numExecutions << " execucoes) ---" << std::endl;

    // Encontra o índice da primeira ocorrência do melhor valor geral
    int overallBestSolutionIndex = -1;
    for (size_t i = 0; i < bestValues.size(); ++i) {
        if (bestValues[i] == bestOfAll) {
            overallBestSolutionIndex = i;
            break; // Encontrou a primeira ocorrência
        }
    }

    if (overallBestSolutionIndex != -1) {
        const std::vector<int>& overallBestSolution = bestSolutions[overallBestSolutionIndex];
        int overallBestWeight = 0;
        int overallBestValueCalculated = 0;

        std::cout << "Valor da Solucao: " << bestOfAll << std::endl;
        std::cout << "Itens Incluidos (ID - Valor, Peso):" << std::endl;
        for (size_t i = 0; i < overallBestSolution.size(); ++i) {
            if (overallBestSolution[i] == 1) { // Se o item está na mochila
                std::cout << "- Item " << items[i].id << " (Valor: " << items[i].value << ", Peso: " << items[i].weight << ")" << std::endl;
                overallBestWeight += items[i].weight;
                overallBestValueCalculated += items[i].value;
            }
        }
        std::cout << "Peso Total da Solucao: " << overallBestWeight << " / " << capacity << std::endl;

        // Verificação final de viabilidade e consistência
        if (overallBestWeight > capacity) {
            std::cerr << "ATENÇÃO: A melhor solução geral encontrada é INVIÁVEL (Peso excede capacidade)!" << std::endl;
        }
        if (overallBestValueCalculated != bestOfAll) {
            std::cerr << "ATENÇÃO: Inconsistencia no valor calculado da melhor solucao geral. Esperado: " << bestOfAll << ", Calculado: " << overallBestValueCalculated << std::endl;
        }
        std::cout << "Seed da Execucao que gerou esta solucao: " << seedsUsed[overallBestSolutionIndex] << std::endl;
        std::cout << "Tempo da Execucao que gerou esta solucao: " << std::fixed << std::setprecision(4) << executionTimes[overallBestSolutionIndex] << "s" << std::endl;

    } else {
        std::cerr << "ATENÇÃO: Não foi possível encontrar a melhor solução geral armazenada." << std::endl;
    }

    return 0; // Indica que o programa terminou com sucesso
}