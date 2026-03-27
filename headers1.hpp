#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <memory>
using namespace std;
// hey

enum class TileType { WALL, FLOOR, STAIRS, ENTRANCE }; // these are tile types in dungeon


class Entity{
protected:
    int x, y;
    char symbol;
public:
    Entity(int x, int y, char symbol) : x(x), y(y), symbol(symbol) {}
    virtual ~Entity() = default;
    int getX() const {return x;}
    int getY() const {return y;}
    void setPos(int nx, int ny) {x = nx; y = ny;}
};


class Item : public Entity{
protected:
    string name;
public:
    Item(int x, int y, string nam) : Entity(x, y, 'I'), name(nam) {}
    string getName() const {return name;}
    virtual void use() = 0; // for using item 
};


class Potion : public Item{
    int healAmount;
public:
    Potion(int x, int y, string nam) : Item(x, y, "healing potion"), healAmount(20) {}
    void use() override {}
};


class Character : public Entity{
    int hp, maxHp;
public:
    Character(int x, int y, char symbol, int hp) : Entity(x, y, symbol), hp(hp), maxHp(hp) {}
    void takeDamage(int dmg) {hp = max(0, hp - dmg);}
    bool isDead() const {return hp <= 0;}
};


class Player : public Character{
    vector<unique_ptr<Item>> inventory; 
public:
    Player(int x, int y) : Character(x, y, 'P', 100) {}
    void addToInventory(unique_ptr<Item> item) {
        inventory.push_back(std::move(item));
    }
};


class Enemy : public Character{
    int aggroRadius;
public:
    Enemy(int x, int y) : Character(x, y, 'E', 50), aggroRadius(5) {}
    void chasePlayer(int px, int py){}
};


class Floor{
private:
    static const int maxWidth = 30;
    static const int maxHeight = 25;
    TileType grid[maxHeight][maxWidth];
    bool fog[maxHeight][maxWidth];
public:
    vector<unique_ptr<Item>> items;
    vector<unique_ptr<Enemy>> enemies;
    sf::Vector2i entrance, exit;
    Floor(int level){}
    void generateLayout(){}
    void updateFog(int px, int py){
        for(int dy=-1; dy<=1; dy++) {
            for(int dx=-1; dx<=1; dx++) {
                int nx = px + dx; int ny = py + dy;
                if(nx >=0 && nx < maxWidth && ny >=0 && ny < maxHeight) 
                    fog[ny][nx] = false;
            }
        }
    }
    bool isWalkable(int x, int y) const {
        return grid[y][x] != TileType::WALL;
    }
    TileType getTile(int x, int y) const {return grid[y][x];}
    bool isVisible(int x, int y) const {return !fog[y][x];}
};