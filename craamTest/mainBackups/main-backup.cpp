#include "craam/RMDP.hpp"
#include "craam/modeltools.hpp"
#include "craam/algorithms/values.hpp"
#include <iostream>
#include <vector>

using namespace craam;
using namespace std;

int main() {
    MDP mdp(3);
    
	// transitions for action 0
    add_transition(mdp, 0, 0, 0, 1, 0);
    add_transition(mdp, 1, 0, 0, 1, 1);
    add_transition(mdp, 2, 0, 1, 1, 1);
    
	// transitions for action 1
    add_transition(mdp, 0, 1, 1, 1, 0);
    add_transition(mdp, 1, 1, 2, 1, 0);
    add_transition(mdp, 2, 1, 2, 1, 1.1);
    
	// solve using Jacobi value iteration
    auto&& re = algorithms::solve_mpi(mdp, 0.9, numvec(0), indvec(0), 100, SOLPREC, 100, SOLPREC/2, true);
    for(auto v : re.valuefunction){
        cout << v << " ";
    }
    
	return 0;
}
