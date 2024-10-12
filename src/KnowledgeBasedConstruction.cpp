/**
 * @file KnowledgeBasedConstruction.cpp
 * @brief KBC implementation
 * @author Filip Jamroga
 */

#include "KnowledgeBasedConstruction.hpp"
#include "DotGraph.hpp"

//The comment below contains the code for a breakpoint (works only on x86!). Copy without the "//" if you need a breakpoit for debugging somewhere. Delet afterwards!
//asm("INT3");

using namespace::std;

const bool kbc_debug = true;

void ModelDotDump(GlobalModel *const gm, string prefix){
	DotGraph(gm, true).saveToFile("KBCDOT", prefix+"global-");
	for(Agent* a : gm->agents){
		DotGraph(a).saveToFile("KBCDOT", prefix+"local-"+to_string(a->id)+"-");
	}
}

void KBCprojection(GlobalModel *const gm, int agent_id){
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
					lt->name.assign(EPSILON);
					lt->localName.assign(EPSILON);
				}
				
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
	
	DotGraph(gm, true).saveToFile("KBCDOT", "global-epsilon-"+to_string(rand())+"-");
}

bool CheckCommon(std::set<GlobalState*> const& inSetA, std::set<GlobalState*> const& inSetB){
	for(auto el_a : inSetA){
		for(auto el_b :inSetB){
			if(el_a->hash==el_b->hash){
				return true;
			}
		}
	}
	return false;
	//return std::find_first_of (inSetA.begin(), inSetA.end(),
							   //inSetB.begin(), inSetB.end()) != inSetA.end();
}

