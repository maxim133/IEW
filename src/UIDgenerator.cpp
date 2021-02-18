#include "UIDgenerator.h"
#include <random>
#include <ctime>
#include <algorithm>

namespace UIDgenerator
{

std::string generateUID(uint32_t maxLenght)
{
    std::string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

    std::random_device rd;
    std::mt19937 generator(rd());

    std::shuffle(str.begin(), str.end(), generator);

    return str.substr(0, maxLenght);
}

}