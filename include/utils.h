#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

struct Item {
    int id;
    int value;
    int weight;
};

std::pair<int, std::vector<Item>> readKnapsackInstance(const std::string& filename);

#endif // UTILS_H