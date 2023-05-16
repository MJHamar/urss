#include "succinct_tree.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <math.h>

// ------------------ Helper functions ------------------

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

void succinct_tree::push_two(uint64_t val)
{
    uint64_t helper, carry;
    carry = val;
    int index = 0;
    do {   
        // save high end of the current bits
        helper = (_bits[index] >> 62) & 3UL;
        // throw logic error if (next) carry is not zero and index is maximum.
        if (helper != 0 && index == __bits_len-1)
        {
            resize();
        }
        // shift the current bits to the left by two
        _bits[index] = (_bits[index] << 2) | carry;
        // set carry to helper
        carry = helper;
        // increment index
        index++;
    } while (carry != 0);
}

uint64_t succinct_tree::pop_two()
{
    uint64_t helper, carry;
    carry = 0;
    int index = __bits_len-1;
    uint64_t mask = 3;
    do {
        // save low end of the current bits
        helper = (_bits[index] & mask);
        // shift the current bits to the left by two
        _bits[index] = (_bits[index] >> 2) | (carry << 62);
        // set carry to helper
        carry = helper;
        // increment index
        index--;
    } while (index >= 0);
    return carry;
}

int succinct_tree::size() const
{
    return __bits_len;
}

int succinct_tree::height() const
{
    for (int i = 0; i < __bits_len*sizeof(_bits)*8; i+=2)
    {
        uint64_t bit = (_bits[i / 64] >> i) & 3;
        if (bit == EON)
        {
            return i/2;
        }
    }
    return __bits_len*sizeof(_bits)*8/2;
}

void succinct_tree::first_child()
{
    push_two(SEP);
}

void succinct_tree::next_sibling()
{
    // pop until a SEP is found,
    // then increment the binary string and push it back.
    uint64_t bit;
    std::vector<uint64_t> stack;
    uint64_t mask = 3;
    // pop until a SEP or EON is found.
    while ((bit = pop_two()) != SEP and bit != EON)
    {
        stack.push_back(bit);
    }
    if (bit == EON)
    {
        // if EON is found, we are at root, nothing to do
        return;
    }
    // put the SEP back on the tree
    push_two(SEP);
    // if stack is empty, push a ZERO on the stack
    if (stack.empty())
    {
        stack.push_back(ZERO);
    } else {
        // increment bitstring found on stack
        uint64_t carry = ONE;
        for (int i = 0; i < stack.size(); i++)
        {
            if (stack[i] == ZERO and carry == ONE){
                stack[i] = ONE;
                carry = ZERO;
            } else if (stack[i] == ONE and carry == ONE){
                stack[i] = ZERO;
                carry = ONE;
            } else {
                // no more carry left, remainder does not change
                break;
            }
        }
        // if there is a carry left, start the new string with it
        if (carry == ONE)
        {
            push_two(ONE);
        }
    }
    // push the stack back on the tree
    while (!stack.empty())
    {
        push_two(stack.back());
        stack.pop_back();
    }

}

void succinct_tree::parent()
{
    // pop until a SEP is found.
    uint64_t bit;
    uint64_t mask = 3;
    while ((bit = pop_two()) != SEP and bit != EON)
    {
        // do nothing
    }
}

void succinct_tree::resize()
{
    // resize the tree by doubling the size of the bit array
    uint64_t *new_bits = new uint64_t[__bits_len*2];
    for (int i = 0; i < __bits_len; i++)
    {
        new_bits[i] = _bits[i];
    }
    delete[] _bits;
    _bits = new_bits;
    __bits_len *= 2;
}

std::string succinct_tree::to_string(bool decimal) const
{
    // find EON
    int index = 0;
    uint64_t mask = 3;
    while (index < __bits_len*sizeof(_bits)*8)
    {
        uint64_t bit = (_bits[index / 64] >> index) & mask;
        if (bit == EON)
        {
            break;
        }
        index += 2;
    }
    if (index > 0)
        index -= 2;
    // emit starting from given index.
    // if decimal is true, emit decimal numbers.
    std::stringstream result;
    result << "r";
    if (decimal)
    {
        std::vector<uint64_t> stack;
        for (int i = index; i >= 0; i-=2)
        {
            uint64_t bit = (_bits[i / 64] >> i) & mask;
            if (bit == SEP)
            {
                if (!stack.empty())
                {
                    int dec = 0;
                    int pos = 0;
                    while (!stack.empty())
                    {
                        dec += stack.back() * pow(2, pos);
                        stack.pop_back();
                        pos++;
                    }
                    result << std::to_string(dec);
                }
                result << ",";
            }
            else if (bit == ZERO)
            {
                stack.push_back(0);
            }
            else if (bit == ONE)
            {
                stack.push_back(1);
            }
        }
        if (!stack.empty())
        {
            int dec = 0;
            int pos = 0;
            while (!stack.empty())
            {
                dec += stack.back() * pow(2, pos);
                stack.pop_back();
                pos++;
            }
            result << std::to_string(dec);
        }
    }
    else
    {
        for (int i = index; i >= 0; i-=2)
        {
            uint64_t bit = (_bits[i / 64] >> i) & mask;
            if (bit == SEP)
            {
                result << ",";
            }
            else if (bit == ZERO)
            {
                result << "0";
            }
            else if (bit == ONE)
            {
                result << "1";
            }
        }
    }
    return result.str();
}

