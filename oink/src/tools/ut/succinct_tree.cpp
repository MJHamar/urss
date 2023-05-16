#include "succinct_tree.hpp"
#include <iostream>
#include <string>
#include <vector>

void succinct_tree::first_child()
{
    // we need to have space for a SEP and a ZERO
}

void succinct_tree::next_sibling()
{
    // we need to have space for a ONE
}

void succinct_tree::push_left(int n)
{
    
}

std::string bit_array_to_string(uint64_t *bits, int size)
{
    std::cout << "size: " << size << std::endl;
    std::string result;
    for (int i = 0; i < size; i++)
    {
        uint64_t bit = (bits[i / 64] >> i) & 1;
        if (bit == 0)
        {
            result += "0";
        }
        else
        {
            result += "1";
        }
        if (i%4 == 3)
        {
            result += " ";
        }
        if (i%64 == 63)
        {
            result += "\n";
        }
    }
    return result;
}

std::string enctobin(uint64_t *bits, int size)
{
    std::string result;
    for (int i = 0; i < size; i+=2)
    {
        uint64_t bit = (bits[i / 64] >> i) & 3;
        if (bit == ZERO)
        {
            result += "0";
        }
        else if (bit == SEP)
        {
            result += ",";
        }
        else if (bit == ONE)
        {
            result += "1";
        }
        else
        {
            break;
        }
        if (i%4 == 3)
        {
            result += " ";
        }
        if (i%64 == 63)
        {
            result += "\n";
        }
    }
    return result;
}

int main(int argc, const char** argv) {
    // try to figure out how bit arrays work.
    uint64_t *bits = new uint64_t[2];
    memset(bits, 0, 2*sizeof(uint64_t));
    std::cout << bit_array_to_string(bits, 2*sizeof(uint64_t)*8) << std::endl;
    bits[0] = (bits[0] << 2) | SEP;
    std::cout << bit_array_to_string(bits, 2*sizeof(uint64_t)*8) << std::endl;
    std::cout << enctobin(bits, 2*sizeof(uint64_t)*8) << std::endl;
    bits[0] = (bits[0] << 2) | ZERO;
    std::cout << bit_array_to_string(bits, 2*sizeof(uint64_t)*8) << std::endl;
    std::cout << enctobin(bits, 2*sizeof(uint64_t)*8) << std::endl;
    bits[0] = (bits[0] << 2) | ONE;
    std::cout << bit_array_to_string(bits, 2*sizeof(uint64_t)*8) << std::endl;
    std::cout << enctobin(bits, 2*sizeof(uint64_t)*8) << std::endl;
    uint64_t bit = bits[0] >> 2;
    std::cout << "poped bit:" << bit << std::endl;
    std::cout << bit_array_to_string(&bit, sizeof(uint64_t)*8) << std::endl;
    std::cout << enctobin(&bit, sizeof(uint64_t)*8) << std::endl;
    bits[0] >>= 2;
    std::cout << bit_array_to_string(bits, 2*sizeof(uint64_t)*8) << std::endl;
    std::cout << enctobin(bits, 2*sizeof(uint64_t)*8) << std::endl;
    return 0;

}