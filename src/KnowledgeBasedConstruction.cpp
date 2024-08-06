/**
 * @file KnowledgeBasedConstruction.cpp
 * @brief KBC implementation
 * @author Filip Jamroga
 */

#include "KnowledgeBasedConstruction.hpp"

using namespace::std;

const bool kbc_debug = true;

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
					lt->name.assign(EPSILON);
					lt->localName.assign(EPSILON);
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
	
	//cout << "Epsilon:" << c << endl;
}

Agent* KBCexpansion(GlobalModel *const gm, int agent_id){
	Agent* o = new Agent(agent_id, gm->agents[agent_id]->name);
	
	queue<set<GlobalState*>*> observationQueue;
	set<set<GlobalState*>> searchedObservations;
	set<set<GlobalState*>*> allObservationPointers;
	set<tuple<
		set<GlobalState*>*,
		set<GlobalState*>*,
		string
		>> transitions;
	
	
	//initial state observation -> queue
	set<GlobalState*> initialObservation;
	initialObservation.insert(gm->initState);
	for(pair<Agent*, EpistemicClass*> ee : gm->initState->epistemicClasses){
		//asm("INT3");
		//cout << "A: " << ee.first->id << endl;
		if(ee.first->id == agent_id){
			for(pair<string, GlobalState*> ecs : ee.second->globalStates){
				//cout << "E: " << ecs.first << endl;
				initialObservation.insert(ecs.second);
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
			//cout << " S:" << observation->size() << endl;
			int i = 0;
			for(GlobalState* gs : *observation) {
				i++;
				//cout << " I:" << i++ << " GH:" << gs->hash << endl;
				for(GlobalTransition* gt : gs->globalTransitions){
					for(LocalTransition* lt : gt->localTransitions){
						labels.insert(lt->name);
					}
				}
				if(i==observation->size()) break;
			}
			
			//search results of each action separately
			for(string label : labels){
				//find all global states succeeding the states in our current observation
				set<GlobalState*> succeedingRaw;
				//search for states reached by using the $label transition 
				int i = 0;
				for(GlobalState* gs : *observation){
					i++;
					int j = 0;
					//asm("INT3");
					
					//if(gs == (GlobalState *)0x1) cout << "HIT!" << endl;
					//if(gs == (GlobalState *)0x1) break;//I have no clue why I have to do this, but at this point this seems to be the only solution that works (please send help)
					if(gs->globalTransitions.size()<=0) break;
					for(GlobalTransition* gt : gs->globalTransitions){
						j++;
						bool relevant = false;
						for(LocalTransition* lt : gt->localTransitions) if(lt->agent->id==agent_id && lt->name==label){
							//a local transition is only relevant if it belongs to both the agent in question, and represents the currently searched action
							relevant = true;
							break;
						}
						if(relevant) succeedingRaw.insert(gt->to);
						if(j==gs->globalTransitions.size()) break;
					}
					if(i==observation->size()) break;
				}
				
				//EPSILON greed
				int succeedingSizeLast = 0;
				//int greed = 0;
				while(succeedingSizeLast < succeedingRaw.size()){
					succeedingSizeLast = succeedingRaw.size();
					int i = 0;
					for(GlobalState* gs : succeedingRaw){
						i++;
						int j = 0;
						//asm("INT3");
						
						//if(gs == (GlobalState *)0x1) cout << "HIT!" << endl;
						//if(gs == (GlobalState *)0x1) break;//I have no clue why I have to do this, but at this point this seems to be the only solution that works (please send help)
						if(gs->globalTransitions.size()<=0) break;
						for(GlobalTransition* gt : gs->globalTransitions){
							j++;
							bool relevant = false;
							for(LocalTransition* lt : gt->localTransitions) if(lt->name==EPSILON){
								//a local transition is only relevant if it belongs to both the agent in question, and represents the currently searched action
								relevant = true;
								break;
							}
							if(relevant) succeedingRaw.insert(gt->to);
							//if(relevant) greed++;
							if(j==gs->globalTransitions.size()) break;
						}
						if(i==succeedingRaw.size()) break;
					}
				}
				//cout<<"Greed: "<<greed<<endl;
				
				//if(succeedingRaw.size()==0) continue;
				
				//cout << "SRS:" << succeedingRaw.size() << endl;
				//find all subsets of globalstates, which are indistinguishable from the point of view of the relevant agent
				set<set<GlobalState*>*> succeedingObservations;
				vector<GlobalState*> succeedingRawVector(succeedingRaw.begin(), succeedingRaw.end());
				while(succeedingRawVector.size()>0){
					vector<GlobalState*> succeedingRawVectorNext;
					GlobalState* gs = succeedingRawVector[0];
					
					//if(gs == (GlobalState *)0x1) cout << "HIT!" << endl;
					set<GlobalState*>* nextObservation = new set<GlobalState*>;
					nextObservation->insert(gs);
					allObservationPointers.insert(nextObservation);
					
					for(int i=1; i<succeedingRawVector.size(); i++){
						bool added2obs = false;
						for(pair<Agent*, EpistemicClass*> ee : gs->epistemicClasses){
							if(ee.first->id == agent_id){
								for(pair<string, GlobalState*> ecs : ee.second->globalStates){
									if(ecs.first == succeedingRawVector[i]->hash){
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
					observationQueue.push(nextObservation);
					succeedingRawVector = succeedingRawVectorNext;
				}
				
				//register all found transitions
				for(set<GlobalState*>* obs : succeedingObservations){
					tuple<
						set<GlobalState*>*,
						set<GlobalState*>*,
						string
						> tr;
					get<0>(tr) = observation;
					get<1>(tr) = obs;
					get<2>(tr) = label;
					transitions.insert(tr);
				}
				
				// set<set<GlobalState*>*> unsearchedObservations;
				// set_difference(succeedingObservations.begin(), succeedingObservations.end(), searchedObservations.begin(), searchedObservations.end(),
					// inserter(unsearchedObservations, unsearchedObservations.end()));
				
				// for(set<GlobalState*>* obs : unsearchedObservations){
					// observationQueue.push(obs);
				// }
			}
		}
	}
	
	//Convert all searched observations into local states in the output agent
	map<set<GlobalState*>, int> obs2id;
	int ls_id = 0;
	for(set<GlobalState*> obs : searchedObservations){
		//set<GlobalState*> observation2 = *obs;
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
		//for(GlobalState* gs : obs)ls->name+=gs->hash+"-";
		//if(ls->name.size()>0) ls->name.pop_back();
		//Bind the state to the output agent
		ls->agent = o;
		o->localStates.push_back(ls);
		//increment local state counter
		ls_id++;
	}
	
	//Generate all local transitions and bind them transitons to their local states
	int lt_id=0;
	for(auto tr : transitions){
		LocalTransition* t = new LocalTransition();
		t->id = lt_id;
		t->name.assign(get<2>(tr));
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
	//cout << "!" << endl;
	
	//Tell the agent what it's initial state is
	o->initState = o->localStates[0];
	//cout << o->initState->name << endl;
	
	cout << "E: " << searchedObservations.size() << "|" << transitions.size() << endl;
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
