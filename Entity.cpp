#include "Entity.hpp"
#include <queue>
#include <map>

struct Node {
    Point pos;
    int dist;
    bool operator>(const Node& other) const { return dist > other.dist; }
};

// --- DungeonFloor Implementation ---
DungeonFloor::DungeonFloor() {
    for (int row = 0; row < maxHeight; row++) {
        for (int col = 0; col < maxWidth; col++) {
            grid[row][col] = TileType::WALL;
            fogArray[row][col] = true;
        }
    }
    generateLevel();
}

void DungeonFloor::generateLevel() {
    Point walker = { maxWidth / 2, maxHeight / 2 };
    entrancePosition = walker;
    grid[walker.y][walker.x] = TileType::ENTRANCE;
    std::vector<Point> pathHistory;
    for (int step = 0; step < 200; step++) {
        int direction = rand() % 4;
        if (direction == 0 && walker.y > 1) walker.y--;
        else if (direction == 1 && walker.y < maxHeight - 2) walker.y++;
        else if (direction == 2 && walker.x > 1) walker.x--;
        else if (direction == 3 && walker.x < maxWidth - 2) walker.x++;
        grid[walker.y][walker.x] = TileType::FLOOR;
        pathHistory.push_back(walker);
    }
    exitPosition = pathHistory.back();
    grid[exitPosition.y][exitPosition.x] = TileType::STAIRS;
}

void DungeonFloor::revealArea(int playerX, int playerY) {
    for (int offsetY = -2; offsetY <= 2; offsetY++) {
        for (int offsetX = -2; offsetX <= 2; offsetX++) {
            int scanX = playerX + offsetX;
            int scanY = playerY + offsetY;
            if (scanX >= 0 && scanX < maxWidth && scanY >= 0 && scanY < maxHeight) {
                fogArray[scanY][scanX] = false;
            }
        }
    }
}

// --- Rendering Logic ---
void Entity::draw(sf::RenderWindow& window) {
    sprite.setPosition({(float)position.x * tileSize + tileSize / 2.0f, 
                        (float)position.y * tileSize + tileSize / 2.0f});
    window.draw(sprite);
}

void Item::draw(sf::RenderWindow& window) {
    sf::CircleShape shape(tileSize / 6.f, 3);
    shape.setFillColor(renderColor);
    shape.setPosition({(float)position.x * tileSize + 15, (float)position.y * tileSize + 15});
    window.draw(shape);
}

// --- Enemy AI ---
void Enemy::moveEnemy(Point playerPos, DungeonFloor& currentFloor, const std::vector<std::unique_ptr<Item>>& itemsOnFloor, const std::vector<std::unique_ptr<Enemy>>& otherEnemies){
    int distanceToPlayer = std::abs(playerPos.x - position.x) + std::abs(playerPos.y - position.y);
    if (distanceToPlayer > 0 && distanceToPlayer < 4) {
        int moveX = 0, moveY = 0;
        if (playerPos.x > position.x) moveX = 1; else if (playerPos.x < position.x) moveX = -1;
        else if (playerPos.y > position.y) moveY = 1; else if (playerPos.y < position.y) moveY = -1;
        
        Point targetPosition = { position.x + moveX, position.y + moveY };
        bool isPathClear = (currentFloor.grid[targetPosition.y][targetPosition.x] == TileType::FLOOR);
        for (const auto& item : itemsOnFloor) if (item->getPosition() == targetPosition) isPathClear = false;
        for (const auto& enemy : otherEnemies) if (enemy.get() != this && enemy->getPosition() == targetPosition) isPathClear = false;
        if (isPathClear) position = targetPosition;
    }
}

// --- Game Engine ---
class Game {
    sf::RenderWindow gameWindow;
    sf::Font gameFont;
    sf::Texture playerTex, enemyTex, dummyTex; // Added Textures
    std::unique_ptr<sf::Text> interfaceText;
    std::string activeMessage;
    sf::Clock messageClock, inputCooldownClock;
    std::unique_ptr<DungeonFloor> currentFloor;
    std::unique_ptr<Player> playerEntity;
    std::vector<std::unique_ptr<Enemy>> activeEnemies;
    std::vector<std::unique_ptr<Item>> floorItems;
    int turnCounter = 0;
    int floorLevel = 1;
    bool isGameComplete = false;

public:
    Game(): gameWindow(sf::VideoMode({maxWidth * tileSize, maxHeight * tileSize + 40}), "Random Dungeons") {
        srand((unsigned)time(0));
        
        // Load Textures
        if (!playerTex.loadFromFile("player.png")) std::cerr << "Error loading player.png\n";
        if (!enemyTex.loadFromFile("enemy.png")) std::cerr << "Error loading enemy.png\n";
        
        if (!gameFont.openFromFile("arial.ttf")) std::cerr << "Font file error!\n";
        interfaceText = std::make_unique<sf::Text>(gameFont, "", 20);
        interfaceText->setPosition({10, (float)maxHeight * tileSize + 5});
        resetDungeon();
    }

