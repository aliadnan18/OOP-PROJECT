#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <memory>
#include <ctime>
#include <iostream>
#include <optional>
#include <cmath>

const int tileSize = 40;
const int maxWidth = 20;
const int maxHeight = 15;
enum class TileType { 
    WALL, FLOOR, STAIRS, ENTRANCE };
enum class GameState {
    MENU,
    PLAYING,
    GAME_OVER,
    WIN
};

enum class GameMode {
    NORMAL,
    HARD
};
struct Point {
    int x, y;
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

class DungeonFloor {
public:
    TileType grid[maxHeight][maxWidth];
    float fogArray[maxHeight][maxWidth];
    Point entrancePosition, exitPosition;
    DungeonFloor();
    void generateLevel();
    void revealArea(int playerX, int playerY, GameMode mode);
};

class Entity {
protected:
    Point position;
    sf::Sprite sprite; 
public:
   
    Entity(int x, int y, const sf::Texture& texture) 
        : position({x, y}), sprite(texture)
    {
        // Scale 16x16 sprite to fit roughly 80% of the 40x40 tile
        float scaleFactor = (tileSize * 0.8f) / 16.0f;
        sprite.setScale({scaleFactor, scaleFactor});
        
        // Set origin to center (8, 8 for a 16x16 image)
        sprite.setOrigin({8.0f, 8.0f}); 
    }
    virtual ~Entity() = default;
    Point getPosition() const { return position; }
    virtual void draw(sf::RenderWindow& window)=0;
};

class Item : public Entity {
    std::string itemName;
    sf::Color renderColor; 
public:
    Item(int x, int y, std::string name, const sf::Texture& dummy) 
        : Entity(x, y, dummy), itemName(name), renderColor(sf::Color::Yellow) {}
    void draw(sf::RenderWindow& window) override;
};

class Character : public Entity {
protected:
    int healthPoints;
    int maxHealth;

public:
    Character(int x, int y, const sf::Texture& texture, int health)
        : Entity(x, y, texture),
          healthPoints(health),
          maxHealth(health) {}

    void setPosition(Point newPos) { position = newPos; }

    int getHealth() const { return healthPoints; }
    int getMaxHealth() const { return maxHealth; }

    void takeDamage(int damage) {
        healthPoints -= damage;

        if (healthPoints < 0)
            healthPoints = 0;
    }

    bool isDead() const {
        return healthPoints <= 0;
    }

    void draw(sf::RenderWindow& window) override;
};

class Player : public Character {
public:
    Player(int x, int y, const sf::Texture& tex) : Character(x, y, tex, 100) {}
    void move(int deltaX, int deltaY) { position.x += deltaX; position.y += deltaY; }
    void resetHealth() {
    healthPoints = 100;
    }
};

class Enemy : public Character {
public:
    Enemy(int x, int y, const sf::Texture& tex) : Character(x, y, tex, 30) {}
    void moveEnemy(Point playerPos, DungeonFloor& currentFloor, 
                   const std::vector<std::unique_ptr<Item>>& itemsOnFloor, 
                   const std::vector<std::unique_ptr<Enemy>>& otherEnemies);
};
