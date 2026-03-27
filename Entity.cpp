#include "Entity.hpp" 
#include <iostream>

using namespace std;
// potion functions
void Potion::use(Character& target) {
    target.heal(healAmount); 
}
// floor functions
void Floor::updateFog(int px, int py){
        for(int dy=-1; dy<=1; dy++) {
            for(int dx=-1; dx<=1; dx++) {
                int nx = px + dx; int ny = py + dy;
                if(nx >=0 && nx < maxWidth && ny >=0 && ny < maxHeight) 
                    fog[ny][nx] = false;
            }
        }
    }


bool Floor::isWalkable(int x, int y) const{
    if (x < 0 || x >= maxWidth || y < 0 || y >= maxHeight) {
        return false;
    }
        return grid[y][x] != TileType::WALL;
    }