// ---------------------- TEST ----------------------

int main(int argc, const char** argv) {

    succinct_tree tree;
    std::cout << "0 level child 0: " << tree.to_string() << "\t" << tree.to_string(false) << std::endl;
    tree.next_sibling();
    std::cout << "0 level child 0: " << tree.to_string() << "\t" << tree.to_string(false) << std::endl;
    tree.first_child();
    std::cout << "1 level child 0: " << tree.to_string() << "\t" << tree.to_string(false) << std::endl;
    tree.next_sibling();
    std::cout << "1 level child 1: " << tree.to_string() << "\t" << tree.to_string(false) << std::endl;
    tree.first_child();
    std::cout << "2 level child 0: " << tree.to_string() << "\t" << tree.to_string(false) << std::endl;
    tree.first_child();
    std::cout << "3 level child 0: " << tree.to_string() << "\t" << tree.to_string(false) << std::endl;
    tree.next_sibling();
    std::cout << "3 level child 1: " << tree.to_string() << "\t" << tree.to_string(false) << std::endl;
    tree.next_sibling();
    std::cout << "3 level child 2: " << tree.to_string() << "\t" << tree.to_string(false) << std::endl;
    tree.next_sibling();
    std::cout << "3 level child 3: " << tree.to_string() << "\t" << tree.to_string(false) << std::endl;
    tree.next_sibling();
    std::cout << "3 level child 4: " << tree.to_string() << "\t" << tree.to_string(false) << std::endl;
    tree.next_sibling();
    std::cout << "3 level child 5: " << tree.to_string() << "\t" << tree.to_string(false) << std::endl;
    tree.next_sibling();
    std::cout << "3 level child 6: " << tree.to_string() << "\t" << tree.to_string(false) << std::endl;
    tree.next_sibling();
    std::cout << "3 level child 7: " << tree.to_string() << "\t" << tree.to_string(false) << std::endl;
    tree.next_sibling();
    std::cout << "3 level child 8: " << tree.to_string() << "\t" << tree.to_string(false) << std::endl;
    tree.parent();
    std::cout << "2 level child 0: " << tree.to_string() << "\t" << tree.to_string(false) << std::endl;
    tree.parent();
    std::cout << "1 level child 1: " << tree.to_string() << "\t" << tree.to_string(false) << std::endl;
    tree.parent();
    std::cout << "0 level child 0: " << tree.to_string() << "\t" << tree.to_string(false) << std::endl;
    tree.parent();
    std::cout << "0 level child 0: " << tree.to_string() << "\t" << tree.to_string(false) << std::endl;

    // uint64_t *bits = new uint64_t[2];
    // memset(bits, 0, 2*sizeof(uint64_t));
    // std::cout << bit_array_to_string(bits, 2*sizeof(uint64_t)*8) << std::endl;
    // bits[0] = (bits[0] << 2) | SEP;
    // std::cout << bit_array_to_string(bits, 2*sizeof(uint64_t)*8) << std::endl;
    // std::cout << enctobin(bits, 2*sizeof(uint64_t)*8) << std::endl;
    // bits[0] = (bits[0] << 2) | ZERO;
    // std::cout << bit_array_to_string(bits, 2*sizeof(uint64_t)*8) << std::endl;
    // std::cout << enctobin(bits, 2*sizeof(uint64_t)*8) << std::endl;
    // bits[0] = (bits[0] << 2) | ONE;
    // std::cout << bit_array_to_string(bits, 2*sizeof(uint64_t)*8) << std::endl;
    // std::cout << enctobin(bits, 2*sizeof(uint64_t)*8) << std::endl;
    // uint64_t bit = bits[0] >> 2;
    // std::cout << "poped bit:" << bit << std::endl;
    // std::cout << bit_array_to_string(&bit, sizeof(uint64_t)*8) << std::endl;
    // std::cout << enctobin(&bit, sizeof(uint64_t)*8) << std::endl;
    // bits[0] >>= 2;
    // std::cout << bit_array_to_string(bits, 2*sizeof(uint64_t)*8) << std::endl;
    // std::cout << enctobin(bits, 2*sizeof(uint64_t)*8) << std::endl;

    // std::cout << "pushing" << std::endl;
    // succinct_tree tree(2);
    // std::cout << bit_array_to_string(tree.get_bits(), tree.size()*sizeof(uint64_t)*8) << std::endl;
    // for (int i = 0; i < 33+30; i++)
    // {
    //     tree.push_two(SEP);
    //     std::cout << bit_array_to_string(tree.get_bits(), tree.size()*sizeof(uint64_t)*8) << std::endl;
    //     std::cout << "height: " << tree.height() << std::endl;
    // }
    // std::cout << "tring another push" << std::endl;
    // try
    // {
    //     tree.push_two(EON);
    // }
    // catch(std::logic_error e)
    // {
    //     std::cout << e.what() << std::endl;
    // }    
    // std::cout << "popping" << std::endl;
    // for (int i = 0; i < 33+32; i++)
    // {
    //     tree.pop_two();
    //     std::cout << bit_array_to_string(tree.get_bits(), tree.size()*sizeof(uint64_t)*8) << std::endl;
    //     std::cout << "height: " << tree.height() << std::endl;
    // }
    return 0;
}