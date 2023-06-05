#include "GlobalTransition.hpp"

atomic_uint32_t GlobalTransition::next_id;

GlobalTransition::GlobalTransition(){
   id = next_id++;
}