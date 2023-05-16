
/*
 * Naive implementation of McNaughton-Zielonka algorithm
 * with search-space reduction using Strahler universal trees
 */

#include <algorithm>
#include <sstream>
#include <vector>
#include <queue>
#include <cassert>

#include "nzlk.hpp"

#include <math.h>

namespace pg
{



#define KC "\033[36;1m"

  NZLKSolver::NZLKSolver(Oink *oink, Game *game) : Solver(oink, game), Q(game->nodecount())
  {
  }

  NZLKSolver::~NZLKSolver()
  {
  }

  pg::bitset
  NZLKSolver::McNZ()
  {
    pg::bitset am(nodecount());  // my attractor
    pg::bitset  u(nodecount());  // my and other's attractor
    int *region = new int[nodecount()];
    std::vector<int> D; // target set
    pg::bitset queued(nodecount());

    while (!stack.empty())
    {      
      // obtain current stack frame
      struct stack_elem &crnt = stack.top();
      
      // Initialise //
      am.reset();
      u.reset();
      queued.reset();
      D.clear();
      for (int i = 0; i < nodecount(); i++) region[i] = -1;

      // decide priority and player
      int n = crnt.v;
      const int pr = crnt.d;
      const int pl = pr & 1;

      if (crnt.stage == 0)
      {
        if (crnt.v < 0 || crnt.d < 0) {
          TREE_BOUNDS_ERROR;
        }

        // _________ Produce output __________
        //logger << "_____________" << std::endl;
        //logger << " McNZ called d = " << d << " n = " << v << " pr = " << priority(v) << std::endl;
        //logger << "even_id = " << even << " odd_id = " << odd << std::endl;
        //logger << "even level = " << agent->get(even)->level() << " odd level = " << agent->get(odd)->level() << std::endl;
        //logger << "even = " << agent->get(even)->to_string() << " odd = " << agent->get(odd)->to_string() << std::endl;
        
        //agent->dump(logger);

        // ___________________________________    
        
        // compute extended attractor, that may be empty
        for (; n >= 0; n--){
          if (priority(n) != pr) break;             // no more nodes in target
          if (disabled[n] || !crnt.G[n]) continue;  // not in subgame
          if (am[n]) continue;                      // already attracted

          winning[n]  = pl;
          strategy[n] = -1;

          Q.push(n);
          queued[n] = true;
          while (!Q.empty()){
            int cur = Q.pop();
            if (priority(cur) == pr) D.push_back(cur);

            am[cur] = true;
            queued[cur] = false;

            for (auto *from = ins(cur); *from != -1; from++){
              if (disabled[*from] || !crnt.G[*from]) continue; // not in subgame
              if (am[*from]) continue;                    // already attracted
              if (queued[*from]) continue;                // already in queue
              if (owner(*from) == pl){
                // owned by me, attract
                winning[*from]  = pl;
                strategy[*from] = cur;

                Q.push(*from);
                queued[*from] = true;
              } else {
                // owned by other, set counter appropriately
                int count = region[*from];
                if (count < 0){
                  // first visit, count neighbours to positive
                  for (auto *to = outs(*from); *to != -1; to++){
                    if (disabled[*to] || !crnt.G[*to]) continue; // not in subgame
                    count++;
                  }
                } else {
                  // not the first visit, decrement counter
                  count--;
                }
                if (count == 0){
                  // all neighbours are in the attractor, force this node
                  winning[*from] = pl;
                  strategy[*from] = -1; // opponent's node does not have a strategy

                  Q.push(*from);
                  queued[*from] = true;
                } else {
                  // save counter to region
                  region[*from] = count;
                }
              }
            }
          }
        }
        
        crnt.G -= am;
        // increment stage counter
        crnt.stage++;
        // 2. go recursive
        if (n >= 0)
        { // only if there is a point
          struct stack_elem next; 
          if (pl == 1) 
          { next.v = n; next.d = crnt.d-1; next.G = crnt.G; next.even = crnt.even; next.odd = agent->step_down(1); next.r = crnt.r+1; next.stage = 0; }
          else 
          { next.v = n; next.d = crnt.d-1; next.G = crnt.G; next.even = agent->step_down(0); next.odd = crnt.odd; next.r = crnt.r+1;  next.stage = 0; }
          stack.push(next);
        }
      }
      // after first recursion step
      else if (crnt.stage == 1)
      {
        // 3. compute opponent attractor
        for (int i = nodecount()-1; i >= 0; i--){
          if (disabled[i] || !crnt.G[i]) continue;  // not in subgame
          if (!crnt.ao[i]) continue;                // not attracted

          u[i] = true;

          winning[i] = 1-pl;
          Q.push(i);
        }
        while (!Q.empty()){
          int cur = Q.pop();

          crnt.ao[cur] = true;

          for (auto *from = ins(cur); *from != -1; from++){
            bool can_escape;
            if (disabled[*from] || !crnt.G[*from]) continue; // not in subgame
            if (crnt.ao[*from]) continue;                    // already attracted

            if (owner(*from) == pl){
              // Owner is me, need to ensure that I can't escape to any neighbours
              can_escape = false; // assume that I can't escape
              for (auto *to = outs(*from); *to != -1; to++){
                if (disabled[*to] || !crnt.G[*to]) continue; // not in subgame
                if (crnt.ao[*to]) continue;                  // not an escape
                can_escape = true; // I escaped!
                break;
              }
            } else {
              // owner is the opponent, see if she can stay in region
              // she can
              can_escape = false;
            }
            if (!can_escape){
              // schedule this node for attractor computation
              winning[*from]  = 1-pl;
              strategy[*from] = owner(*from) == pl ? -1 : cur;
              Q.push(*from);
            }
          }
        }

        // 4. set opponent side attractor
        agent->map(1-pl, 2, crnt.ao-u);

        // compute next child if needed.
        if (crnt.ao.any())
        {
          pl == 0 ? crnt.odd = agent->step_right(1) : crnt.even = agent->step_right(0);
        }

        // 5. compute G_{i+1}
        crnt.G -= crnt.ao;

        // 6. attract to new target set
        std::vector<int> temp_d;
        am.reset();
        queued.reset();
        for (auto i : D){
          if (!crnt.G[i]) continue;    // not in subgame anymore
          temp_d.push_back(i);
          if (am[i]) continue;    // already attracted
          winning[i] = pl;
          strategy[i] = -1;
          Q.push(i);
          queued[i] = true;
          while (!Q.empty()){

            int cur = Q.pop();
            queued[cur] = false;

            am[cur] = true;

            for (auto *from = ins(cur); *from != -1; from++){
              if (disabled[*from] || !crnt.G[*from]) continue; // not in subgame
              if (am[*from]) continue;                    // already attracted
              if (queued[*from]) continue;                // already queued
              if (owner(*from) == pl){
                // owned by me, attract
                winning[*from]  = pl;
                strategy[*from] = cur;
                Q.push(*from);
                queued[*from] = true;
              } else {
                // owned by opponent, see if it escapes
                bool can_escape = false;
                for (auto *to = outs(*from); *to != -1; to++){
                  if (disabled[*to] || !crnt.G[*to]) continue; // not in subgame
                  if (am[*to]) continue;                  // not an escape
                  can_escape = true;
                  break;
                }
                if (!can_escape){
                  // I can enforce this node in my attractor, schedule it
                  winning[*from] = pl;
                  strategy[*from] = -1;
                  Q.push(*from);
                  queued[*from] = true;
                }
              }
            }
          }
        }
        // set new D
        D = temp_d;

        if (!crnt.ao.any()) {
          crnt.stage++;
        }

        // TODO: make a second recursion step.
      }

      else 
      {
        // everzthing here is TODO
        // 7. set head node strategies before returning
        for (auto i : D)
        {
          if (owner(i) != pl) continue;
          for (auto *to = outs(i); *to != -1; to++)
          {
            if (disabled[*to] || !G[*to])
              continue; // not in subgame
            strategy[i] = *to;
            break;
          }
        }


        // 8. set player top attractor and target set
        pg::bitset help_me(nodecount());
        for (auto i : D)
        {
          help_me[i] = true;
        }
        am -= help_me;

        
        agent->map(pl, 0, D);
        agent->map(pl, 1, am);

        // 9. step up
        agent->step_up(1-pl);

        // 10. output tree
        //agent->make_plot_out();

        //return G;
      }

      delete[] region;
    }

  }

