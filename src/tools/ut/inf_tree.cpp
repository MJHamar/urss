#include "universal_tree.hpp"

namespace utree {
/**
 * @brief Definitions for the infinite_tree class
 * 
 */

  ordered_tree *
  infinite_tree::up() {
    if (_parent == NULL) TREE_BOUNDS_ERROR;
    return this->_parent;
  }
  ordered_tree *
  infinite_tree::down() {
    if (_kids.size() == 0 ) _kids.push_back(new infinite_tree(_size, _d-2, _parity, this, 0));
    return _kids[0];
  }
  ordered_tree *
  infinite_tree::left() {
    if (_parent == NULL) {
      std::cout << "ERROR: root.left() called" << std::endl;
      TREE_BOUNDS_ERROR;  
    }
    return _parent->left_ch(this);
  }
  ordered_tree *
  infinite_tree::right() {
    if (_parent == NULL)
    {
      std::cout << "ERROR: root.left() called" << std::endl;
      TREE_BOUNDS_ERROR;
    }
    return _parent->right_ch(this);
  }

  /**
   * @brief This funtion is equivalent to up(). _parent is always an existing pointer, copying it to another makes the tree ambiguous
   * 
   * @param mem pointer to a memory handler (called agent)
   * @return infinite_tree* the parent of this node.
   */
  infinite_tree *
  infinite_tree::up(agent<infinite_tree> *mem)
  {
    if (_parent == NULL) {
      std::cout << "ERROR: root.up() called" << std::endl;
      TREE_BOUNDS_ERROR
    }
    return this->_parent;
  }
  infinite_tree *
  infinite_tree::down(agent<infinite_tree> *mem)
  {
    if (_kids.size() == 0){
      // reserve new location
      auto *kid = mem->get(mem->get_id());
      *kid = infinite_tree(_size, _d-2, _parity, this, 0);
      _kids.push_back(kid);
    }
    return _kids[0];
  }
  infinite_tree *
  infinite_tree::left(agent<infinite_tree> *mem)
  {
    if (_parent == NULL){
      std::cout << "ERROR: root.left() called" << std::endl;
      TREE_BOUNDS_ERROR;
    } // root has no siblings
    return this->_parent->left_ch(this, mem);
  }
  infinite_tree *
  infinite_tree::right(agent<infinite_tree> *mem)
  {
    if (_parent == NULL)
    {
      std::cout << "ERROR: root.left() called" << std::endl;
      TREE_BOUNDS_ERROR;
    } // root has no siblings
    return this->_parent->right_ch(this, mem);
  }

  infinite_tree *
  infinite_tree::left_ch(infinite_tree *from){
    // linear search for given child
    int i = 0;
    while (i < (int) _kids.size() && _kids[i] != from)
    {
      i++;
    } // i points to from

    // report errors
    if (i == (int) _kids.size() || _kids[i] != from)
    {
      std::cout << "ERROR:: child not found, but I am its parent. " << std::endl;
      TREE_BOUNDS_ERROR;
    }

    if (i == 0)
    {
      std::cout << "ERROR:: no left child of smallest child ! " << std::endl;
      TREE_BOUNDS_ERROR;
    }

    return _kids[i - 1];
  }
  infinite_tree *
  infinite_tree::right_ch(infinite_tree *from){
    // linear search for given child
    int i = 0;
    while (i < (int) _kids.size() && _kids[i] != from)
    {
      i++;
    } // i points to from

    // report errors
    if (i == (int) _kids.size() || _kids[i] != from)
    {
      std::cout << "ERROR:: child not found, but I am its parent. " << std::endl;
      TREE_BOUNDS_ERROR;
    }

    if (i == (int) _kids.size() - 1)
    {
      // compute right child.
      infinite_tree *kid = new infinite_tree(_size, _d-2, _parity, this, _kids[i]->get_repr() + 1);

      _kids.push_back(kid);
    }

    return _kids[i + 1];
  }
  infinite_tree *
  infinite_tree::left_ch(infinite_tree *from, agent<infinite_tree> *mem){
    // linear search for given child
    int i = 0;
    while (i < (int) _kids.size() && _kids[i] != from) { i++; } // i points to from

    // report errors
    if (i == (int) _kids.size() || _kids[i] != from)
    {
      std::cout << "ERROR:: child not found, but I am its parent. " << std::endl;
      TREE_BOUNDS_ERROR;
    }

    if (i == 0) {
      std::cout << "ERROR:: no left child of smallest child ! " << std::endl;
      TREE_BOUNDS_ERROR;
    }

    return _kids[i-1];

  }
  infinite_tree *
  infinite_tree::right_ch(infinite_tree *from, agent<infinite_tree> *mem){
    // linear search for given child
    int i = 0;
    while (i < (int) _kids.size() && _kids[i] != from) { i++; } // i points to from

    // report errors
    if (i == (int) _kids.size() || _kids[i] != from)
    {
      std::cout << "ERROR:: child not found, but I am its parent. " << std::endl;
      TREE_BOUNDS_ERROR;
    }

    if (i == (int) _kids.size()-1) {
      // compute right child.
      auto *kid = mem->get(mem->get_id());

      *kid = infinite_tree(_size, _d-2, _parity, this, _kids[i]->get_repr()+1);

      _kids.push_back(kid);
    }

    return _kids[i+1];
  }

