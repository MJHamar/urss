#ifndef UNIVERSAL_TREE_H
#define UNIVERSAL_TREE_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>
#include "bitset.hpp"
#include "uintqueue.hpp"


#define TREE_BOUNDS_ERROR {throw utree::Error("Tree bounds reached");}
#define TREE_MAP_COUNT_ERROR {throw utree::Error("Less than 0 nodes are mapped to this node!");}

namespace utree
{
  #define BOT_C 1
  #define MID_C 0
  #define TOP_C 2


  class Error : public std::exception
  {
  protected:
    const char *msg;

  public:
    Error(const char *msg) : msg(msg) {}
    ~Error() throw() {}
    virtual const char *what() throw()
    {
      return msg;
    }
  };

  template< class T > class agent;

  /**
   * Base class. Defines one-step navigation to all directions 
   *  up (go to parent)
   *  down (go to smallest child)
   *  left (go to smaller sibling if possible)
   *  right (go to larger sibling if possible)
   * 
   *  to_string 
   * 
   * Does not define any representation
   * 
   */
  class ordered_tree {
    public: 

      ordered_tree() {};
      virtual ~ordered_tree() {};

      virtual ordered_tree *up()    = 0;
      virtual ordered_tree *down()  = 0;
      virtual ordered_tree *left()  = 0;
      virtual ordered_tree *right() = 0;

      virtual std::string to_string() = 0;

  };

  /**
   * Simplest ordered tree with no size restrictions. 
   * Each instance represents a node that is either root or an inner node that has a parent.
   * 
   */
  class infinite_tree : public virtual ordered_tree, public pg::bitset {
    private:

      int _d = 0;
      int _parity = 0;

      infinite_tree *_parent;
      std::vector<infinite_tree *> _kids;

      int _repr = -1;

    public:
      infinite_tree() : 
      pg::bitset(), _parent(NULL) {}

      infinite_tree(int size) : 
      pg::bitset(size), _parent(NULL) {}

      infinite_tree(int size, int d, int parity) : 
      pg::bitset(size), _d(d), _parity(parity), _parent(NULL) {}

      infinite_tree(int size, int d, int parity, int repr) : 
      pg::bitset(size), _d(d), _parity(parity), _parent(NULL), _repr(repr) {}

      infinite_tree(int size, int d, int parity, infinite_tree *parent) : 
      pg::bitset(size), _d(d), _parity(parity), _parent(parent) {}

      infinite_tree(int size, int d, int parity, infinite_tree *parent, int repr) : 
      pg::bitset(size), _d(d), _parity(parity), _parent(parent), _repr(repr) {}

      infinite_tree(int size, int d, int parity, infinite_tree *parent, std::vector<infinite_tree *> kids) : 
      pg::bitset(size), _d(d), _parity(parity), _parent(parent), _kids(kids) {}

      infinite_tree(int size, int d, int parity, infinite_tree *parent, std::vector<infinite_tree *> kids, int repr) : 
      pg::bitset(size), _d(d), _parity(parity), _parent(parent), _kids(kids), _repr(repr) {}

      ~infinite_tree() override { }

      virtual ordered_tree *up() override;
      virtual ordered_tree *down() override;
      virtual ordered_tree *left() override;
      virtual ordered_tree *right() override;

      virtual infinite_tree *up(agent<infinite_tree> *mem)   ;
      virtual infinite_tree *down(agent<infinite_tree> *mem) ;
      virtual infinite_tree *left(agent<infinite_tree> *mem) ;
      virtual infinite_tree *right(agent<infinite_tree> *mem);

    private : 
      infinite_tree *left_ch(infinite_tree *from) ;
      infinite_tree *right_ch(infinite_tree *from);
      infinite_tree *left_ch(infinite_tree *from, agent<infinite_tree> *mem) ;
      infinite_tree *right_ch(infinite_tree *from, agent<infinite_tree> *mem);

    public :
      virtual std::string to_string() override                ;
      virtual std::string to_string(agent<infinite_tree> &mem);

