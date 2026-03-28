#include "Entity.hpp" 
#include <iostream>
#include <ctime>

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


Floor::Floor(int level){
    for(int y=0;y<maxHeight;y++){
        for(int x=0;x<maxWidth;x++){
            grid[y][x] = TileType::WALL;
            fog[y][x] = false;
            

            
        }
    }
    generateLayout();
}


void Floor::generateLayout(){

    for(int y=1;y<maxHeight-1;y++){
        for(int x=1;x<maxWidth-1;x++){
            grid[y][x] =TileType::FLOOR;
            

}
    }

// seperate functions 
void processInput() {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }
        if (inputTimer.getElapsedTime().asMilliseconds() < 150) return;
        int dx = 0, dy = 0;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) dy = -1;
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) dy = 1;
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) dx = -1;
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) dx = 1;
        if (dx != 0 || dy != 0) {
            int nx = pPos.x + dx;
            int ny = pPos.y + dy;
            if (nx >= 0 && nx < MAX_W && ny >= 0 && ny < MAX_H && currentFloor->grid[ny][nx] != WALL) {
                pPos = {nx, ny};
                currentFloor->reveal(pPos.x, pPos.y);
                inputTimer.restart();
            }
        }
    }