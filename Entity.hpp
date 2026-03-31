#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <memory>
#include <ctime>
#include <iostream>
#include <optional>

const int tileSize = 40;
const int maxWidth = 20;
const int maxHeight = 15;
enum class TileType { WALL, FLOOR, STAIRS, ENTRANCE }; // these are the tile types in dungeon

struct Point {  // to represent coordinates in dungeon
    int x, y;
    bool operator==(const Point& other) const{ 
        return x == other.x && y == other.y; 
    }
};


class DungeonFloor {
public:
    TileType grid[maxHeight][maxWidth];
    bool fogArray[maxHeight][maxWidth]; // for our fog system like tile is visible or not
    Point entrancePosition, exitPosition;
    DungeonFloor();
    void generateLevel(); // to make dungeon floor with random walk algorithm 
    void revealArea(int playerX, int playerY); // if you know about persona 3 then you will understand fog mechanic
};


class Entity {
protected:
    Point position;
    sf::Color renderColor;
public:
    Entity(int x, int y, sf::Color color) : position({x, y}), renderColor(color) {}
    virtual ~Entity() = default;
    Point getPosition() const{ 
        return position; 
    }
    virtual void draw(sf::RenderWindow& window) = 0; // this will be overridden by player, enemy and item to display them on floor
};


class Item : public Entity {
    std::string itemName;
public:
    Item(int x, int y, std::string name) : Entity(x, y, sf::Color::Yellow), itemName(name) {}
    std::string getName() const{ 
        return itemName; 
    }
    void draw(sf::RenderWindow& window) override;
};


class Character : public Entity {
protected:
    int healthPoints;
public:
    Character(int x, int y, sf::Color color, int health) : Entity(x, y, color), healthPoints(health) {}
    void setPosition(Point newPos) { 
        position = newPos; 
    }
};


class Player : public Character {
public:
    Player(int x, int y) : Character(x, y, sf::Color::Cyan, 100) {}
    void move(int deltaX, int deltaY){ 
        position.x += deltaX; position.y += deltaY; 
    }
    void draw(sf::RenderWindow& window) override;
};


class Enemy : public Character {
public:
    Enemy(int x, int y) : Character(x, y, sf::Color::Red, 30) {}
    void moveEnemy(Point playerPos, DungeonFloor& currentFloor, const std::vector<std::unique_ptr<Item>>& itemsOnFloor, const std::vector<std::unique_ptr<Enemy>>& otherEnemies);
    void draw(sf::RenderWindow& window) override;
};