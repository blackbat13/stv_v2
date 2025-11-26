#include "craam/RMDP.hpp"
#include "craam/modeltools.hpp"
#include "craam/algorithms/values.hpp"
#include <iostream>
#include <vector>

using namespace craam;
using namespace std;

int main() {
    MDP mdp(1);
 	
	// transitions for action q0 -> q1
	int actionID = 0;
    add_transition(mdp, 0, actionID, 1, 1, 0);
    
	// transitions for action q1 -> q2
	actionID++;
    add_transition(mdp, 1, actionID, 2, 1, 0);
	
	// transitions for action q2 -> q3, q2 -> q4 (T)
	actionID++;
    add_transition(mdp, 2, actionID, 3, 0.3, 0);
    add_transition(mdp, 2, actionID, 4, 0.7, 1);
	
	// transitions for action q4 (T) -> q5 (T)
	actionID++;
    add_transition(mdp, 4, actionID, 5, 1, 0);
	
	// transitions for action q5 (T) -> q6 (q1 T)
	actionID++;
    add_transition(mdp, 5, actionID, 6, 1, 0);
	
	// transitions for action q6 (q1 T) -> q7 (q2 T)
	actionID++;
    add_transition(mdp, 6, actionID, 7, 1, 0);
	
	// transitions for action q7 (q1 T) -> q8 (q2 T), q7 (q1 T) -> q4 (T)
	actionID++;
    add_transition(mdp, 7, actionID, 8, 0.3, 0);
    add_transition(mdp, 7, actionID, 4, 0.7, 0);
	
	// solve using Jacobi value iteration
    auto&& re = algorithms::solve_mpi(mdp, 1, numvec(0), indvec(0), 100, SOLPREC, 100, SOLPREC/2, true);
    for(auto v : re.valuefunction){
        cout << v << " ";
    }
    cout << endl;
	return 0;
}