      int get_count() { return pg::bitset::count(); }
      int level()     { return _d; }
      int get_repr()  { return this->_repr; }
      int get_parity(){ return this->_parity; }

      void reserve()   ;  // mark this object as 'reserved'
      void release()   ;  // opposite of reverse
      void release(int);  // remove given index kid so its not in the way

      infinite_tree              *get_parent() { return this->_parent; }
      std::vector<infinite_tree *>get_kids()   { return this->_kids; }
      infinite_tree *truncate(int level);

      bool is_descendant_of(infinite_tree *parent);

      infinite_tree &operator=(const infinite_tree &src);
      infinite_tree &operator-=(const infinite_tree &src);
      infinite_tree &operator&=(const infinite_tree &src);
      infinite_tree &operator|=(const infinite_tree &src);
      infinite_tree &operator-=(const pg::bitset &src)   ;
      infinite_tree &operator&=(const pg::bitset &src)   ;
      infinite_tree &operator|=(const pg::bitset &src)   ; 
      /*TODO
      bool operator<(const infinite_tree &other)   ;
      bool operator<=(const infinite_tree &other)  ;
      bool operator>(const infinite_tree &other)   ;
      bool operator>=(const infinite_tree &other)  ;
      bool operator==(const infinite_tree &other)  ;
      */
  };



  template <class T>
  class agent
  {
    struct map_helper
    {
      int node;
      short sign;
    };

    class display
    {

      class tree_display
      {

        class node
        {
        private:
          std::vector<node *> _kids;
          int _level;
          float x_pos = -1;
          float y_pos = -1;

          bool _head = false;
          int *_counts = new int[3];

          float _vertical_step, _horizontal_step;

        public:
          node()
          {
            for (int i = 0; i < 3; i++) { _counts[i] = 0; }
          }
          node(int level) : _level(level)
          {
            for (int i = 0; i < 3; i++) { _counts[i] = 0; }
          }
          node(std::vector<node *> kids, int level) : _kids(kids), _level(level)
          {
            for (int i = 0; i < 3; i++) { _counts[i] = 0; }
          }
          ~node()
          {
            while (!_kids.empty())
            {
              auto *kid = _kids.back();
              _kids.pop_back();
              delete kid;
            }
            delete[] _counts;
          }

          void add_kid(node *kid) { _kids.push_back(kid); }

          float get_x() { return x_pos; }
          void set_x(float x) { this->x_pos = x; }
          float get_y() { return y_pos; }
          void set_y(float y) { this->y_pos = y; }
          void set_vh(float _vertical, float _horizontal)
          {
            _vertical_step = _vertical;
            _horizontal_step = _horizontal;
            for (auto *kid : _kids)
            {
              kid->set_vh(_vertical, _horizontal);
            }
          }
          void set_head(bool head) { _head = head; }

          int get_level() { return _level; }
          int get_min_level()
          {
            int min = _level;
            for (auto *kid : _kids)
            {
              int curr_l = kid->get_min_level();
              if (curr_l < min)
              {
                min = curr_l;
              }
            }
            return min;
          }

          std::vector<node *> get_kids() { return _kids; }
          node *get_kid(int kid) { return _kids[kid]; }
          std::vector<node *> get_leaves()
          {

            std::vector<node *> ret;
            if (_kids.size() == 0)
            {
              ret.push_back(this);
              return ret;
            }
            for (auto *kid : _kids)
            {
              auto kidskids = kid->get_leaves();
              if (kidskids.size() > 0)
              {
                for (auto *k : kidskids)
                {
                  ret.push_back(k);
                }
              }
            }
            return ret;
          }

          void map(int count, int sign)
          {
            _counts[sign] += count;
          }

