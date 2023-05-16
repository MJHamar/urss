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

#include <algorithm>
#include <iomanip>
#include "tspm.hpp"

namespace pg {

TSPMSolver::TSPMSolver(Oink *oink, Game *game) : Solver(oink, game)
{
}

TSPMSolver::~TSPMSolver()
{
}

/**
 * Returns true if a progress measure "a" is less than "b"
 * up to and including priority <d>, for player <pl>.
 * 
 * In the paper, this is "turncated less than"
 */
bool
TSPMSolver::pm_less(int* a, int* b, int d, int pl)
{
    // cases where a or b is Top
    if (b[pl] == -1) return a[pl] != -1;
    else if (a[pl] == -1) return false;
    // normal comparison, start with highest priority
    const int start = ((k&1) == pl) ? k-2 : k-1;
    for (int i=start; i>=d; i-=2) {
        if (a[i] == b[i]) continue;
        if (a[i] > counts[i] and b[i] > counts[i]) return false;
        return a[i] < b[i];
    }
    return false;
}

/**
 * Copy for player <pl>.
 */
void
TSPMSolver::pm_copy(int *dst, int *src, int pl)
{
    for (int i=pl; i<k; i+=2) dst[i] = src[i];
}

/**
 * Write pm to ostream.
 */
void
TSPMSolver::pm_stream(std::ostream &out, int *pm)
{
    bool tope = pm[0] == -1; // Top for Even
    bool topo = pm[1] == -1; // Top for Odd
    out << " {";
    if (tope) out << " \033[1;33mTe\033[m";
    else out << " " << pm[0];
    if (topo) out << " \033[1;33mTo\033[m";
    else out << " " << pm[1];
    for (int i=2; i<k; i++) {
        // equivalent to i%2 != 0
        if (i&1) out << " " << (topo ? 0 : pm[i]);
        else     out << " " << (tope ? 0 : pm[i]);
    }
    out << " } ";
}

/**
 * Perform update for player <pl>, node with priority <d>.
 * 
 * This function is the actor in raising the progress measures
 * Also repsonsible for setting Top for player pl.
 */
void
TSPMSolver::Prog(int *dst, int *src, int d, int pl)
{
    // check if top
    if (src[pl] == -1) {
        dst[pl] = -1;
        return;
    }

    // set every value lower than <d> to 0.
    // dst is used as the output of this function. So we initialise to 0
    int i = pl;
    for (; i<d; i+=2) dst[i] = 0;

    int carry = (d == i) ? 1 : 0; // only increase if <d> has parity <pl>

    for (; i<k; i+=2) {
        // increase or copy for same parity
        // eg. :
        // for            counts = [2,0,2,0,2]
        // and            dst    = [2,0,2,0,1],
        // this loop sets src    = [0,0,0,0,2]
        // and this pm will not become Top
        int v = src[i] + carry;
        if (v > counts[i]) {
            dst[i] = 0;
            carry = 1;
        } else {
            dst[i] = v;
            carry = 0;
        }
    }

    // check if top
    if (carry) dst[pl] = -1;
}

bool
TSPMSolver::canlift(int node, int pl)
{
    // obtain ptr to current progress measure
    int *pm = pms + k*node;

    // check if already Top
    if (pm[pl] == -1) return false;

    const int d = priority(node);

    if (owner(node) == pl) {
        // do max
        for (auto curedge = outs(node); *curedge != -1; curedge++) {
            int to = *curedge;
            if (disabled[to]) continue;
            Prog(tmp, pms + k*to, d, pl); // does not modify any progress measurte modification, just makes a goal pm sotred in tmp
            if (pm_less(pm, tmp, d, pl)) return true;
        }
        return false;
    } else {
        // do min
        int best_to = -1;
        for (auto curedge = outs(node); *curedge != -1; curedge++) {
            int to = *curedge;
            if (disabled[to]) continue;
            Prog(tmp, pms + k*to, d, pl);
            // this runs at least once as best_to is initialised to -1
            // also to never == -1 so best_to == -1 iff for each outedge, the receiving node is disabled
            if (best_to == -1 or pm_less(tmp, best, d, pl)) {
                // copy the computed goal pm to best and set best_to to the current receiving node
                for (int i=0; i<k; i++) best[i] = tmp[i];
                best_to = to;
            }
        }
        // means that all receiving neighbours are disabled
        if (best_to == -1) return false;
        // can perform a lift if the current progress measure is less than the computed progress measure
        // will not perform lift here 
        return pm_less(pm, best, d, pl);
    }
}

bool
TSPMSolver::lift(int node, int target)
{
    // obtain ptr to current progress measure
    int *pm = pms + k*node;

    // check if already Top for both players
    if (pm[0] == -1 and pm[1] == -1) return false;

    lift_attempt++;

    // initialize stuff
    // max is Even in the paper, min is Odd
    const int pl_max = owner(node);
    const int pl_min = 1 - pl_max;
    // node priority
    const int d = priority(node);

#ifndef NDEBUG
    if (trace >= 2) {
        logger << "\033[1mupdating node " << node << "/" << d << (owner(node)?" (odd)":" (even)") << "\033[m with current progress measure";
        pm_stream(logger, pm);
        logger << std::endl;
    }
#endif

    // these will track 
    int best_ch0 = -1, best_ch1 = -1;

    // do max for player <pl_max> if not top
    if (pm[pl_max] != -1) {
#ifndef NDEBUG
        if (trace >= 2) {
            logger << "computing max" << std::endl;
            pm_copy(tmp, pm, 1-pl_max);
        }
#endif
        if (target != -1) {
            // just look at target
            Prog(tmp, pms + k*target, d, pl_max);
#ifndef NDEBUG
            if (trace >= 2) {
                logger << "successor node " << target << "/" << priority(target) << " results in";
                pm_stream(logger, tmp);
                logger << std::endl;
            }
#endif
            // this is the same as below in the else branch
            if (pm_less(pm, tmp, d, pl_max)) {
                pm_copy(pm, tmp, pl_max);
                if (pl_max) best_ch1 = target;
                else best_ch0 = target;
            }
        } else {
            // otherwise, do min
            for (auto curedge = outs(node); *curedge != -1; curedge++) {
                int to = *curedge;
                if (disabled[to]) continue;
                Prog(tmp, pms + k*to, d, pl_max);
#ifndef NDEBUG
                if (trace >= 2) {
                    logger << "successor node " << to << "/" << priority(to) << " results in";
                    pm_stream(logger, tmp);
                    logger << std::endl;
                }
#endif
                if (pm_less(pm, tmp, d, pl_max)) {
                    pm_copy(pm, tmp, pl_max);
                    if (pl_max) best_ch1 = to;
                    else best_ch0 = to;
                }
            }
        }
    }

    // do min for player <pl_min> if not top and target is unspecified of target is at the end of the strategy edge from node
    if (pm[pl_min] != -1 and (target == -1 or target == strategy[node])) {
#ifndef NDEBUG
        if (trace >= 2) logger << "computing min" << std::endl;
        if (trace >= 2) pm_copy(tmp, pm, 1-pl_min);
#endif
        int best_to = -1;
        for (auto curedge = outs(node); *curedge != -1; curedge++) {
            int to = *curedge;
            if (disabled[to]) continue;
            Prog(tmp, pms + k*to, d, pl_min);
#ifndef NDEBUG
            if (trace >= 2) {
                logger << "successor node " << to << "/" << priority(to) << " results in";
                pm_stream(logger, tmp);
                logger << std::endl;
            }
#endif
            if (best_to == -1 or pm_less(tmp, best, d, pl_min)) {
                for (int i=0; i<k; i++) best[i] = tmp[i];
                best_to = to;
            }
        }
        // for minimizing player (node parity != player parity)
        // strategy edge will be the minimum progress measure that was found
        strategy[node] = best_to;
        // note: sometimes only the strategy changes, but the lowest pm stays the same
        // now "best" contains the smallest Prog, which may be higher than the current min
        if (pm_less(pm, best, d, pl_min)) {
            pm_copy(pm, best, pl_min); // modify node's progress measure with the one computed
            if (pl_min) best_ch1 = best_to;
            else best_ch0 = best_to;
        }
    }

    bool ch0 = best_ch0 != -1;
    bool ch1 = best_ch1 != -1;

    if (ch0 or ch1) {
        if (trace) {
            logger << "\033[1;32mupdated node " << node << "/" << d << (owner(node)?" (odd)":" (even)") << "\033[m to";
            pm_stream(logger, pm);
            logger << std::endl;
        }
        // update counts if a changed pm is now Top, but only if priority has same parity as winner
        // Top nodes are never part of any winning cycle, so we decrease the upper bound on progress measures
        if (ch0 and pm[0] == -1 and (d&1) == 0) counts[d]--;
        if (ch1 and pm[1] == -1 and (d&1) == 1) counts[d]--;
        // increase count and return true
        lift_count++;
        return true;
    } else {
        // no lift was performed, return false. 
        return false;
    }
}

void
TSPMSolver::update(int pl)
{
    std::queue<int> q;

    // find unstable nodes (for measure <pl>)
    for (int i=0; i<nodecount(); i++) {
        if (disabled[i]) continue;
        unstable[i] = 0; // first mark as stable
        if (pms[k*i + pl] == -1 or canlift(i, pl)) {
            unstable[i] = 1;
            q.push(i);
        }
    }

    while (!q.empty()) {
        int n = q.front();
        q.pop();
        for (auto curedge = ins(n); *curedge != -1; curedge++) {
            int m = *curedge;
            if (disabled[m] or unstable[m]) continue;
            if (owner(m) != pl) {
                int best_to = -1;
                const int d = priority(m);
                for (auto curedge = outs(m); *curedge != -1; curedge++) {
                    int to = *curedge;
                    if (disabled[to]) continue;
                    if (unstable[to]) continue;
                    Prog(tmp, pms + k*to, d, pl);
                    if (best_to == -1 or pm_less(tmp, best, d, pl)) {
                        for (int i=0; i<k; i++) best[i] = tmp[i];
                        best_to = to;
                    }
                }
                if (best_to != -1 and !pm_less(pms+k*m, best, d, pl)) continue;
            }
            unstable[m] = 1;
            q.push(m);
        }
    }

    for (int i=0; i<nodecount(); i++) {
        if (disabled[i]) continue;
        if (unstable[i] == 0 and pms[k*i + 1-pl] != -1) {
            if ((priority(i)&1) != pl) counts[priority(i)]--;
            pms[k*i + 1-pl] = -1;
            todo_push(i);

            if (trace) {
                logger << "\033[1;33mupdated node " << i << "/" << priority(i) << (owner(i)?" (odd)":" (even)") << "\033[m to";
                pm_stream(logger, pms + i*k);
                logger << std::endl;
            }
        }
    }
}

void
TSPMSolver::run()
{
    // determine k = highest priority + 1
    k = priority(nodecount()-1)+1;
    if (k < 2) k = 2;

    // now create the data structure, for each node
    pms = new int[(size_t)k*nodecount()];
    strategy = new int[nodecount()];
    counts = new int[k];
    tmp = new int[k];
    best = new int[k];
    dirty = new int[nodecount()];
    unstable = new int[nodecount()];

    // initialize all measures to 0
    for (int i=0; i<k*nodecount(); i++) pms[i] = 0;

    // initialize strategy to -1
    for (int i=0; i<nodecount(); i++) strategy[i] = -1;

    // initialize counts for each priority
    for (int i=0; i<k; i++) counts[i] = 0;
    for (int i=0; i<nodecount(); i++) if (disabled[i] == 0) counts[priority(i)]++;

    // initialize all nodes as not dirty
    for (int n=0; n<nodecount(); n++) dirty[n] = 0;

    // set number of lifts and lift attempts to 0
    lift_count = lift_attempt = 0;

    /**
     * Strategy that updates predecessors then marks updated predecessors for processing.
     * Uses a queue/stack to store the dirty vertices.
     */

    /**
     * Initialization loop.
     */

    for (int n=nodecount()-1; n>=0; n--) {
        if (!disabled[n] and lift(n, -1)) {
            for (auto curedge = ins(n); *curedge != -1; curedge++) {
                int from = *curedge;
                if (!disabled[from] and lift(from, n)) todo_push(from);
            }
        }
    }
    
    /**
     * The main loop.
     */

    logger << "main loop now" << std::endl;
    int64_t last_update = 0;

    while (!todo.empty()) {
        // get a node that is not progressive
        int n = todo_pop();
        // for each incoming edge for node n
        for (auto curedge = ins(n); *curedge != -1; curedge++) {
            // call the other end of this edge *from* s.t. (from, n)\in E
            int from = *curedge;
            // perform lift operation to make (from, n) progressive
            // mark node *from* for another progressivity check
            if (!disabled[from] and lift(from, n)) todo_push(from);
        }
        if (last_update + 10*nodecount() < lift_count) {
            last_update = lift_count;
            update(0);
            update(1);
        }
    }

#ifndef NDEBUG
    if (trace >= 2) {
        for (int n=0; n<nodecount(); n++) {
            if (disabled[n]) continue;
            logger << "\033[35m**\033[m \033[1mnode " << n << "/" << priority(n) << (owner(n)?" (odd)":" (even)") << "\033[m is";
            pm_stream(logger, pms + k*n);
            logger << std::endl;
        }
    }
#endif
    
    // Now set dominions and derive strategy for even.
    for (int n=0; n<nodecount(); n++) {
        if (disabled[n]) continue;
        int *pm = pms + k*n;
        if ((pm[0] == -1) == (pm[1] == -1)) LOGIC_ERROR;
        const int winner = pm[0] == -1 ? 0 : 1;
        oink->solve(n, winner, game->owner(n) == winner ? strategy[n] : -1);
    }

    delete[] pms;
    delete[] strategy;
    delete[] counts;
    delete[] tmp;
    delete[] best;
    delete[] dirty;
    delete[] unstable;

    logger << "solved with " << lift_count << " lifts, " << lift_attempt << " lift attempts." << std::endl;
}

}
