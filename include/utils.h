#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <string>
#include <utility> // Para std::pair
#include <fstream>
#include <iostream>

struct Item {
    int id;
    int value;
    int weight;
};

// A função de leitura da instância
std::pair<int, std::vector<Item>> readKnapsackInstance(const std::string& filePath);

#endif // UTILS_H