          /* function that sets y coordinate to the given value, and based on
           * this node's children's x coordinates, computes and returns this node's
           * x coordinate
          */
          float allocate(float y_coord, float step_size)
          {
            // set y coordinate
            this->y_pos = y_coord;
            // if this is a child, it should already have an x
            if (this->_kids.size() < 1){
              if (this->x_pos < 0){
                TREE_BOUNDS_ERROR;
              }
              return this->x_pos;
            }
            // find x coordinate
            float x = 0;
            for (auto *kid : this->_kids){
              x += kid->allocate(y_coord + step_size, step_size);
            }
            x /= _kids.size();
            // set x
            this->x_pos = x;
            // return x
            return x;
          }

          void svg_xml(std::ostream &out, std::string name)
          {
            // create node as a sparate entity
            out << "<circle ";
            out << "cx=\"" << x_pos << "mm\" cy=\"" << y_pos << "mm\" ";
            out << "r=\"1mm\" ";
            out << "/>" << std::endl;
            // create lines between this and its children
            int i = 0;
            for (auto *kid : _kids){
              out << "<line ";
              out << "x1=\"" << x_pos        << "mm\" y1=\"" << y_pos        << "mm\" ";
              out << "x2=\"" << kid->get_x() << "mm\" y2=\"" << kid->get_y() << "mm\" ";
              out << "stroke=\"black\"/>" << std::endl;
              kid->svg_xml(out, name+std::to_string(i++));
            }
            // create top and side attractor
            float top_x = x_pos + 5;
            float top_y = y_pos - 2;
            float bot_x = x_pos - 5; 
            float bot_y = y_pos + 2;
            out << "<circle ";
            out << "cx=\"" << top_x << "mm\" cy=\"" << top_y << "mm\" ";
            out << "r=\"1mm\" ";
            out << "fill=\"red\"/>" << std::endl;
            out << "<circle ";
            out << "cx=\"" << bot_x << "mm\" cy=\"" << bot_y << "mm\" ";
            out << "r=\"1mm\" ";
            out << "fill=\"green\"/>" << std::endl;
            // and the lines
            out << "<line ";
            out << "x1=\"" << x_pos << "mm\" y1=\"" << y_pos << "mm\" ";
            out << "x2=\"" << top_x << "mm\" y2=\"" << top_y << "mm\" ";
            out << "stroke=\"red\"/>" << std::endl;
            out << "<line ";
            out << "x1=\"" << x_pos << "mm\" y1=\"" << y_pos << "mm\" ";
            out << "x2=\"" << bot_x << "mm\" y2=\"" << bot_y << "mm\" ";
            out << "stroke=\"green\"/>" << std::endl;
          }
        };

      private:
        std::string _msg;
        node *_root;
        int _w = 12; // cm
        int _h = 20; // cm

        float _vertical_step, _horizontal_step; // distance between levels in mm
        float _canvas_width, _canvas_height;

        node *convert_tree(infinite_tree *root)
        {
          //std::cout << "converting tree. current level is " << root->level() << std::endl;
          node *ret = new node(root->level() / 2);
          for (auto *kid : root->get_kids())
          {
            ret->add_kid(convert_tree(kid));
          }
          return ret;
        }

        /**
         * @brief recursively find the horizontal coordinates of given leaves
         *
         */
        void allocate_leaves(std::vector<node *> leaves)
        {
          // start from 0
          float x = 8;
          for (auto leaf : leaves){
            leaf->set_x(x);
            x += _horizontal_step;
          }
        }

        node *find_node(infinite_tree *head)
        {
          // navigation path is the same as children ids (in our case at least)
          std::vector<int> reverse_path;
          auto *tmp = head;
          while (tmp->get_parent() != NULL)
          {
            reverse_path.push_back(tmp->get_repr());
            tmp = tmp->get_parent();
          }
          node *new_h = this->_root;
          while (!reverse_path.empty())
          {
            int kid = reverse_path.back();
            reverse_path.pop_back();
            new_h = new_h->get_kid(kid);
          }
          return new_h;
        }

        void build_myself(infinite_tree *root)
        {
          // first convert the tree to a better data-structure. This should be done recursively.
          this->_root = convert_tree(root);
          compute_coordinates();
          // create a mapping between tree levels and vertical coordinates.
        }