  std::string
  infinite_tree::to_string() {
    std::string ret = "";
    if (_parent != NULL) {
      ret += _parent->to_string() + "," + std::to_string(_repr);
    } else {
      ret += (_parity ? "o" : "e");
    }
    return ret;
  }
  std::string
  infinite_tree::to_string(agent<infinite_tree> &mem) {
    std::string ret = "";
    if (_repr == -1) { 
      ret += "         "; 
      return ret;
    }
    if (_parent != NULL) {
      int parent_id = _parent - mem.get(0);
      ret += "{";
      if (parent_id < 10) { ret += " "; }
      ret += std::to_string(parent_id) + "}";
      ret += std::to_string(_repr);
    } else {
      ret += _parity == 0 ? "    e" : "    o";
    }
    ret += "[";
    if (this->get_count() < 10) { ret += " "; }
    ret += std::to_string(this->get_count()) + "]";
    return ret;
  }

  infinite_tree *
  infinite_tree::truncate(int level) { 
    int curr = this->level();
    auto *ret = this;
    if (level % 2 != _parity) { std::cout << "Requested level has different parity than the tree" << std::endl; TREE_BOUNDS_ERROR;}
    if (curr > level)         { return this; }
    if (level > 2*_d-_parity) { std::cout << "Requested level > max hight!" << std::endl;     TREE_BOUNDS_ERROR; }
    for (; curr < level; curr += 2) {
      if (ret->_parent == NULL) { std::cout << "ERROR: parent is NULL" << std::endl; TREE_BOUNDS_ERROR;}
      ret = ret->_parent; 
    }
    // sanity check
    if (ret == NULL)            { std::cout << "ERROR: Truncate returning with Root!" << std::endl; TREE_BOUNDS_ERROR; }
    if (ret->level() != level)  { std::cout << "ERROR: Truncated to the wrong level!" << std::endl; TREE_BOUNDS_ERROR; }
    return ret;
  }

  bool
  infinite_tree::is_descendant_of(infinite_tree *parent){
    if (this == parent)        { return true;  }
    if (this->_parent == NULL) { return false; }
    return _parent->is_descendant_of(parent);
  }

  void 
  infinite_tree::reserve(){
    // do nothing
    return ;
  }
  void 
  infinite_tree::release(){
    _kids.clear();
    _parent->release(_repr);
    _repr = -1;
  }
  void 
  infinite_tree::release(int repr){
    if (repr < 0)                   { std::cout << "ERROR: repr less than 0!" << std::endl; return ; }
    

    std::vector<infinite_tree *> help;
    // search for given repr. this should be unique right?
    while (!_kids.empty()){
      if (_kids.back()->get_repr() == repr) { _kids.pop_back(); break; }
      help.push_back(_kids.back());
      _kids.pop_back();
    }
    // resore array
    while (!help.empty()){ 
      _kids.push_back(help.back());
      help.pop_back();
    }
  }

  infinite_tree &
  infinite_tree::operator=(const infinite_tree &src){
    pg::bitset::operator=(src);
    _d      = src._d;
    _parity = src._parity;
    _parent = src._parent;
    _kids   = src._kids;
    _repr   = src._repr;
    return *this;
  }
  infinite_tree &
  infinite_tree::operator-=(const infinite_tree &src){
    pg::bitset::operator-=(src);
    if (_parent != NULL) { *_parent -= src; }
    return *this;
  }
  infinite_tree &
  infinite_tree::operator&=(const infinite_tree &src){
    pg::bitset::operator&=(src);
    if (_parent != NULL) { *_parent &= src; }
    return *this;
  }
  infinite_tree &
  infinite_tree::operator|=(const infinite_tree &src){
    pg::bitset::operator|=(src);
    if (_parent != NULL) { *_parent |= src; }
    return *this;
  }
  // TODO: additional operators

  infinite_tree &
  infinite_tree::operator-=(const pg::bitset &src){
    pg::bitset::operator-=(src);
    if (_parent != NULL) { *_parent -= src; }
    return *this;
  }
  infinite_tree &
  infinite_tree::operator&=(const pg::bitset &src){
    pg::bitset::operator&=(src);
    if (_parent != NULL) { *_parent &= src; }
    return *this;
  }
  infinite_tree &
  infinite_tree::operator|=(const pg::bitset &src){
    pg::bitset::operator|=(src);
    if (_parent != NULL) { *_parent |= src; }
    return *this;
  }


}