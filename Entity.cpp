#include "Entity.hpp" 
#include <iostream>

using namespace std;

void Potion::use(Character& target) {
    target.heal(healAmount); 
}

void Floor::updateFog(int px, int py){
    for(int dy=-1; dy<=1; dy++) {
        for(int dx=-1; dx<=1; dx++) {
            int nx = px + dx; 
            int ny = py + dy;
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

Floor::Floor(int level){
    for(int y=0; y<maxHeight; y++){
        for(int x=0; x<maxWidth; x++){
            grid[y][x] = TileType::WALL;
            fog[y][x] = false;
        }
    }
    generateLayout();
}

void Floor::generateLayout(){
    for(int y=1; y<maxHeight-1; y++){
        for(int x=1; x<maxWidth-1; x++){
            grid[y][x] = TileType::FLOOR;
        }
    }

    
    entrance = sf::Vector2i(3, 3);
    exit = sf::Vector2i(maxWidth - 2, maxHeight - 2);
    
  
    grid[exit.y][exit.x] = TileType::STAIRS;

    
    items.push_back(make_unique<Potion>(3, 3, "healing potion"));
}