        void compute_coordinates()
        {
          // now, find height and width of the tree.
          int tree_height = _root->get_level();
          auto leaves = _root->get_leaves();

          // first need to find step sizes
          _vertical_step = 15;
          _horizontal_step = 15;

          _canvas_width  = leaves.size() * _horizontal_step + 10;
          _canvas_height = tree_height * _vertical_step + 10;
          
          // perform horizontal centering of leaves
          allocate_leaves(leaves);

          this->_root->allocate(5, _vertical_step);
        }

      public:
        tree_display() : _msg() {}
        tree_display(infinite_tree *root, std::string msg) : _msg(msg)
        {
          build_myself(root);
        }
        ~tree_display() { delete _root; }

        void svg_xml(std::ostream &out)
        {
          // header
          out << "<!DOCTYPE html>" << std::endl;
          out << "<html>" << std::endl;
          out << "<body>" << std::endl;
          out << "<h1> " << _msg << "</h1>" << std::endl;

          out << "<svg width=\"" << _canvas_width << "mm\" height=\"" << _canvas_height << "mm\">" << std::endl;
          //body
          this->_root->svg_xml(out, "r");
          // footer
          out << "</svg>" << std::endl;
          out << "</body>" << std::endl;
          out << "</html>" << std::endl;
        }
        void set_head(infinite_tree *head)
        {
          auto *new_h = find_node(head);
          new_h->set_head(true);
        }
        void map(infinite_tree *node, int sign)
        {
          auto *n = find_node(node);
          n->map(1, sign);
        }

        void reset(infinite_tree *root)
        {
          delete this->_root;
          build_myself(root);
        }
      };

    private:
      agent<infinite_tree> *_mem = NULL;
      std::ostream &_eout;
      std::ostream &_oout;

      tree_display *_even = NULL;
      tree_display *_odd = NULL;

    public:
      display() : _mem(NULL), _eout(std::cout), _oout(std::cout) {}
      display(agent<infinite_tree> *mem) : _mem(mem), _eout(std::cout), _oout(std::cout) {}
      display(agent<infinite_tree> *mem, std::ostream &eout, std::ostream &oout) : _mem(mem), _eout(eout), _oout(oout) {}
      ~display()
      {
        if (_even != NULL) { delete _even; }
        if (_odd != NULL)  { delete _odd;  }
      }

      void load_trees(int e_root, int o_root)
      {
        if (_even != NULL) { delete _even; }
        if (_odd != NULL)  { delete _odd;  }
        _even = new tree_display(_mem->get(e_root), "Even tree");
        _odd =  new tree_display(_mem->get(o_root), "Odd  tree");
        _even->set_head(_mem->get_head(0));
        _odd->set_head(_mem->get_head(1)) ;
      }

      void map(struct map_helper *array, int size)
      {
        for (int i = 0; i < size; i++)
        {
          auto *tree = _mem->get(array[i].node);
          if (tree->get_parity() % 2 == 1) { _odd->map(tree, array[i].sign); continue; }
          _even->map(tree, array[i].sign);
        }
      }

      void svg_xml(){
        _even->svg_xml(_eout);
        _odd->svg_xml(_oout);
      }

    };

  private:


    display *_disp;
    std::ofstream _even_out;
    std::ofstream  _odd_out;

    int _size;
    int _d;
    int _pool_size;
    int *_trees;  // reading heads that the user moves around in the memory.
    /** mapping between game nodes and augmented tree nodes */
    struct map_helper *_map;
    /** pool of integers that we give to each recursive call. */
    pg::uintqueue _pool;
    /**
     * 4n long array of tree nodes where we maintain the current shape of the tree.
     * a recursive call may access its tree node using the integer key given to it.
     */
    T *_buffer;

    int _collections = 0;


  public:
    agent(int size, int d) : _size(size), _d(d)
    {
      _pool_size = _size > _d ? _size*4 : _d*4;
      _map = new struct map_helper[_size*2]; 
      _buffer = new T[_pool_size]; 
      _trees = new int[2];
      _pool.resize(_pool_size);
      _even_out.open("./even.html");
      _odd_out.open("./odd.html")  ;
      _disp = new display(this, _even_out, _odd_out);
      for (int i = _pool_size-1; i >= 0; i--) _pool.push(i);
    }

