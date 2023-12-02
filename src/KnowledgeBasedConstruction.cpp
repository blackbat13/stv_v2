/**
 * @file KnowledgeBasedConstruction.hpp
 * @brief Blah
 * Blah blah
 */

#include "KnowledgeBasedConstruction.hpp"

using namespace::std;

const bool kbc_debug = true;

/*
void mockKBC(GlobalModel *const gm){
	for(int i=0; i<(gm->agents.size()); i++) mockLocalKBC(gm->agents[i]);
}

void mockLocalKBC(Agent *const a){//A fake KBC-esque function
	if(kbc_debug){//debug printout
		for(int i=0; i<a->localStates.size(); i++){
			cout << "\"" << a->localStates[i]->name << "\"" <<" ";
		}
		cout << endl;
	}
	
	//delete all transitions
	for(int i=0; i<a->localTransitions.size(); i++){//Delete individual transitions just in case it actually frees up memory (not sure if I'm doing it correctly)
		delete a->localTransitions[i];
	}
	a->localTransitions.clear();
	
	//convert localStates into P(localStates) minus the empty set
	vector<LocalState*> temp;
	unsigned long int set_size = a->localStates.size();
    unsigned long int pow_set_size = pow(2, set_size);
    int counter, j;
    for (counter = 0; counter < pow_set_size; counter++){
		/* Very simple power set algorithm, that uses a binary counter to check what objects should be in any given element of the power set
		 * Modified to exclude the empty set
		 * Might run into issues if there are more than 4294967295 states in the local model provided your ULONG_MAX=18446744073709551615
		 * This might not be an issue anyone could run into, however:
		 * Given a very generous (and unrealistic) 256 bits per LocalState object, it would mean that the local model takes up more than 128GiB of RAM, and it's resulting "powersetted" equivalent more than 512EiB
		 * While one might find a workstation in which such an unexpanded model would fit into memory, as of the time of this writing (2023) the biggest supercomputer in the world only has 32PiB of RAM
		 *\/
		LocalState* t = new LocalState();
		t->id = counter;
		t->name = "";
		t->agent = a;
		for(int i=0; i<a->localTransitions.size(); i++){
			t->localTransitions.insert(a->localTransitions[i]);
		}
		
		bool changed = false;
		for (j = 0; j < set_size; j++) {
			if (counter & (1 << j)){
				changed = true;
				t->name += ";"+a->localStates[j]->name;
				t->environment.insert(a->localStates[j]->environment.begin(), a->localStates[j]->environment.end());
			}
		}
		if(changed) temp.push_back(t);
	}
	
	//Place new set of locations in a, and erase old one
	a->localStates.swap(temp);
	for(int i=0; i<temp.size(); i++){//Delete individual state objects just in case it actually frees up memory (not sure if I'm doing it correctly)
		delete temp[i];
	}
	temp.clear();
	
	//populate transitions with (loc_i -> loc_i) \forall loc_i in localStates
	for(int i=0; i<a->localStates.size(); i++){
		LocalTransition t;
		t.id = i;//This should work, I think
		t.name = a->localStates[i]->name+" on "+a->localStates[i]->name+"@"+a->name;//Might not generate unique names
		//localName is left uninitialized, according to spec in LocalTransition.hpp
		t.isShared = false;//Don't know if this is correct
		t.sharedCount = 1;//Same as above
		//I don't really know what to do about the conditions
		t.agent = a;
		t.from = a->localStates[i];
		t.to = a->localStates[i];
		
		a->localTransitions.push_back(&t);
	}
	
	if(kbc_debug){//debug printout
		for(int i=0; i<a->localStates.size(); i++){
			cout << "\"" << a->localStates[i]->name << "\"" <<" ";
		}
		cout << endl;
		cout << "---" << endl;
	}
}
*/

void KBCprojection(GlobalModel *const gm, int agent_id){
	// LocalTransition et;//epsilon transition
	// et.id=-1;
	// et.name = EPSILON;
	// et.localName = EPSILON;
	// et.isShared = false;
	// et.sharedCount = -1;
	
	int c = 0;
	for(GlobalState* gs : gm->globalStates){
		set<GlobalTransition*> globalTransitionsProjected;
		
		for(auto gt : gs->globalTransitions){//identify epsilon transitions
			int relevance = 0;
			for(LocalTransition* lt : gt->localTransitions)
				if(lt->agent->id == agent_id) relevance++;
			if(relevance==0){
				c++;
				for(LocalTransition* lt : gt->localTransitions){
					lt->name = EPSILON;
					lt->localName = EPSILON;
				}
				// gt->localTransitions.clear();
				// gt->localTransitions.insert(&et);
				
				bool isDuplicate = false;//if current element is a duplicate, don't insert it
				for(GlobalTransition* gtc : globalTransitionsProjected)
					if(gtc->from == gt->from && gtc->to == gt->to && gtc->localTransitions == gt->localTransitions){
						isDuplicate = true;
						break;
					}
				if(isDuplicate) continue;
			}
			globalTransitionsProjected.insert(gt);
		}
		gs->globalTransitions.clear();
		gs->globalTransitions.insert(globalTransitionsProjected.begin(), globalTransitionsProjected.end());
	}
	
	cout << "Epsilon:" << c << endl;
}

