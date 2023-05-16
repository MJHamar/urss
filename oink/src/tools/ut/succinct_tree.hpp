/**
 * @file succinct_tree.hpp
 * @brief Representaton of a tree node as a tuple of binary strings.
 * encoded on a uint64_t array.
 * 
 * The binary strings are encoded as follows:
 * ZERO -> 01   (binary zero)
 * ONE -> 11    (binary one)
 * SEP -> 10    (separator)
 * EON -> 00    (end of node)
 */
#include <cstdint>
#include <cstring>
#include <string>
#include <stdexcept>
#include <iostream>

#define EON 0
#define ZERO 1
#define SEP 2
#define ONE 3

class succinct_tree
{
public:
    /**
     * @brief Construct a new succinct tree object
    */
    succinct_tree() : __bits_len(1), _bits(new uint64_t[1])
    {
        // initialize all bits to 0
        memset(_bits, 0, sizeof(uint64_t));
    }
    /**
     * @brief Construct a new succinct tree object
     * 
     * @param size the size of the tree
     */
    succinct_tree(int size) : __bits_len(size), _bits(new uint64_t[size])
    {
        // initialize all bits to 0
        memset(_bits, 0, size * sizeof(uint64_t));
    }
    /**
     * @brief Construct a new succinct tree object
     * 
     * @param size the size of the tree
     * @param bits the bits to be copied
     */
    succinct_tree(int size, uint64_t *bits) : __bits_len(size), _bits(new uint64_t[size]){
        memcpy(this->_bits, bits, size * sizeof(uint64_t));
    }
    /** 
     * @brief Copy constructor
    */
    succinct_tree(const succinct_tree &other) : __bits_len(other.size()), _bits(new uint64_t[other.size()]){
        memcpy(_bits, other._bits, other.size() * sizeof(uint64_t));
    }
    /**
     * @brief Desctruct the succinct tree object
    */
    ~succinct_tree()
    {
        delete[] _bits;
    }
    /**
     * @brief append a SEP and a ZERO to the end of the bits array.
     * move overflown bits to the next uint64_t in the array, 
     * if size is a multiple of 64.
     */
    void first_child();
    /**
     * @brief increment the last binary string in the bits array.
     */
    void next_sibling();
    /**
     * @brief pop the end of the bits array.
     * until a SEP is found.
     */
    void parent();

    /**
     * @brief return a string representation of the bits array.
     * 
     * @param decimal if true, return a decimal representation.
     * otherwise, return a binary representation.
    */
    std::string to_string(bool decimal = true) const;

    /**
     * @brief return the height of the tree.
    */
    int size() const;
    /**
     * @brief return the height of the tree.
     * i.e. the number of bit pairs until EON.
    */
    int height() const;
private:
    /**
     * @brief the height of the tree.
    */
    int __bits_len;
    uint64_t *_bits;
    
    /**
     * @brief reallocate the _bits array with size incremented by 1.
    */
    void resize();

    /**
     *  @brief Push two bits to the beginning of the bits array.
    */
    void push_two(uint64_t val = EON);

    /**
     * @brief Pop two bits from the beginning of the bits array.
     * @return the two bits as a uint64_t.
    */
    uint64_t pop_two();

};

/**
 * @brief convert given decimal value (0 or 1) to binary value (ZERO or ONE)
*/
int enctob(int val)
{
    switch (val)
    {
    case ZERO:
        // 01 -> 0
        return 0;
    case ONE:
        // 11 -> 1
        return 1;
    default: // error
        throw new std::logic_error("Illegal character");
    }
}

/**
 * @brief convert given binary value (ZERO or ONE) to decimal value (0 or 1)
*/
int btoenc(int val)
{
    switch (val)
    {
    case 0:
        return ZERO;
    case 1:
        return ONE;
    default:
        throw new std::logic_error("given number is non-binary!");
    }
}