    void resetDungeon() {
        if (floorLevel > 5) { isGameComplete = true; return; }
        currentFloor = std::make_unique<DungeonFloor>();
        playerEntity = std::make_unique<Player>(currentFloor->entrancePosition.x, currentFloor->entrancePosition.y, playerTex);
        currentFloor->revealArea(playerEntity->getPosition().x, playerEntity->getPosition().y);
        activeEnemies.clear();
        floorItems.clear();
        turnCounter = 0;

        int enemiesPlaced = 0;
        while (enemiesPlaced < 4) {
            int randomX = rand() % maxWidth, randomY = rand() % maxHeight;
            if (currentFloor->grid[randomY][randomX] == TileType::FLOOR && std::abs(randomX - playerEntity->getPosition().x) > 4) {
                activeEnemies.push_back(std::make_unique<Enemy>(randomX, randomY, enemyTex));
                enemiesPlaced++;
            }
        }

        for (int attempt = 0; attempt < 2; attempt++) {
            int randomX = rand() % maxWidth, randomY = rand() % maxHeight;
            if (currentFloor->grid[randomY][randomX] == TileType::FLOOR && rand() % 100 < 40) {
                floorItems.push_back(std::make_unique<Item>(randomX, randomY, "Reveal Potion", dummyTex));
            }
        }
        activeMessage = "Dungeon Floor: " + std::to_string(floorLevel) + " / 5";
    }

    void handleInput() {
        while (const std::optional currentEvent = gameWindow.pollEvent()) {
            if (currentEvent->is<sf::Event::Closed>()) gameWindow.close();
        }
        if (isGameComplete) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) {  // this is at the end for restarting game after compeltion
                floorLevel = 1;
                isGameComplete = false;
                activeMessage = "Restarting...";
                messageClock.restart();
                resetDungeon();
            }
            return;
        }

        if (inputCooldownClock.getElapsedTime().asMilliseconds() < 130) return;
        int moveX = 0, moveY = 0;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) moveY = -1;
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) moveY = 1;
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) moveX = -1;
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) moveX = 1;

        if (moveX != 0 || moveY != 0) {
            Point target = {playerEntity->getPosition().x + moveX, playerEntity->getPosition().y + moveY};
            if (target.x >= 0 && target.x < maxWidth && target.y >= 0 && target.y < maxHeight && 
                currentFloor->grid[target.y][target.x] != TileType::WALL) {
                playerEntity->move(moveX, moveY);
                currentFloor->revealArea(playerEntity->getPosition().x, playerEntity->getPosition().y);
                turnCounter++;
                if (turnCounter % 4 == 0) {
                    for (auto& enemy : activeEnemies) enemy->moveEnemy(playerEntity->getPosition(), *currentFloor, floorItems, activeEnemies);
                }
                inputCooldownClock.restart();
            }
        }
    }

    void updateLogic() {
        if (isGameComplete) { 
            activeMessage = "Dungeon explored! Press R to Restart."; 
            return; 
        }
        if (playerEntity->getPosition() == currentFloor->exitPosition) { 
            floorLevel++; 
            resetDungeon(); 
        }
        // potion pickup logic
        for (auto itemIterator = floorItems.begin(); itemIterator != floorItems.end();) {
            if ((*itemIterator)->getPosition() == playerEntity->getPosition()) {
                activeMessage = "Map Revealed!";
                for (int row = 0; row < maxHeight; row++)
                    for (int col = 0; col < maxWidth; col++)
                        currentFloor->fogArray[row][col] = false;

                messageClock.restart();
                itemIterator = floorItems.erase(itemIterator);
            } else ++itemIterator;
        }

        // this is when enemy collides with player object so floor resets and this is temporary as we didnt implement combat yet
        for (auto& enemy : activeEnemies) {
            if (enemy->getPosition() == playerEntity->getPosition()) {
                activeMessage = "You are caught by monster, restarting floor...";
                messageClock.restart();
                resetDungeon(); 
                return;
            }
        }

        if (messageClock.getElapsedTime().asSeconds() > 2.0f) 
            activeMessage = "Dungeon Floor: " + std::to_string(floorLevel) + " / 5";
    }

    void renderFrame() {
        gameWindow.clear(sf::Color(20, 15, 10));
        sf::RectangleShape tileShape({tileSize - 2.f, tileSize - 2.f});
        for (int row = 0; row < maxHeight; row++) {
            for (int col = 0; col < maxWidth; col++) {
                if (currentFloor->fogArray[row][col]) continue;
                tileShape.setPosition({(float)col * tileSize, (float)row * tileSize});
                if (currentFloor->grid[row][col] == TileType::WALL) tileShape.setFillColor(sf::Color(70, 60, 50));
                else if (currentFloor->grid[row][col] == TileType::STAIRS) tileShape.setFillColor(sf::Color::Blue);
                else tileShape.setFillColor(sf::Color(150, 140, 130));
                gameWindow.draw(tileShape);
            }
        }
        for (auto& item : floorItems) if (!currentFloor->fogArray[item->getPosition().y][item->getPosition().x]) item->draw(gameWindow);
        for (auto& enemy : activeEnemies) if (!currentFloor->fogArray[enemy->getPosition().y][enemy->getPosition().x]) enemy->draw(gameWindow);
        playerEntity->draw(gameWindow);
        
        if (interfaceText) {
            interfaceText->setString(activeMessage);
            gameWindow.draw(*interfaceText);
        }
        gameWindow.display();
    }

    void run() {
        while (gameWindow.isOpen()) {
            handleInput();
            updateLogic();
            renderFrame();
        }
    }
};

int main() {
    Game game;
    game.run();
    return 0;
}