Agent* KBCexpansion(GlobalModel *const gm, int agent_id){
	Agent* o = new Agent(agent_id, gm->agents[agent_id]->name);
	
	queue<set<GlobalState*>*> observationQueue;
	set<set<GlobalState*>*> searchedObservations;
	set<tuple<
		set<GlobalState*>*,
		set<GlobalState*>*,
		string
		>*> transitions;
	
	
	//initial state observation -> queue
	set<GlobalState*> initialObservation;
	for(pair<Agent*, EpistemicClass*> ee : gm->initState->epistemicClasses){
		if(ee.first->id == agent_id){
			for(pair<string, GlobalState*> ecs : ee.second->globalStates){
				initialObservation.insert(ecs.second);
			}
		}
	}
	observationQueue.push(&initialObservation);
	
	
	while(observationQueue.size() > 0){//process all observations in the queue
		set<GlobalState*> observation = *observationQueue.front();	//Get the next element of the queue,...
		observationQueue.pop();										//...remove it from it,...
		if(searchedObservations.count(&observation)==0){			//...check if observation has already been searched...
			searchedObservations.insert(&observation);				//...and add it to the list of searched observations if not.
			
			//get all action labels
			set<string> labels;
			for(GlobalState* gs : observation) for(GlobalTransition* gt : gs->globalTransitions) for(LocalTransition* lt : gt->localTransitions)
				labels.insert(lt->name);
			
			//search results of each action separately
			for(string label : labels){
				//find all global states succeeding the states in our current observation
				set<GlobalState*> succeedingRaw;
				for(GlobalState* gs : observation) for(GlobalTransition* gt : gs->globalTransitions){
					bool relevant = false;
					for(LocalTransition* lt : gt->localTransitions) if(lt->agent->id==agent_id && lt->name==label){
						//a local transition is only relevant if it belongs to both the agent in question, and represents the currently searched action
						relevant = true;
						break;
					}
					if(relevant) succeedingRaw.insert(gt->to);
				}
				
				//find all subsets of globalstates, which are indistinguishable from the point of view of the relevant agent
				set<set<GlobalState*>*> succeedingObservations;
				while(succeedingRaw.size()>0){
					auto element = succeedingRaw.begin();
					GlobalState* gs = *element;
					set<GlobalState*> nextObservation;
					
					for(pair<Agent*, EpistemicClass*> ee : gs->epistemicClasses){
						if(ee.first->id == agent_id){
							for(pair<string, GlobalState*> ecs : ee.second->globalStates){
								if(succeedingRaw.count(ecs.second)>0){
									nextObservation.insert(ecs.second);
									succeedingRaw.erase(ecs.second);
								}
							}
						}
					}
					
					
					succeedingRaw.erase(gs);
					succeedingObservations.insert(&nextObservation);
				}
				
				//register all found transitions
				for(set<GlobalState*>* obs : succeedingObservations){
					tuple<
						set<GlobalState*>*,
						set<GlobalState*>*,
						string
						> tr;
					get<0>(tr) = &observation;
					get<1>(tr) = obs;
					get<2>(tr) = label;
					transitions.insert(&tr);
				}
				
				set<set<GlobalState*>*> unsearchedObservations;
				set_difference(succeedingObservations.begin(), succeedingObservations.end(), searchedObservations.begin(), searchedObservations.end(),
					inserter(unsearchedObservations, unsearchedObservations.end()));
				
				for(set<GlobalState*>* obs : unsearchedObservations){
					observationQueue.push(obs);
				}
			}
		}
	}
	
	o->initState = new LocalState();
	o->initState->id = 0;
	o->initState->name = "";
	for(GlobalState* gs : initialObservation) o->initState->name+=gs->hash+"|"; //for(LocalState* ls : gs->localStatesProjection) s+=ls->name+"|";
	if(o->initState->name.size()>0) o->initState->name.pop_back();
	cout << o->initState->name << endl;
	o->initState->agent = o;
	o->localStates.push_back(o->initState);
	
	cout << "E: " << searchedObservations.size() << "|" << transitions.size() << endl;
	return o;
}

GlobalModel* cloneGlobalModel(LocalModels* localModels, Formula* formula){
	LocalModels clonedLM;
	for(Agent* a : localModels->agents){
		clonedLM.agents.push_back(a->clone());
	}
	GlobalModelGenerator* cloneGenerator = new GlobalModelGenerator();
	cloneGenerator->initModel(&clonedLM, formula);
	cloneGenerator->expandAllStates();
	auto verification = new Verification(cloneGenerator);
	verification->verify();
	
	GlobalModel *out = cloneGenerator->getCurrentGlobalModel();
	
	return out;
}