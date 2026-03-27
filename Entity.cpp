#include "Entity.hpp" 
#include <iostream>

using namespace std;

void Potion::use(Character& target) {
    target.heal(healAmount); 
}