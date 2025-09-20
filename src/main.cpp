#include <raylib.h>
#include<iostream>
#include<deque>
#include <cmath>

using namespace std;

Color green={173,204,98,255};
Color darkgreen={20,160,133,255};

int cellSize = 30;
int cellCount = 30;
int offset = 75;
double lastUpdateTime = 0;

// Global texture for background image
Texture2D backgroundTexture;

// Game states and difficulty levels
enum class GameState {
    MAIN_MENU,
    DIFFICULTY_MENU,
    PLAYING,
    GAME_OVER
};

enum class DifficultyLevel {
    BEGINNER,
    MEDIUM,
    ADVANCED
};

bool Elementindeque(Vector2 element, deque<Vector2> &deque) {
    for (const auto& elem : deque) {
        if (elem.x == element.x && elem.y == element.y) {
            return true;
        }
    }
    return false;
}

bool eventHappened(double interval) {
    double currentTime = GetTime();
    if (currentTime - lastUpdateTime >= interval) {
        lastUpdateTime = currentTime;
        return true;
    }
    return false;
}

// Difficulty manager to handle game speeds
class DifficultyManager {
public:
    static double getSpeed(DifficultyLevel level) {
        switch (level) {
            case DifficultyLevel::BEGINNER:
                return 0.3;  // Slower speed
            case DifficultyLevel::MEDIUM:
                return 0.2;  // Medium speed
            case DifficultyLevel::ADVANCED:
                return 0.1;  // Faster speed
            default:
                return 0.2;
        }
    }
    
    static const char* getDifficultyText(DifficultyLevel level) {
        switch (level) {
            case DifficultyLevel::BEGINNER:
                return "BEGINNER";
            case DifficultyLevel::MEDIUM:
                return "MEDIUM";
            case DifficultyLevel::ADVANCED:
                return "ADVANCED";
            default:
                return "UNKNOWN";
        }
    }
};

// Base GameObject class
class GameObject {
public:
    virtual ~GameObject() = default;
    virtual void Draw() = 0;  // Pure virtual function
    virtual void Update() = 0; // Pure virtual function
};

// Snake class inheriting from GameObject
class Snake : public GameObject {
public:
    deque<Vector2> body = {Vector2{6,9}, Vector2{5,9}, Vector2{4,9}};
    Vector2 direction = {1, 0}; // Initial direction to the right
    bool addSegment = false;
    bool running = true;
    int score = 0;

    void Draw() override {
        for (unsigned int i = 0; i < body.size(); i++) {
            float x = body[i].x;
            float y = body[i].y;
            Rectangle segment = Rectangle{offset + x * cellSize, offset + y * cellSize, (float)cellSize, (float)cellSize};
            DrawRectangleRounded(segment, 0.5, 6, darkgreen);
        }
    }

    void Update() override {
        if (running) {
            body.push_front(Vector2{body[0].x + direction.x, body[0].y + direction.y});
            if (addSegment == true) {
                addSegment = false; // Reset the flag
            } else {
                body.pop_back();
            }
        }
    }

    void Reset() {
        body = {Vector2{6,9}, Vector2{5,9}, Vector2{4,9}};
        direction = {1, 0}; // Reset direction to right
        running = true;
        score = 0;
    }

    void CheckEdgeCollision() {
        if (body[0].x == -1 || body[0].x == cellCount || 
            body[0].y == -1 || body[0].y >= cellCount) {
            running = false; // Just stop the game, don't reset score
        }
    }

    void CheckTailCollision() {
        deque<Vector2> headlessBody = body;
        headlessBody.pop_front();
        if (Elementindeque(body[0], headlessBody)) {
            running = false; // Just stop the game, don't reset score
        }
    }

    void GameOver() {
        running = false; // Only stop the game, keep the score
    }

    void Grow() {
        addSegment = true;
        score++;
    }
};

// Food class inheriting from GameObject
class Food : public GameObject {
public:
    Vector2 position;

    Food(deque<Vector2> snakeBody) {
        position = GetRandomPos(snakeBody);
    }
    ~Food() {}

    void Draw() override {
        DrawRectangle(offset + position.x * cellSize, offset + position.y * cellSize, cellSize, cellSize, RED);
    }