  void
  NZLKSolver::run()
  {
    /**
     * Initialise data structures. 
     *
     * First, we count Even and Odd vertices for the univeral trees. 
     * Then, we allocate region, winner and strategy arrays 
     * Region array maps positions to nodes of the tree
     * winner is initialised to -1
     * strategy is initialised to -1
     * 
     */
    iterations = 0;

    winning   = new int[nodecount()];
    strategy  = new int[nodecount()];

    pg::bitset game(nodecount());


    memset(winning, -1, sizeof(int));
    memset(strategy, -1, sizeof(int));


    const int max_prio = priority(nodecount()-1);
    int ce = 0;
    int co = 0;
    for (int n=nodecount()-1; n>=0; n--) {
      game[n] = true;
      if (owner(n) == 0) ce++;
      else co++;
    }
    if (max_prio == -1) LOGIC_ERROR; // unexpected empty game

    agent = new utree::agent<utree::infinite_tree>(nodecount(), max_prio);


    //logger << "__________  Main loop ____________" << std::endl;

    int even = agent->init(0) ;
    int odd  = agent->init(1) ;

    //logger << "initial even level " << agent->get(even)->level() << " odd level " << agent->get(odd)->level() << std::endl;

    // call the recursive algorithm
    struct stack_elem first;
    if (max_prio % 2 == 0) 
    { first.v = nodecount()-1; first.d = max_prio, first.G = game; first.even = even; first.odd = agent->step_down(1); first.r = 0; first.stage = 0; }
    else                   
    { first.v = nodecount()-1; first.d = max_prio, first.G = game; first.even = agent->step_down(1); first.odd = odd; first.r = 0;  first.stage = 0; }

    stack.push(first);
    // all the McNaughton-Zielonka method
    McNZ();

    // compute strategies better
    for (int i = 0; i < nodecount(); i++){
      // solve the game as well
      oink->solve(i, winning[i], strategy[i]);
    }

    //logger << "__________  End of main loop ____________" << std::endl;

    //logger << "Finished recursive calls, winning array is " << std::endl;
    std::ofstream map_out, winning_out;
    map_out.open("./map.csv");
    winning_out.open("./winning.csv");
    winning_out << "node;winner;strategy" << std::endl;
    for (int i = 0; i < nodecount(); i++){
      winning_out << i << ";" << winning[i] << ";" << strategy[i] << std::endl;
    }
    agent->show_mappings(map_out);
    agent->gb_collect();
    agent->make_plot_out();

    // clean upc
    map_out.close();
    winning_out.close();
    delete   agent;
    delete[] winning;
    delete[] strategy;

  }
}
