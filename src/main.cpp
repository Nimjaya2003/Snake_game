#include <raylib.h>
#include<iostream>
#include<deque>

using namespace std;

Color green={173,204,98,255};
Color darkgreen={20,160,133,255};

int cellSize = 30;
int cellCount = 30;
int offset = 75;
double lastUpdateTime = 0;
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
class Game {
    public:
        Snake snake = Snake();
        Food food = Food(snake.body);
        bool running = true;
        int score = 0;
        void draw() {
            snake.Draw();
            food.Draw();
        }

        void Update() {
            if (running) {
                snake.update();
                CheckCollision();
                CollisionEdge();
                collwithTail();
            }
        }
        void CheckCollision() {
            // Check if the snake's head collides with the food
            if (snake.body[0].x == food.position.x && snake.body[0].y == food.position.y) {
                 // Grow the snake
                food.position = food.GetRandomPos(snake.body);
                snake.addSegment = true; // Set the flag to add a new segment
                score++;
            }
        }
        void CollisionEdge(){
            if (snake.body[0].x == -1 || snake.body[0].x == cellCount || 
                snake.body[0].y == -1 || snake.body[0].y >= cellCount) {
                GameOver(); // Reset direction to right
            }
        }
        void GameOver(){
            snake.Reset();
            food.position = food.GetRandomPos(snake.body);
            running = false;
            score = 0;
        }
        void collwithTail(){
            deque<Vector2> headlessBody = snake.body;
            headlessBody.pop_front();
            if (Elementindeque(snake.body[0], headlessBody)) {
                GameOver();
            }
        }
};
class Snake{

    public:
        deque<Vector2> body={Vector2{6,9}, Vector2{5,9}, Vector2{4,9}};
        Vector2 direction = {1, 0}; // Initial direction to the right
        bool addSegment = false;

        void Draw() {
            for (unsigned int i=0; i<body.size(); i++) {
                float x = body[i].x ;
                float y = body[i].y ;
                Rectangle segment=Rectangle{offset+x* cellSize, offset+y* cellSize, (float)cellSize, (float)cellSize};
                DrawRectangleRounded(segment,0.5,6,darkgreen);
            }
        }
        void update(){
            body.push_front(Vector2{body[0].x + direction.x, body[0].y + direction.y});
            if (addSegment==true) {
                 // Add a new segment at the end
                addSegment = false; // Reset the flag
            }else{
                body.pop_back();
                
            }

            
        }
        void Reset() {
            body={Vector2{6,9}, Vector2{5,9}, Vector2{4,9}};
            
            direction = {1, 0}; // Reset direction to right
            // Reset the flag
        }

};
class Food{
    public:
        Vector2 position;

        Food(deque<Vector2> snakeBody) {
            position = GetRandomPos(snakeBody);
        }
        ~Food() {}
        
        
        void Draw() {
            DrawRectangle(offset+position.x*cellSize, offset+position.y*cellSize, cellSize, cellSize, RED);
        }
        Vector2 GetRandomCell() {
            float x = GetRandomValue(0, cellCount - 1);
            float y = GetRandomValue(0, cellCount - 1);
            return Vector2 {x, y};
        }
        Vector2 GetRandomPos(deque<Vector2> snakeBody) {

            Vector2 position = GetRandomCell();
            while (Elementindeque(position, snakeBody)) {
                position = GetRandomCell();
                
            }
            return position;
            
        }

};




int main() 
{
    cout<<"Starting the game...."<<endl;
    InitWindow(2*offset+cellCount*cellSize, 2*offset+cellCount*cellSize, "My Game Window");
    SetTargetFPS(60); // Set the game to run at 60 frames per second

    Game game = Game();

    while(WindowShouldClose() == false) 
    {
        BeginDrawing();
        if (eventHappened(0.2)){
            game.Update();
        }
        if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1) {
            game.snake.direction = {1, 0};
            game.running = true; // Move right
        } else if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1) {
            game.snake.direction = {-1, 0};
            game.running = true; // Move left
        } else if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1) {
            game.snake.direction = {0, -1};
            game.running = true; // Move up
        } else if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1) {
            game.snake.direction = {0, 1};
            game.running = true; // Move down
        }
        ClearBackground(green);
        DrawRectangleLinesEx(Rectangle{(float)offset-5, (float)offset-5, (float)cellSize*cellCount+10, (float)cellSize*cellCount+10},5, darkgreen);
        DrawText(TextFormat("Score: %d", game.score), offset-5, offset+cellSize*cellCount+10, 40, darkgreen);
        game.draw();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}