    void Update() override {
        // Food doesn't need to update every frame, but we implement it for consistency
    }

    Vector2 GetRandomCell() {
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        return Vector2{x, y};
    }

    Vector2 GetRandomPos(deque<Vector2> snakeBody) {
        Vector2 position = GetRandomCell();
        while (Elementindeque(position, snakeBody)) {
            position = GetRandomCell();
        }
        return position;
    }

    void Respawn(deque<Vector2> snakeBody) {
        position = GetRandomPos(snakeBody);
    }
};

// Game Manager class to handle states and menu
class GameManager {
private:
    GameState currentState;
    DifficultyLevel selectedDifficulty;
    double gameSpeed;
    Snake snake;
    Food food;
    int finalScore; // Store the final score when game ends
    int highScore;  // Store the highest score achieved
    
public:
    GameManager() : currentState(GameState::MAIN_MENU), selectedDifficulty(DifficultyLevel::MEDIUM), 
                   gameSpeed(DifficultyManager::getSpeed(selectedDifficulty)), food(snake.body), 
                   finalScore(0), highScore(0) {
    }

    void Update() {
        switch (currentState) {
            case GameState::MAIN_MENU:
                UpdateMainMenu();
                break;
            case GameState::DIFFICULTY_MENU:
                UpdateDifficultyMenu();
                break;
            case GameState::PLAYING:
                UpdateGame();
                break;
            case GameState::GAME_OVER:
                UpdateGameOver();
                break;
        }
    }

    void Draw() {
        switch (currentState) {
            case GameState::MAIN_MENU:
                DrawMainMenu();
                break;
            case GameState::DIFFICULTY_MENU:
                DrawDifficultyMenu();
                break;
            case GameState::PLAYING:
                DrawGame();
                break;
            case GameState::GAME_OVER:
                DrawGameOver();
                break;
        }
    }

private:

    void UpdateMainMenu() {
        Vector2 mousePoint = GetMousePosition();
        
        // Define button rectangles
        Rectangle playBtn = { GetScreenWidth()/2.0f - 100, GetScreenHeight()/2.0f - 25, 200, 50 };
        Rectangle exitBtn = { GetScreenWidth()/2.0f - 100, GetScreenHeight()/2.0f + 50, 200, 50 };

        // Check button clicks
        if (CheckCollisionPointRec(mousePoint, playBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            currentState = GameState::DIFFICULTY_MENU;
        }
        else if (CheckCollisionPointRec(mousePoint, exitBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            exit(0);
        }
    }

    void UpdateDifficultyMenu() {
        Vector2 mousePoint = GetMousePosition();
        
        // Define button rectangles
        Rectangle beginnerBtn = { GetScreenWidth()/2.0f - 100, GetScreenHeight()/2.0f - 80, 200, 50 };
        Rectangle mediumBtn = { GetScreenWidth()/2.0f - 100, GetScreenHeight()/2.0f - 20, 200, 50 };
        Rectangle advancedBtn = { GetScreenWidth()/2.0f - 100, GetScreenHeight()/2.0f + 40, 200, 50 };
        Rectangle backBtn = { GetScreenWidth()/2.0f - 100, GetScreenHeight()/2.0f + 100, 200, 50 };

        // Check button clicks
        if (CheckCollisionPointRec(mousePoint, beginnerBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            StartGame(DifficultyLevel::BEGINNER);
        }
        else if (CheckCollisionPointRec(mousePoint, mediumBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            StartGame(DifficultyLevel::MEDIUM);
        }
        else if (CheckCollisionPointRec(mousePoint, advancedBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            StartGame(DifficultyLevel::ADVANCED);
        }
        else if (CheckCollisionPointRec(mousePoint, backBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            currentState = GameState::MAIN_MENU;
        }

        // ESC key to go back to main menu
        if (IsKeyPressed(KEY_ESCAPE)) {
            currentState = GameState::MAIN_MENU;
        }
    }

    void UpdateGame() {
        if (eventHappened(gameSpeed)) {
            snake.Update();
        
            // Check collision between snake and food
            if (snake.body[0].x == food.position.x && snake.body[0].y == food.position.y) {
                snake.Grow();
                food.Respawn(snake.body);
            }
        }
        
        // Handle input
        if (IsKeyPressed(KEY_RIGHT) && snake.direction.x != -1) {
            snake.direction = {1, 0};
            snake.running = true;
        } else if (IsKeyPressed(KEY_LEFT) && snake.direction.x != 1) {
            snake.direction = {-1, 0};
            snake.running = true;
        } else if (IsKeyPressed(KEY_UP) && snake.direction.y != 1) {
            snake.direction = {0, -1};
            snake.running = true;
        } else if (IsKeyPressed(KEY_DOWN) && snake.direction.y != -1) {
            snake.direction = {0, 1};
            snake.running = true;
        }

        // Check collisions
        snake.CheckEdgeCollision();
        snake.CheckTailCollision();

        // Check if game is over
        if (!snake.running) {
            finalScore = snake.score;
            
            if (finalScore > highScore) {
                highScore = finalScore;
            }
            
            currentState = GameState::GAME_OVER;
        }

        // Allow returning to menu with ESC
        if (IsKeyPressed(KEY_ESCAPE)) {
            currentState = GameState::MAIN_MENU;
        }
    }

    void UpdateGameOver() {
        // Press SPACE to restart or ESC to return to menu
        if (IsKeyPressed(KEY_SPACE)) {
            RestartGame();
        }
        else if (IsKeyPressed(KEY_ESCAPE)) {
            currentState = GameState::MAIN_MENU;
        }
    }

    void DrawMainMenu() {
        ClearBackground(GREEN);
        
        // Draw background image if loaded
        if (backgroundTexture.id > 0) {
            // Scale the image to fit the screen while maintaining aspect ratio
            float screenWidth = GetScreenWidth();
            float screenHeight = GetScreenHeight();
            float imageWidth = backgroundTexture.width;
            float imageHeight = backgroundTexture.height;
            
            // Calculate scaling factor to cover the entire screen
            float scaleX = screenWidth / imageWidth;
            float scaleY = screenHeight / imageHeight;
            float scale = fmax(scaleX, scaleY); // Use the larger scale to cover screen
            
            // Calculate position to center the image
            float scaledWidth = imageWidth * scale;
            float scaledHeight = imageHeight * scale;
            float posX = (screenWidth - scaledWidth) / 2;
            float posY = (screenHeight - scaledHeight) / 2;
            
            // Draw the background image
            DrawTextureEx(backgroundTexture, Vector2{posX, posY}, 0.0f, scale, WHITE);
            
            // Add a semi-transparent overlay to make text more readable
            DrawRectangle(0, 0, screenWidth, screenHeight, ColorAlpha(BLACK, 0.3f));
        }
        
        // Draw title with outline for better visibility
        const char* title = "SNAKE GAME";
        int titleWidth = MeasureText(title, 60);
        int titleX = GetScreenWidth()/2 - titleWidth/2;
        int titleY = GetScreenHeight()/4;
        
        // Draw title outline
        DrawText(title, titleX-2, titleY-2, 60, BLACK);
        DrawText(title, titleX+2, titleY-2, 60, BLACK);
        DrawText(title, titleX-2, titleY+2, 60, BLACK);
        DrawText(title, titleX+2, titleY+2, 60, BLACK);
        // Draw title
        DrawText(title, titleX, titleY, 60, WHITE);

        // Draw subtitle
        const char* subtitle = "Welcome to Snake Adventure!";
        int subtitleWidth = MeasureText(subtitle, 25);
        int subtitleX = GetScreenWidth()/2 - subtitleWidth/2;
        int subtitleY = GetScreenHeight()/3;
        
        // Draw subtitle outline
        DrawText(subtitle, subtitleX-1, subtitleY-1, 25, BLACK);
        DrawText(subtitle, subtitleX+1, subtitleY-1, 25, BLACK);
        DrawText(subtitle, subtitleX-1, subtitleY+1, 25, BLACK);
        DrawText(subtitle, subtitleX+1, subtitleY+1, 25, BLACK);
        // Draw subtitle
        DrawText(subtitle, subtitleX, subtitleY, 25, WHITE);

        Vector2 mousePoint = GetMousePosition();
        
        // Define and draw buttons
        Rectangle playBtn = { GetScreenWidth()/2.0f - 100, GetScreenHeight()/2.0f - 25, 200, 50 };
        Rectangle exitBtn = { GetScreenWidth()/2.0f - 100, GetScreenHeight()/2.0f + 50, 200, 50 };

        // Draw buttons
        DrawButton(playBtn, "PLAY", mousePoint, GREEN);
        DrawButton(exitBtn, "EXIT", mousePoint, RED);
        
        // Draw high score
        const char* highScoreText = TextFormat("High Score: %d", highScore);
        int highScoreWidth = MeasureText(highScoreText, 25);
        int highScoreX = GetScreenWidth()/2 - highScoreWidth/2;
        int highScoreY = GetScreenHeight() - 80;
        
        Color highScoreColor = highScore > 0 ? YELLOW : WHITE;
        // Draw high score outline
        DrawText(highScoreText, highScoreX-1, highScoreY-1, 25, BLACK);
        DrawText(highScoreText, highScoreX+1, highScoreY-1, 25, BLACK);
        DrawText(highScoreText, highScoreX-1, highScoreY+1, 25, BLACK);
        DrawText(highScoreText, highScoreX+1, highScoreY+1, 25, BLACK);
        // Draw high score
        DrawText(highScoreText, highScoreX, highScoreY, 25, highScoreColor);

        // Draw instructions
        const char* instruction = "Click PLAY to start or use mouse to navigate";
        int instrWidth = MeasureText(instruction, 18);
        int instrX = GetScreenWidth()/2 - instrWidth/2;
        int instrY = GetScreenHeight() - 40;
        
        // Draw instruction outline
        DrawText(instruction, instrX-1, instrY-1, 18, BLACK);
        DrawText(instruction, instrX+1, instrY-1, 18, BLACK);
        DrawText(instruction, instrX-1, instrY+1, 18, BLACK);
        DrawText(instruction, instrX+1, instrY+1, 18, BLACK);
        // Draw instruction
        DrawText(instruction, instrX, instrY, 18, WHITE);
    }

    void DrawDifficultyMenu() {
        ClearBackground(GREEN);
        
        // Draw background image if loaded
        if (backgroundTexture.id > 0) {
            // Scale the image to fit the screen while maintaining aspect ratio
            float screenWidth = GetScreenWidth();
            float screenHeight = GetScreenHeight();
            float imageWidth = backgroundTexture.width;
            float imageHeight = backgroundTexture.height;
            
            // Calculate scaling factor to cover the entire screen
            float scaleX = screenWidth / imageWidth;
            float scaleY = screenHeight / imageHeight;
            float scale = fmax(scaleX, scaleY); // Use the larger scale to cover screen
            
            // Calculate position to center the image
            float scaledWidth = imageWidth * scale;
            float scaledHeight = imageHeight * scale;
            float posX = (screenWidth - scaledWidth) / 2;
            float posY = (screenHeight - scaledHeight) / 2;
            
            // Draw the background image
            DrawTextureEx(backgroundTexture, Vector2{posX, posY}, 0.0f, scale, WHITE);
            
            // Add a semi-transparent overlay to make text more readable
            DrawRectangle(0, 0, screenWidth, screenHeight, ColorAlpha(BLACK, 0.3f));
        }
        
        // Draw title
        const char* title = "SELECT DIFFICULTY";
        int titleWidth = MeasureText(title, 50);
        int titleX = GetScreenWidth()/2 - titleWidth/2;
        int titleY = GetScreenHeight()/4;
        
        // Draw title outline
        DrawText(title, titleX-2, titleY-2, 50, BLACK);
        DrawText(title, titleX+2, titleY-2, 50, BLACK);
        DrawText(title, titleX-2, titleY+2, 50, BLACK);
        DrawText(title, titleX+2, titleY+2, 50, BLACK);
        // Draw title
        DrawText(title, titleX, titleY, 50, WHITE);

        // Draw subtitle
        const char* subtitle = "Choose your challenge level";
        int subtitleWidth = MeasureText(subtitle, 25);
        int subtitleX = GetScreenWidth()/2 - subtitleWidth/2;
        int subtitleY = GetScreenHeight()/3;
        
        // Draw subtitle outline
        DrawText(subtitle, subtitleX-1, subtitleY-1, 25, BLACK);
        DrawText(subtitle, subtitleX+1, subtitleY-1, 25, BLACK);
        DrawText(subtitle, subtitleX-1, subtitleY+1, 25, BLACK);
        DrawText(subtitle, subtitleX+1, subtitleY+1, 25, BLACK);
        // Draw subtitle
        DrawText(subtitle, subtitleX, subtitleY, 25, WHITE);

        Vector2 mousePoint = GetMousePosition();
        
        // Define and draw difficulty buttons
        Rectangle beginnerBtn = { GetScreenWidth()/2.0f - 100, GetScreenHeight()/2.0f - 80, 200, 50 };
        Rectangle mediumBtn = { GetScreenWidth()/2.0f - 100, GetScreenHeight()/2.0f - 20, 200, 50 };
        Rectangle advancedBtn = { GetScreenWidth()/2.0f - 100, GetScreenHeight()/2.0f + 40, 200, 50 };
        Rectangle backBtn = { GetScreenWidth()/2.0f - 100, GetScreenHeight()/2.0f + 100, 200, 50 };

        // Draw buttons with enhanced visibility
        DrawButton(beginnerBtn, "BEGINNER (Slow)", mousePoint, GREEN);
        DrawButton(mediumBtn, "MEDIUM (Normal)", mousePoint, ORANGE);
        DrawButton(advancedBtn, "ADVANCED (Fast)", mousePoint, RED);
        DrawButton(backBtn, "BACK", mousePoint, GRAY);

        // Draw instructions
        const char* instruction = "Click on a difficulty level to start | ESC: Back";
        int instrWidth = MeasureText(instruction, 18);
        int instrX = GetScreenWidth()/2 - instrWidth/2;
        int instrY = GetScreenHeight() - 60;
        
        // Draw instruction outline
        DrawText(instruction, instrX-1, instrY-1, 18, BLACK);
        DrawText(instruction, instrX+1, instrY-1, 18, BLACK);
        DrawText(instruction, instrX-1, instrY+1, 18, BLACK);
        DrawText(instruction, instrX+1, instrY+1, 18, BLACK);
        // Draw instruction
        DrawText(instruction, instrX, instrY, 18, WHITE);
    }

    void DrawGame() {
        ClearBackground(green);
        
        // Draw game border
        DrawRectangleLinesEx(Rectangle{(float)offset-5, (float)offset-5, 
                                     (float)cellSize*cellCount+10, 
                                     (float)cellSize*cellCount+10}, 5, darkgreen);
        
        // Draw game objects using polymorphism
        snake.Draw();
        food.Draw();
        
        // Draw UI
        DrawText(TextFormat("Score: %d", snake.score), offset-5, offset+cellSize*cellCount+15, 30, darkgreen);
        DrawText(TextFormat("High Score: %d", highScore), offset-5, offset+cellSize*cellCount+50, 20, 
                 snake.score >= highScore && snake.score > 0 ? RED : GRAY); // Highlight if approaching/beating high score
        DrawText(TextFormat("Difficulty: %s", DifficultyManager::getDifficultyText(selectedDifficulty)), 
                offset-5, 20, 20, darkgreen);
        DrawText("ESC: Menu", offset-5, 50, 16, darkgreen);
    }

    void DrawGameOver() {
        ClearBackground(green);
        
        // Draw game over text
        const char* gameOverText = "GAME OVER!";
        int gameOverWidth = MeasureText(gameOverText, 60);
        DrawText(gameOverText, GetScreenWidth()/2 - gameOverWidth/2, GetScreenHeight()/3, 60, RED);

        // Check if it's a new high score
        bool isNewHighScore = (finalScore == highScore && finalScore > 0);
        
        // Draw final score
        const char* scoreText = TextFormat("Final Score: %d", finalScore);
        int scoreWidth = MeasureText(scoreText, 40);
        DrawText(scoreText, GetScreenWidth()/2 - scoreWidth/2, GetScreenHeight()/2, 40, darkgreen);
        
        // Draw high score
        const char* highScoreText = TextFormat("High Score: %d", highScore);
        int highScoreWidth = MeasureText(highScoreText, 30);
        DrawText(highScoreText, GetScreenWidth()/2 - highScoreWidth/2, GetScreenHeight()/2 + 45, 30, 
                 isNewHighScore ? RED : darkgreen);
        
        // Show "NEW HIGH SCORE!" if it's a new record
        if (isNewHighScore) {
            const char* newHighScoreText = "NEW HIGH SCORE!";
            int newHighScoreWidth = MeasureText(newHighScoreText, 35);
            DrawText(newHighScoreText, GetScreenWidth()/2 - newHighScoreWidth/2, GetScreenHeight()/2 + 80, 35, RED);
        }

        // Draw difficulty
        const char* diffText = TextFormat("Difficulty: %s", DifficultyManager::getDifficultyText(selectedDifficulty));
        int diffWidth = MeasureText(diffText, 25);
        DrawText(diffText, GetScreenWidth()/2 - diffWidth/2, GetScreenHeight()/2 + (isNewHighScore ? 120 : 90), 25, darkgreen);

        // Draw instructions
        const char* restartText = "SPACE: Play Again    ESC: Main Menu";
        int restartWidth = MeasureText(restartText, 25);
        DrawText(restartText, GetScreenWidth()/2 - restartWidth/2, GetScreenHeight() - 100, 25, darkgreen);
    }

    void DrawButton(Rectangle bounds, const char* text, Vector2 mousePoint, Color baseColor) {
        bool isHovered = CheckCollisionPointRec(mousePoint, bounds);
        
        // Draw button background with enhanced visibility
        Color buttonColor = isHovered ? ColorAlpha(baseColor, 0.9f) : ColorAlpha(baseColor, 0.7f);
        DrawRectangleRec(bounds, buttonColor);
        DrawRectangleLinesEx(bounds, 3, isHovered ? WHITE : BLACK);
        
        // Add inner border for better definition
        DrawRectangleLinesEx(Rectangle{bounds.x+2, bounds.y+2, bounds.width-4, bounds.height-4}, 1, 
                            isHovered ? ColorAlpha(WHITE, 0.5f) : ColorAlpha(BLACK, 0.3f));
        
        // Draw button text with outline for better visibility
        int textWidth = MeasureText(text, 20);
        int textX = bounds.x + bounds.width/2 - textWidth/2;
        int textY = bounds.y + bounds.height/2 - 10;
        Color textColor = isHovered ? WHITE : BLACK;
        Color outlineColor = isHovered ? BLACK : WHITE;
        
        // Draw text outline
        DrawText(text, textX-1, textY-1, 20, outlineColor);
        DrawText(text, textX+1, textY-1, 20, outlineColor);
        DrawText(text, textX-1, textY+1, 20, outlineColor);
        DrawText(text, textX+1, textY+1, 20, outlineColor);
        // Draw text
        DrawText(text, textX, textY, 20, textColor);
    }

    void StartGame(DifficultyLevel difficulty) {
        selectedDifficulty = difficulty;
        gameSpeed = DifficultyManager::getSpeed(difficulty);
        snake.Reset();
        food.Respawn(snake.body);
        currentState = GameState::PLAYING;
    }

    void RestartGame() {
        snake.Reset();
        food.Respawn(snake.body);
        currentState = GameState::PLAYING;
    }
};

int main() 
{
    cout<<"Starting Snake Game..."<<endl;
    InitWindow(2*offset+cellCount*cellSize, 2*offset+cellCount*cellSize, "Snake Game - Menu");
    SetTargetFPS(60);

    // Load background image
    backgroundTexture = LoadTexture("c:\\Users\\PWIT\\Desktop\\Raylib-CPP-Starter-Template-for-VSCODE-V2-main\\pic.png");

    GameManager gameManager;

    while(!WindowShouldClose()) 
    {
        // Update game logic
        gameManager.Update();

        // Draw everything
        BeginDrawing();
        gameManager.Draw();
        EndDrawing();
    }

    // Unload texture
    if (backgroundTexture.id > 0) {
        UnloadTexture(backgroundTexture);
    }

    CloseWindow();
    return 0;
}