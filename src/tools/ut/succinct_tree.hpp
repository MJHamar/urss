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

#define EON 0
#define ZERO 1
#define SEP 2
#define ONE 3

class succinct_tree
{
public:
    /**
     * @brief Construct a new succinct tree object
     * 
     * @param height the height of the tree
     */
    succinct_tree(int height) : bits(new uint64_t[height])
    {
        // initialize all bits to 0
        memset(bits, 0, height * sizeof(uint64_t));
    }
    /**
     * @brief Construct a new succinct tree object
     * 
     * @param height the height of the tree
     * @param bits the bits to be copied
     */
    succinct_tree(int height, int size, uint64_t *bits) : size(size), bits(bits){}
    /**
     * @brief Desctruct the succinct tree object
    */
    ~succinct_tree()
    {
        delete[] bits;
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
    succinct_tree parent();

    /**
     * @brief return a string representation of the bits array.
    */
    std::string to_string() const;

private:
    uint64_t *bits;
    int size;

    /**
     * @brief Push the bits array by n bits to the left.
     * Look out for any overflown bits.
    */
    void push_left(int n);

    /**
     * @brief Push the bits array by n bits to the right.
     * Look out for any overflown bits.
    */
    void push_right(int n);
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