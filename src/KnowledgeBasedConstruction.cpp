/**
 * @file KnowledgeBasedConstruction.hpp
 * @brief Blah
 * Blah blah
 */

#include "KnowledgeBasedConstruction.hpp"

int totalStates(GlobalModel *const gm){
	long int gms=gm->agents.size();
	long int total=0;
	
	for(long int i=0; i<gms; i++){
		total+=gm->agents[i]->localStates.size();
	}
	
	return total;
}

void mockKBC(GlobalModel *const gm){
	for(int i=0; i<(gm->agents.size()); i++) mockLocalKBC(gm->agents[i]);
}

void mockLocalKBC(Agent *const a){//A fake KBC-esque function
	//delete all transitions
	for(int i=0; i<a->localTransitions.size(); i++){
		delete a->localTransitions[i];
	}
	a->localTransitions.clear();
	
	//convert localStates into P(localStates)
	vector<LocalState*> temp;
	unsigned long int set_size = a->localStates.size();
    unsigned long int pow_set_size = pow(2, set_size);
    int counter, j;
    for (counter = 0; counter < pow_set_size; counter++) {
		for (j = 0; j < set_size; j++) {
			if (counter & (1 << j)){
				LocalState t = *a->localStates[j];
				temp.push_back(&t);
			}
		}
	}
	
	//place new statespace in a, and erase old one
	a->localStates.swap(temp);
	for(int i=0; i<temp.size(); i++){
		delete temp[i];
	}
	temp.clear();
	
	//populate transitions with (loc_i -> loc_i) \forall loc_i in localStates
	for(int i=0; i<a->localStates.size(); i++){
		LocalTransition t;
		t.id = i;
		t.name = a->localStates[i]->name+" on "+a->localStates[i]->name+"@"+a->name;
		t.isShared = false;
		t.sharedCount = 1;
		t.agent = a;
		t.from = a->localStates[i];
		t.to = a->localStates[i];
		
		a->localTransitions.push_back(&t);
	}
}