Agent* KBCexpansion(GlobalModel *const gm, int agent_id){
	Agent* o = new Agent(agent_id, gm->agents[agent_id]->name);
	
	//These are self-explainatory
	queue<set<GlobalState*>*> observationQueue;
	set<set<GlobalState*>> searchedObservations;
	set<set<GlobalState*>*> allObservationPointers;
	int tran = 0;
	//Buffer for holding transition information before we can create the LocalTransition objects
	//Contents: (inObservation, outObservation, localLabel)
	vector<tuple<
		set<GlobalState*>*,
		set<GlobalState*>*,
		string
		>> transitions;
	//Buffer for holding information related to repertoires, similar as above
	//Contents: (localLabel -> (globalLabel -> {outState1, outState2, ...}))
	map<string, map<string, set<GlobalState*>>> repertoireData;
	
	
	//initial state observation -> queue
	set<GlobalState*> initialObservation;
	initialObservation.insert(gm->initState);
	for(pair<Agent*, set<GlobalState*>*> ee : gm->initState->epistemicClassesAllAgents){
		if(ee.first->id == agent_id){
			for(GlobalState* ecs : *ee.second){
				initialObservation.insert(ecs);
			}
		}
	}
	observationQueue.push(&initialObservation);
	
	//For some reason in the below code, C++ is not smart enough to stop itself from iterating past set boundaries, so I have to keep a separate counter for it
	//I assume I am doing something the wrong way here, but for the life of me I cannot find my error
	while(observationQueue.size() > 0){//process all observations in the queue
		set<GlobalState*>* observation = observationQueue.front();	//Get the next element of the queue,...
		observationQueue.pop();										//...remove it from it,...
		if(searchedObservations.count(*observation)==0){			//...check if observation has already been searched...
			searchedObservations.insert(*observation);				//...and add it to the list of searched observations if not.
			
			//get all action labels
			set<string> labels;
			int i = 0;
			for(GlobalState* gs : *observation) {
				i++;
				for(GlobalTransition* gt : gs->globalTransitions){
					for(LocalTransition* lt : gt->localTransitions){
						string name = lt->localName;
						if(name==""){name = lt->name;}
						labels.insert(name);
					}
				}
				if(i==observation->size()) break;
			}
			
			//search results of each action separately
			for(string label : labels){
				//find all global states succeeding the states in our current observation
				set<GlobalState*> succeedingRaw;
				map<string, set<GlobalState*>> localRepertoireData;
				//search for states reached by using the $label transition 
				int i = 0;
				for(GlobalState* gs : *observation){
					i++;
					int j = 0;

					if(gs->globalTransitions.size()<=0) break;
					for(GlobalTransition* gt : gs->globalTransitions){
						j++;
						for(LocalTransition* lt : gt->localTransitions) if(lt->agent->id==agent_id && lt->localName==label){//(lt->name==label || lt->localName==label)){
							//a local transition is only relevant if it belongs to both the agent in question, and represents the currently searched action
							succeedingRaw.insert(gt->to);
							localRepertoireData[lt->name].insert(gt->to);
							break;
						}
						if(j==gs->globalTransitions.size()) break;
					}
					if(i==observation->size()) break;
				}
				
				//EPSILON greed
				int succeedingSizeLast = 0;
				while(succeedingSizeLast < succeedingRaw.size()){
					succeedingSizeLast = succeedingRaw.size();
					int i = 0;
					for(GlobalState* gs : succeedingRaw){
						i++;
						int j = 0;

						if(gs->globalTransitions.size()<=0) break;
						for(GlobalTransition* gt : gs->globalTransitions){
							j++;
							for(LocalTransition* lt : gt->localTransitions) if(lt->name==EPSILON){
								//a local transition is only relevant if it belongs to both the agent in question, and represents the currently searched action
								succeedingRaw.insert(gt->to);
								break;
							}
							if(j==gs->globalTransitions.size()) break;
						}
						if(i==succeedingRaw.size()) break;
					}
				}

				//find all subsets of globalstates, which are indistinguishable from the point of view of the relevant agent
				set<set<GlobalState*>*> succeedingObservations;
				vector<GlobalState*> succeedingRawVector(succeedingRaw.begin(), succeedingRaw.end());
				while(succeedingRawVector.size()>0){
					vector<GlobalState*> succeedingRawVectorNext;
					GlobalState* gs = succeedingRawVector[0];
					
					set<GlobalState*>* nextObservation = new set<GlobalState*>;
					nextObservation->insert(gs);
					allObservationPointers.insert(nextObservation);
					
					for(int i=1; i<succeedingRawVector.size(); i++){
						bool added2obs = false;
						for(pair<Agent*, set<GlobalState*>*> ee : gs->epistemicClassesAllAgents){
							if(ee.first->id == agent_id){
								for(GlobalState* ecs : *ee.second){
									if(ecs->hash == succeedingRawVector[i]->hash){
										added2obs = true;
										nextObservation->insert(succeedingRawVector[i]);
										break;
									}
								}
							}
							if(added2obs) break;
						}
						if(!added2obs) succeedingRawVectorNext.push_back(succeedingRawVector[i]);
					}

					succeedingObservations.insert(nextObservation);
					//register found transition
					transitions.push_back(*(new tuple<set<GlobalState*>*, set<GlobalState*>*,string>()));
					get<0>(transitions.back()) = observation;
					get<1>(transitions.back()) = nextObservation;
					get<2>(transitions.back()) = label;
					observationQueue.push(nextObservation);
					succeedingRawVector = succeedingRawVectorNext;
				}
				
				repertoireData[label] = localRepertoireData;
			}
		}
	}

/*	cout << "RD: " << repertoireData.size() << endl;
	vector<string> keys;
	for (auto & it : repertoireData) {
		keys.push_back(it.first);
		cout << "|-K: " << it.first << endl;
		for (auto & it2 : it.second) {
			cout << "| |-L: " << it2.first << endl;
			for (auto & it3 : it2.second) {
				cout << "| | |-G: " << it3 << endl;
			}
		}
	}*/
	
	//Convert all searched observations into local states in the output agent
	map<set<GlobalState*>, int> obs2id;
	int ls_id = 0;
	for(set<GlobalState*> obs : searchedObservations){
		obs2id[obs] = ls_id;
		
		//Create new local state object
		LocalState* ls = new LocalState();
		//Set its id
		ls->id = ls_id;
		//Generate a name for the new state
		ls->name = "";
		for(GlobalState* gs : obs){
			//Find the local state in the observation that belongs to the agent in focus and set the name of ls as the found state's name
			for(LocalState* x : gs->localStatesProjection){
				if(x->agent->id == agent_id){
					//Copy over the name...
					ls->name+=x->name;
					//... and the variables while we're at it as well! (No need to search again! :D)
					ls->environment = x->environment;
					break;
				}
			}
			//Every instance of the local state belonging to our agent is going to point to the same local state, so there is no need to continue searching
			if(ls->name.size()>0) break;
		}
		//Bind the state to the output agent
		ls->agent = o;
		o->localStates.push_back(ls);
		//increment local state counter
		ls_id++;
	}
	
	//Generate all local transitions and bind them transitons to their local states
	int lt_id=0;
	for(auto tr : transitions){
		for(auto rd : repertoireData[get<2>(tr)]){
			if(true){//if(CheckCommon(rd.second, *get<1>(tr))){
				LocalTransition* t = new LocalTransition();
				t->id = lt_id;
				t->name.assign(rd.first);
				t->localName.assign(get<2>(tr));
				t->isShared = false;
				t->sharedCount = 0;
				/*for(Condition* c : localTransitions[i]->conditions){
					Condition* cc;
					cc->var->name = c->var->name;
					cc->var->initialValue = c->var->initialValue;
					cc->var->persistent = c->var->persistent;
					cc->var->agent = a;
					cc->conditionOperator = c->conditionOperator;
					cc->comparedValue = c->comparedValue;
					t->conditions.insert(cc);
				}*/
				t->agent = o;
				t->from = o->localStates[obs2id[*get<0>(tr)]];
				t->to = o->localStates[obs2id[*get<1>(tr)]];
				o->localTransitions.push_back(t);
				o->localStates[t->from->id]->localTransitions.insert(t);

				lt_id++;
			}
		}
	}
	
	//Tell the agent what it's initial state is
	o->initState = o->localStates[0];
	
	for(auto p : allObservationPointers) delete p;
	return o;
}

GlobalModel* cloneGlobalModel(LocalModels* localModels, Formula* formula){
	LocalModels clonedLM;
	Formula clonedF;
	
	for(Agent* a : localModels->agents){
		clonedLM.agents.push_back(a->clone());
	}
	for(Agent* a : formula->coalition){
		clonedF.coalition.insert(clonedLM.agents[a->id]);
	}
	clonedF.p = formula->p;
	clonedF.isF = formula->isF;
	
	GlobalModelGenerator* cloneGenerator = new GlobalModelGenerator();
	cloneGenerator->initModel(&clonedLM, &clonedF);
	cloneGenerator->expandAllStates();
	//auto verification = new Verification(cloneGenerator);
	//verification->verify();
	
	GlobalModel *out = cloneGenerator->getCurrentGlobalModel();
	
	return out;
}
