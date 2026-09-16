#include "craam/RMDP.hpp"
#include "craam/modeltools.hpp"
#include "craam/algorithms/values.hpp"
#include <iostream>
#include <vector>

using namespace craam;
using namespace std;

int main() {
    MDP mdp(9);
 	
	// IJCAITest6.txt
   
	// transitions for action A
	int actionID = 0;
    add_transition(mdp, 0, actionID, 1, 1, 0);
    
	// transitions for action C
	actionID++;
    add_transition(mdp, 1, actionID, 3, 0.9, 0);
    add_transition(mdp, 1, actionID, 4, 0.1, 1);
	
	// transitions for action loop1
	actionID++;
    add_transition(mdp, 3, actionID, 3, 1, 0);
	
    	// transitions for action loop2
	actionID++;
    add_transition(mdp, 4, actionID, 4, 1, 0);
	
	// solve using Jacobi value iteration
    auto&& re = algorithms::solve_mpi(mdp, 1, numvec(0), indvec(0), 100, SOLPREC, 100, SOLPREC/2, true);
    for(auto v : re.valuefunction){
        cout << v << " ";
    }
    cout << endl;
	return 0;
}
