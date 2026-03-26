#include <SFML/Graphics.hpp>
#include <vector>
#include <ctime>
#include <iostream>
#include <optional>

using namespace std;

const int TILE_SIZE = 40; 
const int MAX_W = 20;
const int MAX_H = 15;

enum TileType { WALL, FLOOR, STAIRS, ENTRANCE };

struct Point {
    int x, y;
    bool operator==(const Point& other) const { return x == other.x && y == other.y; }
    bool operator!=(const Point& other) const { return !(*this == other); }
};

class DungeonFloor {
public:
    TileType grid[MAX_H][MAX_W];
    bool fog[MAX_H][MAX_W];
    Point entrance, exit;
    int level;

    DungeonFloor(int lvl) : level(lvl) {
        for(int i=0; i<MAX_H; i++) {
            for(int j=0; j<MAX_W; j++) {
                grid[i][j] = WALL;
                fog[i][j] = true;
            }
        }
        generate();
    }

    void generate() {
        Point walker = { MAX_W/2, MAX_H/2 };
        entrance = walker;
        grid[walker.y][walker.x] = ENTRANCE;
        vector<Point> paths;

        for(int i=0; i < 200; i++) {
            int dir = rand() % 4;
            if(dir == 0 && walker.y > 1) walker.y--;
            else if(dir == 1 && walker.y < MAX_H-2) walker.y++;
            else if(dir == 2 && walker.x > 1) walker.x--;
            else if(dir == 3 && walker.x < MAX_W-2) walker.x++;

            if(grid[walker.y][walker.x] == WALL) {
                grid[walker.y][walker.x] = FLOOR;
                paths.push_back(walker);
            }
        }
        exit = paths.back();
        grid[exit.y][exit.x] = STAIRS;
    }

    void reveal(int px, int py) {
        for(int dy=-1; dy<=1; dy++) {
            for(int dx=-1; dx<=1; dx++) {
                int nx = px + dx; int ny = py + dy;
                if(nx >=0 && nx < MAX_W && ny >=0 && ny < MAX_H) 
                    fog[ny][nx] = false;
            }
        }
    }
};

class Game {
private:
    sf::RenderWindow window;
    DungeonFloor* currentFloor;
    Point pPos;
    int floorNum;
    sf::Clock inputTimer;

public:
    Game() : window(sf::VideoMode({MAX_W * TILE_SIZE, MAX_H * TILE_SIZE}), "Persona 3 Prototype (SFML 3)") {
        srand(static_cast<unsigned int>(time(0)));
        floorNum = 1;
        currentFloor = new DungeonFloor(floorNum);
        pPos = currentFloor->entrance;
        currentFloor->reveal(pPos.x, pPos.y);
        window.setFramerateLimit(60); 
    }

    ~Game() {
        delete currentFloor;
    }

    void processInput() {
        // SFML 3.0 way of handling events
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        if (inputTimer.getElapsedTime().asMilliseconds() < 150) return;

        int dx = 0, dy = 0;
        // SFML 3.0 uses sf::Keyboard::Key::[Key]
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

    void update() {
        if (pPos == currentFloor->exit) {
            floorNum++;
            delete currentFloor;
            currentFloor = new DungeonFloor(floorNum);
            pPos = currentFloor->entrance;
            currentFloor->reveal(pPos.x, pPos.y);
        }
    }

    void render() {
        window.clear(sf::Color::Black);
        sf::RectangleShape tile(sf::Vector2f{TILE_SIZE - 2.f, TILE_SIZE - 2.f});

        for (int i = 0; i < MAX_H; i++) {
            for (int j = 0; j < MAX_W; j++) {
                if (currentFloor->fog[i][j]) continue;

                tile.setPosition({static_cast<float>(j * TILE_SIZE), static_cast<float>(i * TILE_SIZE)});
                
                if (currentFloor->grid[i][j] == WALL) tile.setFillColor(sf::Color(40, 40, 45));
                else if (currentFloor->grid[i][j] == STAIRS) tile.setFillColor(sf::Color::Blue);
                else if (currentFloor->grid[i][j] == ENTRANCE) tile.setFillColor(sf::Color::Red);
                else tile.setFillColor(sf::Color(180, 180, 180));
                
                window.draw(tile);
            }
        }

        sf::CircleShape playerChar(TILE_SIZE / 3.0f);
        playerChar.setFillColor(sf::Color::Cyan);
        playerChar.setOrigin({playerChar.getRadius(), playerChar.getRadius()});
        playerChar.setPosition({static_cast<float>(pPos.x * TILE_SIZE + TILE_SIZE / 2), 
                                static_cast<float>(pPos.y * TILE_SIZE + TILE_SIZE / 2)});
        window.draw(playerChar);

        window.display();
    }

    void run() {
        while (window.isOpen()) {
            processInput();
            update();
            render();
        }
    }
};

int main() {
    Game game;
    game.run();
    return 0;
}