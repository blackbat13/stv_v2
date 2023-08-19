/**
 * @file Agent.hpp
 * @brief Class of an agent.
 * Class of an agent.
 */

#ifndef AGENT_H
#define AGENT_H

#include "Common.hpp"

/// @brief Contains all data for a single Agent, including id, name and all of the agents' variables.
class Agent {
    public:
        /// @brief Identifier of the agent.
        int id;

        /// @brief Name of the agent.
        string name;

        /// @brief Variable names for the agent.
        set<Var*> vars;

        /// @brief Constructor for the Agent class, assigning it an id and name.
        /// @param _id Identifier of the new agent.
        /// @param _name Name of the new agent.
        Agent(int _id, string _name):id(_id), name(_name) {};
        
        /// @brief Initial state of the agent.
        LocalState* initState;

        /// @brief Local states for this agent.
        vector<LocalState*> localStates; // localStates[i].id == i

        /// @brief Local transitions for this agent.
        vector<LocalTransition*> localTransitions; // localTransitions[i].id == i
        
        // sprawdź, czy stan nie został już wygenerowany.
        
        LocalState* includesState(LocalState *state);
		
		/// @brief Creates a complete, independent copy of the current Agent and its resources, including the ones located at pointers
		Agent clone();
};

#endif // AGENT_H