    ~agent() { 
      delete[] _map; 
      delete[] _buffer; 
      delete[] _trees; 
      delete _disp; 
      _even_out.close();
      _odd_out.close();
    }

    int init(const int pl);

    int step_up(const int pl)   ;
    int step_down(const int pl) ;
    int step_right(const int pl);

    void map(const int pl, const short sign, pg::bitset nodes);
    void map(const int pl, const short sign, std::vector<int> nodes);

    /**
     * @brief If the buffer is full, sweep it from the largest index in decreasing order.
     * delete any node that has 0 positions mapped to it.
     */
    void gb_collect();

    inline std::string sign_str(int pos) {
      switch (_map[pos].sign)
      {
      case BOT_C:
        return "e";
      case MID_C:
        return "0";
      default: // TOP_C
        return "1";
      }
    }

    void show_mappings(std::ostream &out);
    void show_buffer(std::ostream &out);
    void dump(std::ostream &out);
    /**
     * @brief Output textual representation of mapping, save it in tree_out.ut.
     *
     * ** FORMAT **
     * line #1: identity() of underlying tree implementation . <_size>
     *
     */
    void make_plot_out(); // for now, we define out stream in this function

    T *get(int id);
    int get_id();
    T *get_head(int pl);
  };

  // =============================
  //
  //      Desplaying trees
  //
  // =============================


// =============================
//
//      Agent definitions
//
// =============================

template< class B >
int agent<B>::init(const int pl){
  // initialise tree for the given player.
  const int id = _pool.pop();
  if (_d % 2 == pl) { _buffer[id] = B(_size, _d  , pl, 0); }
  else              { _buffer[id] = B(_size, _d+1, pl, 0); }
  _buffer[id].reserve();

  // map everything to root. This is an invariant btw.
  _buffer[id].flip();

  for (int i = 0; i < _size; i++){
    _map[2*i+pl].node = id;
    _map[2*i+pl].sign = MID_C; // initialise everything to tree root
  }
  _trees[pl] = id;
  return id;
}

template< class B >
int agent<B>::step_up(const int pl){
  auto *kid = _buffer[_trees[pl]].up(this);
  _trees[pl] = kid - _buffer;
  return _trees[pl];
}
template< class B >
int agent<B>::step_down(const int pl)
{
  auto *kid = _buffer[_trees[pl]].down(this);
  _trees[pl] = kid - _buffer;
  return _trees[pl];
}
template< class B >
int agent<B>::step_right(const int pl)
{
  auto *kid = _buffer[_trees[pl]].right(this);
  _trees[pl] = kid - _buffer;
  return _trees[pl];
}
template< class B >
void agent<B>::map(const int pl, const short sign, pg::bitset nodes)
{
  for (int i = 0; i < _size; i++){
    if (nodes[i]){
      // remove from previous mapping
      _buffer[_map[2 * i + pl].node][i] = false;
      // map
      _map[2*i+pl].node = _trees[pl];
      _map[2*i+pl].sign = sign;
    }
  }
  // map all positions to this node
  _buffer[_trees[pl]] |= nodes;
}
template< class B >
void agent<B>::map(const int pl, const short sign, std::vector<int> nodes)
{
  pg::bitset nds(_size);
  for (auto i : nodes){
    nds[i] = true;
    // remove from previous mapping
    _buffer[_map[2*i+pl].node][i] = false;
    // map
    _map[2*i+pl].node = _trees[pl];
    _map[2*i+pl].sign = sign;
  }
  // increase node count by nodes.size()
  _buffer[_trees[pl]] |= nds;
}

template< class B >
void agent<B>::gb_collect(){
  _collections++;
  // this should propagate to all parents without the need for deleting parent nodes explicitly
  //std::cout << "Garbage collecting for the " << _collections << "'th time" << std::endl;

  //this->dump(std::cout);
  
  // locations 0 and 1 are (informally) reserved for tree roots
  for (int i = _pool_size - 1; i >= 2; i--){
  
    if (_buffer[i].get_repr() == -1) continue; // already freed

    int curr_p = _buffer[i].get_parity(); // current player/parity

    // if current reading head, skip
    if (_trees[0] == i || _trees[1] == i) continue; 
    
    // if current node is an ancestor of the corresponding reading head, cannot delete. 
    if (_buffer[_trees[curr_p]].is_descendant_of(_buffer + i)) continue;

    // if some nodes are mapped to it, skip
    if (_buffer[i].get_count() > 0) continue;

    // in any other case, let go
    //std::cout << "Pushing " << i << std::endl;
    _buffer[i].release();
    // If the node we are looking at is a direct ancestor of the corresponding reading head, we cannot delete.
    // push this location to the pool of available locations.
    _pool.push(i);
  }

  //std::cout << "Garbage collection ended. Size of pool :: " << _pool.size() << std::endl;

}


template <class B>
void agent<B>::show_mappings(std::ostream &out)
{
  //out << "Nr. of grabage collections: " << _collections << std::endl;
  out << "node;even;odd" << std::endl;
  for (int i = 0; i < _size; i++)
  {
    out << i << ";["  << sign_str(_map[2*i].sign) << ", (" << _buffer[_map[2 * i].node].to_string()     << ")];["
                      << sign_str(_map[2*i].sign) << ", (" << _buffer[_map[2 * i + 1].node].to_string() << ")]" 
                      << std::endl;
  }
}
template <class B>
void agent<B>::show_buffer(std::ostream &out)
{
  for (int i = 0; i < _pool_size; i++){
    out << _buffer + i << " ";
  }
  out << std::endl;

  std::cout << "state of buffer is :: " << std::endl;

  for (int i = _pool_size - 1; i >= 0; i--)
  {
    out << i << "\t" << _buffer[i].to_string() << "\t" << _buffer[i].get_count() << std::endl;
  }
}
template <class B>
void agent<B>::dump(std::ostream &out)
{
  out << "Map :: " ;
  for (int i = 0; i < 2*_size; i++){
    out << i << ":[" << _map[i].node << "]";
  }
  out << std::endl;
  out << "Buffer :: " << std::endl;
  for (int i = 0; i < _pool_size; i++){
    if (i < 10) { out << " "; }
    out << i;
    if (_trees[0] == i || _trees[1] == i) { out << " - "; }
    else                                  { out << " : "; }
    out << _buffer[i].to_string(*this) << ";\t";
    if (i%4 == 1) out << std::endl;
  }
  out << std::endl;
}
template <class B>
void agent<B>::make_plot_out()
{
  // reload display and output to _tree_out
  _disp->load_trees(0,1);
  _disp->map(_map, _size*2);
  _disp->svg_xml();
}
template <class B>
B *agent<B>::get(int id)
{
  return _buffer + id;
}
template <class B>
int agent<B>::get_id()
{
  if (_pool.empty()){ gb_collect(); }
  if (_pool.empty()) { 
    std::cout << "Pool still empty!" << std::endl;
    // double _buffer size
    B *new_buffer = new B[2*_pool_size];
    
    _pool.resize(2*_pool_size);
    // fill pool with new elements
    // copy buffer and
    for (int i = 0; i < _pool_size; i++){
      new_buffer[i] = _buffer[i];
      _pool.push(_pool_size+i);
    }
    // increase size
    _pool_size *= 2;
    // replace buffer pointer
    //delete[] _buffer;
    // TODO: also change _trees[] pointers
    _buffer = new_buffer;
    std::cout << "Resized buffer and pool! " << std::endl;
  }
  int id = _pool.pop();
  _buffer[id].reserve();
  return id;
}
template <class B>
B *agent<B>::get_head(int pl)
{ 
  return _buffer + _trees[pl]; 
}

} // namespace utree

#endif
