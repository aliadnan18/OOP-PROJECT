#include "Entity.hpp"
#include <functional>
#include <queue>
#include <climits>


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

            // fog starts fully hidden
            fogArray[row][col] = 0.0f;
        }
    }
    generateLevel();
}

int steps = 500; // Adjust this value to make the dungeon more or less complex
void DungeonFloor::generateLevel() {
    Point walker = { maxWidth / 2, maxHeight / 2 };
    entrancePosition = walker;
    grid[walker.y][walker.x] = TileType::ENTRANCE;
    std::vector<Point> pathHistory;
    for (int step = 0; step < steps; step++) {
        int direction = rand() % 4;
        if (direction == 0 && walker.y > 1) walker.y--;
        else if (direction == 1 && walker.y < maxHeight - 2) walker.y++;
        else if (direction == 2 && walker.x > 1) walker.x--;
        else if (direction == 3 && walker.x < maxWidth - 2) walker.x++;
        grid[walker.y][walker.x] = TileType::FLOOR;
        pathHistory.push_back(walker);
    }
    if (!pathHistory.empty()) {
        exitPosition = pathHistory.back();
    } else {
        exitPosition = entrancePosition;
    }
    grid[exitPosition.y][exitPosition.x] = TileType::STAIRS;
}

void DungeonFloor::revealArea(int playerX, int playerY, GameMode mode) {

    int radius = 3;

    for (int y = 0; y < maxHeight; y++) {
        for (int x = 0; x < maxWidth; x++) {

            int dx = x - playerX;
            int dy = y - playerY;

            float dist = std::sqrt(dx * dx + dy * dy);

            float visibility = std::max(0.0f, 1.0f - (dist / radius));

            if (mode == GameMode::NORMAL) {
                fogArray[y][x] = std::max(fogArray[y][x], visibility);
            }
            else {
                fogArray[y][x] = visibility;
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
template <typename T>
bool isTileOccupied(const std::vector<std::unique_ptr<T>>& entityList, Point target, const Entity* ignoreEntity = nullptr) {
    for (const auto& entity : entityList) {
        // Skip checking against itself, then check if positions match
        if (entity.get() != ignoreEntity && entity->getPosition() == target) {
            return true; 
        }
    }
    return false;
}

bool inVisionRange(Point enemy, Point player, int range = 6) {
    int dx = enemy.x - player.x;
    int dy = enemy.y - player.y;

    return (dx * dx + dy * dy) <= (range * range);
}

// --- Enemy AI ---
void Enemy::moveEnemy(Point playerPos, DungeonFloor& currentFloor, 
                      const std::vector<std::unique_ptr<Item>>& itemsOnFloor, 
                      const std::vector<std::unique_ptr<Enemy>>& otherEnemies) {
                      
   
    int distances[maxHeight][maxWidth];
    
    Point cameFrom[maxHeight][maxWidth]; 
    
    for(int i = 0; i<maxHeight; ++i){
        for(int j = 0;j < maxWidth ; ++j){
            distances[i][j] = INT_MAX; //so djikstra knows that this path hasnt been explored so far
           
            cameFrom[i][j]= {-1,-1};

        }
    }
    
    distances[position.y][position.x] = 0;
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
    bool canSeePlayer = inVisionRange(position, playerPos, 6);
    bool pathFound = false;

   // THE DIJKSTRA SEARCH LOOP
   if (canSeePlayer) {
   pq.push({position, 0});
    while (!pq.empty()) {
        Node currentNode = pq.top();
        pq.pop();

        
        if (currentNode.pos == playerPos) {
            pathFound = true;
            break;
        }

       
        Point neighbors[4] = {
            {currentNode.pos.x, currentNode.pos.y - 1}, 
            {currentNode.pos.x, currentNode.pos.y + 1}, 
            {currentNode.pos.x - 1, currentNode.pos.y}, 
            {currentNode.pos.x + 1, currentNode.pos.y}  
        };

        for (int i = 0; i < 4; i++) {
            Point nextPos = neighbors[i];

            
            
            bool isPathClear = false; 

            if(nextPos.x >= 0 && nextPos.x < maxWidth && nextPos.y >= 0 && nextPos.y < maxHeight ){
                if(currentFloor.grid[nextPos.y][nextPos.x] != TileType::WALL){
                    

                    if (!isTileOccupied(itemsOnFloor, nextPos) && !isTileOccupied(otherEnemies, nextPos, this)) {
                        isPathClear = true;
                    }

                }
            }

            /*for(auto& items: itemsOnFloor){
                if(items->getPosition() == nextPos){
                    isPathClear = false;
                }
                
            }

            for( auto& enemy:otherEnemies){
                if(enemy.get() != this && enemy->getPosition() == nextPos) isPathClear = false;
            }*/










            if (isPathClear) {
                int newDist = currentNode.dist + 1;
                
                // If we found a faster route to this neighbor
                if (newDist < distances[nextPos.y][nextPos.x]) {
                    distances[nextPos.y][nextPos.x] = newDist;
                    cameFrom[nextPos.y][nextPos.x] = currentNode.pos;
                    pq.push({nextPos, newDist});
                }
            }
        }
    }
    }
    
    // enemy tries to find you 

    if (pathFound) {

    Point tracePos = playerPos;

    // No parent exists
    if (cameFrom[tracePos.y][tracePos.x].x == -1)
        return;

    while (true) {

        Point parent = cameFrom[tracePos.y][tracePos.x];

        // corrupted path protection
        if (parent.x < 0 || parent.y < 0)
            return;

        // reached enemy
        if (parent == position)
            break;

        tracePos = parent;
    }

    position = tracePos;
} else {
        // random no jutsu. It will now just go fully random searching for the player
        
        int direction = rand() % 4;
        int moveX = 0 , moveY = 0;

        if(direction == 0){
            moveY = -1;
        }
        else if(direction == 1){
            moveY = 1;
        }
        else if(direction == 2){
            moveX = -1;
        }
        else if(direction == 3){
            moveX = 1;
        }

        Point target = {position.x + moveX, position.y + moveY};

        if(target.x >=0 && target.x < maxWidth && target.y >= 0 && target.y < maxHeight && currentFloor.grid[target.y][target.x] != TileType::WALL){
            
            /*//item checker
            bool canMove = true;
            for(auto& item : itemsOnFloor ){
                if(item->getPosition() == target){
                    canMove =false;
                }
            }

            //other enemy checker

            for(auto& enemy : otherEnemies){
                if(enemy->getPosition==( target)){


                }
            }*/
            // i found a better way to do this using templates
            if (!isTileOccupied(itemsOnFloor, target) && !isTileOccupied(otherEnemies, target, this)) {
                position = target; 
            }


        }

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
    sf::Clock damageCooldownClock;
    bool playerDead = false;
    int turnCounter = 0;
    int floorLevel = 1;
    bool isGameComplete = false;
    GameState gameState = GameState::MENU;
    GameMode gameMode = GameMode::NORMAL;
    sf::Clock enemyMoveClock;
    float enemyMoveDelay = 0.6f; // enemies move every 0.6 seconds
    sf::RectangleShape damageFlash;
    sf::Clock flashClock;
    bool showFlash = false;

public:
    Game(): gameWindow(sf::VideoMode({maxWidth * tileSize, maxHeight * tileSize + 40}), "Random Dungeons") {
        srand((unsigned)time(0));
        damageFlash.setSize(sf::Vector2f(maxWidth * tileSize, maxHeight * tileSize));
        damageFlash.setFillColor(sf::Color(255, 0, 0, 80)); 
        // Load Textures
        if (!playerTex.loadFromFile("player.png")) std::cerr << "Error loading player.png\n";
        if (!enemyTex.loadFromFile("enemy.png")) std::cerr << "Error loading enemy.png\n";
        
        if (!gameFont.openFromFile("arial.ttf")) std::cerr << "Font file error!\n";
        interfaceText = std::make_unique<sf::Text>(gameFont, "", 20);
        interfaceText->setPosition({10, (float)maxHeight * tileSize + 5});
        resetDungeon();
    }

    void renderMenu() {
    gameWindow.clear(sf::Color(10, 10, 20));
    sf::Text title(gameFont, "RANDOM DUNGEONS", 40);
    title.setPosition({200, 100});
    sf::Text start(gameFont, "Normal Mode(N)", 25);
    start.setPosition({200, 250});
    sf::Text hard(gameFont, "Hard Mode(H)", 25);
    hard.setPosition({200, 300});
    sf::Text exit(gameFont, "EXIT(ESC)", 25);
    exit.setPosition({200, 350});

    gameWindow.draw(title);
    gameWindow.draw(start);
    gameWindow.draw(hard);
    gameWindow.draw(exit);
    gameWindow.display();
    }   

    void resetDungeon() {
    if (floorLevel > 5) {
        gameState = GameState::WIN;
        activeMessage = "SUCCESSFULLY ESCAPED! PRESS R TO RESTART|PRESS M TO RETURN TO MENU";
        return;
    }

    currentFloor = std::make_unique<DungeonFloor>();

    if (!playerEntity) {
        playerEntity = std::make_unique<Player>(
            currentFloor->entrancePosition.x,
            currentFloor->entrancePosition.y,
            playerTex
        );
    } else {
        playerEntity->setPosition(currentFloor->entrancePosition);
        // ❌ NO HP RESET HERE
    }

    currentFloor->revealArea(
        playerEntity->getPosition().x,
        playerEntity->getPosition().y, gameMode
    );

    activeEnemies.clear();
    floorItems.clear();
    turnCounter = 0;

    int enemiesPlaced = 0;
    while (enemiesPlaced < 4) {
        int randomX = rand() % maxWidth;
        int randomY = rand() % maxHeight;

        if (currentFloor->grid[randomY][randomX] == TileType::FLOOR &&
            std::abs(randomX - playerEntity->getPosition().x) > 4) {

            activeEnemies.push_back(
                std::make_unique<Enemy>(randomX, randomY, enemyTex)
            );
            enemiesPlaced++;
        }
    }

    for (int attempt = 0; attempt < 2; attempt++) {
        int randomX = rand() % maxWidth;
        int randomY = rand() % maxHeight;

        if (currentFloor->grid[randomY][randomX] == TileType::FLOOR &&
            rand() % 100 < 40) {

            floorItems.push_back(
                std::make_unique<Item>(randomX, randomY, "Reveal Potion", dummyTex)
            );
        }
    }

    activeMessage = "Dungeon Floor: " + std::to_string(floorLevel) + " / 5";
    }


    void resetGame() {
    floorLevel = 1;
    playerDead = false;
    isGameComplete = false;
    if (playerEntity) {
    playerEntity->resetHealth();
    } 
    resetDungeon();
    }

    void handleInput() {
        if (gameState == GameState::MENU) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::N)) {
            gameMode = GameMode::NORMAL;
            gameState = GameState::PLAYING;
            resetDungeon();
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::H)) {
            gameMode = GameMode::HARD;
            gameState = GameState::PLAYING;
            resetDungeon();
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
            gameWindow.close();
        }

        return;
        }
        if (gameState == GameState::GAME_OVER) {

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) {
                resetGame();
                gameState = GameState::PLAYING;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::M)) {
                resetGame();
                gameState = GameState::MENU;
            }

            return;
        }

        if (gameState == GameState::WIN) {

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) {
                resetGame();
                gameState = GameState::PLAYING;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
                gameState = GameState::MENU;
            }

            return;
        }

       while (const std::optional currentEvent = gameWindow.pollEvent()) {
            if (currentEvent->is<sf::Event::Closed>()) {
            gameWindow.close();
            }
        }
        if (playerDead) {

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) {

                floorLevel = 1;
                playerDead = false;
                resetGame();
                activeMessage = "New Run Started!";
                messageClock.restart();
            }

            return;
        }
        if (isGameComplete) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) {  // this is at the end for restarting game after compeltion
                floorLevel = 1;
                isGameComplete = false;
                activeMessage = "Restarting...";
                messageClock.restart();
                resetGame();
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

            Point target = {
                playerEntity->getPosition().x + moveX,
                playerEntity->getPosition().y + moveY
            };

            bool enemyThere = false;

            for (auto& enemy : activeEnemies) {
                if (enemy->getPosition() == target) {
                    enemyThere = true;
                    break;
                }
            }

            if (target.x >= 0 && target.x < maxWidth &&
                target.y >= 0 && target.y < maxHeight &&
                currentFloor->grid[target.y][target.x] != TileType::WALL &&
                !enemyThere)
            {
                playerEntity->move(moveX, moveY);
                currentFloor->revealArea(playerEntity->getPosition().x,
                                        playerEntity->getPosition().y,
                                        gameMode);

                inputCooldownClock.restart();
            }
        }
        if (enemyMoveClock.getElapsedTime().asSeconds() >= enemyMoveDelay){
            for (auto& enemy : activeEnemies){
                enemy->moveEnemy(
                    playerEntity->getPosition(),
                    *currentFloor,
                    floorItems,
                    activeEnemies
                );
            }

            enemyMoveClock.restart();
        }
    }

    void updateLogic() {
        if (isGameComplete) { 
            activeMessage = "Dungeon explored! Press R to Restart."; 
            return; 
        }
        if (gameMode == GameMode::HARD) {
        currentFloor->revealArea(
            playerEntity->getPosition().x,
            playerEntity->getPosition().y,
            gameMode
        );
        }
        if (playerEntity->getPosition() == currentFloor->exitPosition) { 
            floorLevel++;
            steps -= 50; // increase complexity for next floor 
            resetDungeon(); 
        }
        // potion pickup logic
        for (auto itemIterator = floorItems.begin(); itemIterator != floorItems.end();) {
            if ((*itemIterator)->getPosition() == playerEntity->getPosition()) {
                activeMessage = "Map Revealed!";
                for (int row = 0; row < maxHeight; row++)
                    for (int col = 0; col < maxWidth; col++)
                        currentFloor->fogArray[row][col] = 1.0f;

                messageClock.restart();
                itemIterator = floorItems.erase(itemIterator);
            } else ++itemIterator;
        }

        bool playerHit = false;

        for (auto& enemy : activeEnemies) {

            if (enemy->getPosition() == playerEntity->getPosition()) {

                playerHit = true;
                break;
            }
        }

        if (playerHit &&
            damageCooldownClock.getElapsedTime().asSeconds() > 1.0f) {

            playerEntity->takeDamage(20);
            showFlash = true;
            flashClock.restart();
            damageCooldownClock.restart();

            activeMessage =
                "Player HP: " +
                std::to_string(playerEntity->getHealth());

            messageClock.restart();

            if (playerEntity->isDead()) {
                gameState = GameState::GAME_OVER;
                activeMessage = "GAME OVER! PRESS R TO RESTART|PRESS M TO RETURN TO MENU";
            }
        }

        if (!playerDead &&
    messageClock.getElapsedTime().asSeconds() > 2.0f){
    activeMessage ="Dungeon Floor: " +std::to_string(floorLevel) +" / 5";
    }
}

    void renderFrame() {
        gameWindow.clear(sf::Color(20, 15, 10));
        sf::RectangleShape tileShape({tileSize - 2.f, tileSize - 2.f});
        for (int row = 0; row < maxHeight; row++) {
            for (int col = 0; col < maxWidth; col++) {

                tileShape.setPosition({(float)col * tileSize, (float)row * tileSize});

                sf::Color baseColor;

                if (currentFloor->grid[row][col] == TileType::WALL)
                    baseColor = sf::Color(70, 60, 50);
                else if (currentFloor->grid[row][col] == TileType::STAIRS)
                    baseColor = sf::Color::Blue;
                else
                    baseColor = sf::Color(150, 140, 130);

                // SIMPLE FOG (old style back)
                if (currentFloor->fogArray[row][col] < 0.05f)
                    baseColor = sf::Color(20, 15, 10); // dark fog overlay

                tileShape.setFillColor(baseColor);
                gameWindow.draw(tileShape);
            }
        }
        for (auto& item : floorItems){
            float fog = currentFloor->fogArray[item->getPosition().y][item->getPosition().x];
            if (fog > 0.05f)
            item->draw(gameWindow);
        }
        for (auto& enemy : activeEnemies) {
            float fog = currentFloor->fogArray[enemy->getPosition().y][enemy->getPosition().x];
            if (fog > 0.05f) {
                enemy->draw(gameWindow);
            }
        }
        playerEntity->draw(gameWindow);
        
        if (interfaceText) {
            interfaceText->setString(activeMessage);
            gameWindow.draw(*interfaceText);
        }
              // HP BAR
        float barWidth = 200.f;
        float barHeight = 20.f;

        float healthPercent =
            (float)playerEntity->getHealth() /
            playerEntity->getMaxHealth();

        // background
        sf::RectangleShape backBar({barWidth, barHeight});
        backBar.setFillColor(sf::Color(60, 60, 60));
        backBar.setPosition({10.f, 10.f});

        // green health
        sf::RectangleShape healthBar(
            {barWidth * healthPercent, barHeight}
        );

        healthBar.setFillColor(sf::Color::Green);
        healthBar.setPosition({10.f, 10.f});
        if (showFlash) {
            if (flashClock.getElapsedTime().asMilliseconds() < 120) {
                gameWindow.draw(damageFlash);
            } else {
                showFlash = false;
            }
        }

        gameWindow.draw(backBar);
        gameWindow.draw(healthBar);
        gameWindow.display();
    }

    void run() {
        while (gameWindow.isOpen()) {

            handleInput();

            if (gameState == GameState::MENU) {
                renderMenu();
                continue;
            }

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
