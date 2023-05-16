/*
 * Copyright 2017-2018 Tom van Dijk, Johannes Kepler University Linz
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef NZLK_HPP
#define NZLK_HPP

#include <queue>
#include <stack>

#include "solver.hpp"
#include "tools/ut/universal_tree.hpp"
#include "uintqueue.hpp"

namespace pg
{

  struct stack_elem {
    int v;
    int d;
    pg::bitset G;
    int even;
    int odd;
    int r;
    int stage;     // goes from 0 to 2
    pg::bitset ao; // return value of the recursive step
  };

  class NZLKSolver : public Solver
  {
  public:
    NZLKSolver(Oink *oink, Game *game);
    virtual ~NZLKSolver();

    /**
    * int n is the largest priority node
    * player is the same parity as the node,
    * A is the current game,
    * even is the even tree, 
    * odd is the odd tree.
    * @return A
    */
    pg::bitset McNZ();
    virtual void run();

    int iterations;

    int *winning;
    int *strategy;

    std::stack<struct stack_elem> stack;

    utree::agent<utree::infinite_tree> *agent;

    uintqueue Q;
  };


}

#endif
