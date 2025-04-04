#include "raylib.h"
#include <stdio.h>
#include <stdlib.h> 

// structure for the falling food items
typedef struct {
    float x, y;
    bool active;
    int textureIndex;  // which image is being used for the food
} Food;

// Game State enum
typedef enum {
    GAME_STATE_HOME,
    GAME_STATE_PLAYING,
    GAME_STATE_GAME_OVER // Added GAME_STATE_GAME_OVER
} GameState;

// Function declarations
void UnloadAllTextures(Texture2D background, Texture2D logo, Texture2D startButton, Texture2D soundButton, Texture2D heart);
void scaleWhenHover(Rectangle startButtonRect, Rectangle soundButtonRect, float *scaleStart, float *scaleSound);
void DrawSprite(Texture2D texture, int frameWidth, int frameHeight, int currentFrame, float spritePosX, float spritePosY, float scaleFactor);
void DrawFood(Texture2D textures[], Food food, float scaleFactor);
void DrawHomePageFood (Texture2D foodTextures[], int numTextures, float scaleFactor, float startX, float startY, float gapX);
void UpdateFood(Food *food, float *fallSpeed, int screenHeight, int *lives, int score); //added score
void UpdateMovement(float *spritePosX, int frameWidth, float scaleFactor);
void UpdateAnimation(int *currentFrame, int *frameCounter, int frameSpeed, int totalFrames);
bool CheckCollision(float spritePosX, float spritePosY, int spriteWidth, int spriteHeight, Food food, int foodWidth, int foodHeight);
void DrawLives(int lives, int screenWidth, int screenHeight, Texture2D heart);
void DrawGameOverScreen(int screenWidth, int screenHeight, int score, Texture2D restartButton, float *restartButtonScale); //added
void scaleGameOverButton(Rectangle restartButtonRect, float *restartButtonScale); //added

//main function
int main() {
    //variable declaration
    const int screenWidth = 1000;
    const int screenHeight = 600;
    GameState gameState = GAME_STATE_HOME; //start at home
    bool soundOn = true;
    int score = 0;  // Score variable
    int lives = 5;

    // initialize the window and audio devie
    InitWindow(screenWidth, screenHeight, "Meow-nster: Eats it all!");
    InitAudioDevice();

    // load images to be used
    Texture2D background = LoadTexture("assets/homepage/sky.png");
    Texture2D logo = LoadTexture("assets/homepage/meownsterlogo.png");
    Texture2D startButton = LoadTexture("assets/homepage/start-button.png");
    Texture2D soundButton = LoadTexture("assets/homepage/sound-on-button.png");
    Texture2D popcat = LoadTexture("assets/popcat_spritesheet.png"); //this is a spritesheet
    Music music = LoadMusicStream("assets/music.mp3"); //source: "Cats" by The Living Tombstone (2023)
    Texture2D heart = LoadTexture("assets/heart.png");
    Texture2D restartButton = LoadTexture("assets/homepage/restart-button.png"); // added, changed filename

    // load array of food images
    Texture2D foodTextures[11]; // Array to store 11 textures
    foodTextures[0] = LoadTexture("assets/food/cake_strawberry.png");
    foodTextures[1] = LoadTexture("assets/food/fruit_banana.png");  
    foodTextures[2] = LoadTexture("assets/food/fruit_apple.png");  
    foodTextures[3] = LoadTexture("assets/food/fruit_watermelon_slice.png");  
    foodTextures[4] = LoadTexture("assets/food/eggs_fried.png");  
    foodTextures[5] = LoadTexture("assets/food/soda_pepsi.png");  
    foodTextures[6] = LoadTexture("assets/food/vegetable_carrot.png");  
    foodTextures[7] = LoadTexture("assets/food/vegetable_corn.png");  
    foodTextures[8] = LoadTexture("assets/food/fruit_strawberry.png");  
    foodTextures[9] = LoadTexture("assets/food/coffee_tea.png");  
    foodTextures[10] = LoadTexture("assets/food/icecream.png");


    // initialize the food
    Food food = {GetRandomValue(50, 950), -50, true, 0};  
    float fallSpeed = 0.15f; 
    float foodScaleFactor = 3.0f; //scaling images to desired size

    // so that hover of buttons is not blocky
    SetTextureFilter(startButton, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(soundButton, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(restartButton, TEXTURE_FILTER_BILINEAR); // added

    //play music on start
    PlayMusicStream(music);

    // variables for button scaling
    float scaleStart = 1.0f; 
    float scaleSound = 1.0f; 
    float restartButtonScale = 0.8f; // added, reduced scale to make button smaller
    Rectangle startButtonRect = {300, 245, startButton.width, startButton.height};
    Rectangle soundButtonRect = {930, 20, soundButton.width, soundButton.height};
    Rectangle restartButtonRect = {screenWidth / 2 - restartButton.width / 2 * restartButtonScale, screenHeight / 2 + 75, restartButton.width * restartButtonScale, restartButton.height * restartButtonScale}; // added, changed position

    // variables for spritesheet animation
    int totalFrames = 6;  // number of frames in the sprite sheet
    int frameWidth = popcat.width / totalFrames; //setting width of frame, that is 1/6 width of the spritehseet
    int frameHeight = popcat.height;
    int currentFrame = 0, frameCounter = 0, frameSpeed = 250;  // frame rate
    float spritePosX = 30.0f, spritePosY = 460.0f, scaleFactor = 0.3f; //starting position of the cat and scaling the image

    while (!WindowShouldClose()) { // Game loop runs forever

        // updating stuff
        UpdateAnimation(&currentFrame, &frameCounter, frameSpeed, totalFrames);
        UpdateMusicStream(music);

        // this function handles hover of start and sound button
        scaleWhenHover(startButtonRect, soundButtonRect, &scaleStart, &scaleSound);
        scaleGameOverButton(restartButtonRect, &restartButtonScale); // added

        // if Start button is pressed
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec(GetMousePosition(), startButtonRect)) {
                gameState = GAME_STATE_PLAYING;  // Start the game
                score = 0;
                lives = 5;
                food.y = -50;
                food.x = GetRandomValue(50, GetScreenWidth() - 50);
                fallSpeed = 0.15f;
            }
        }       


        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawTexture(background, 0, 0, WHITE);

            // draw sound button with scaling effect
            Vector2 soundButtonPosition = {soundButtonRect.x + soundButtonRect.width / 2 - soundButton.width * scaleSound / 2,
                                           soundButtonRect.y + soundButtonRect.height / 2 - soundButton.height * scaleSound / 2};
            DrawTextureEx(soundButton, soundButtonPosition, 0.0f, scaleSound, WHITE);          

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { //if mouse button is pressed we check if user pressed the sound button
            if (CheckCollisionPointRec(GetMousePosition(), soundButtonRect)) {
                // Toggle the sound on and off
                soundOn = !soundOn;

                if (soundOn) {
                    // Load the sound-on button texture
                    soundButton = LoadTexture("assets/homepage/sound-on-button.png");
                    PlayMusicStream(music);
                } else {
                    // Load the sound-off button texture
                    soundButton = LoadTexture("assets/homepage/sound-off-button.png");
                    StopMusicStream(music);
                }
            }
        }

            
    if (gameState == GAME_STATE_HOME) {
    //this is the homepage
        
    // Draw the logo, start button, and sound button
    DrawTexture(logo, 100, 20, WHITE);
    DrawHomePageFood(foodTextures, 11, 3.0f, 130.0f, 195.0f, 22.0f);
    DrawSprite(popcat, frameWidth, frameHeight, currentFrame, spritePosX, spritePosY, scaleFactor); //show sprite

            // draw start button with scaling effect
            Vector2 startButtonPosition = {startButtonRect.x + startButtonRect.width / 2 - startButton.width * scaleStart / 2, 
                                           startButtonRect.y + startButtonRect.height / 2 - startButton.height * scaleStart / 2};
            DrawTextureEx(startButton, startButtonPosition, 0.0f, scaleStart, WHITE);

            
    DrawTextureEx(startButton, startButtonPosition, 0.0f, scaleStart, WHITE);
    DrawTextureEx(soundButton, soundButtonPosition, 0.0f, scaleSound, WHITE);
    } else if (gameState == GAME_STATE_PLAYING) {
    
    // gameplay happens here
        UpdateMovement(&spritePosX, frameWidth, scaleFactor);

        UpdateFood(&food, &fallSpeed, GetScreenHeight(), &lives, score); //added score

        int foodWidth = foodTextures[food.textureIndex].width * foodScaleFactor;
        int foodHeight = foodTextures[food.textureIndex].height * foodScaleFactor;

        DrawFood(foodTextures, food, foodScaleFactor);  // Drawing the selected food
        DrawText(TextFormat("Score: %d", score), 20, 20, 30, WHITE);
        DrawLives(lives, screenWidth, screenHeight, heart);
        
        // Check for collision bw cat and food item
        if (CheckCollision(spritePosX, spritePosY, frameWidth * scaleFactor, frameHeight * scaleFactor, food, foodWidth, foodHeight)) {
            score++;  // increment score
            food.y = -50;  // Reset food position
            food.x = GetRandomValue(50, GetScreenWidth() - 50);
            food.textureIndex = GetRandomValue(0, 10);  // Randomly choose a texture (0-10)
        } 
        if (lives <= 0)
        {
            gameState = GAME_STATE_GAME_OVER;
        }
        DrawSprite(popcat, frameWidth, frameHeight, currentFrame, spritePosX, spritePosY, scaleFactor);
    } else if (gameState == GAME_STATE_GAME_OVER) { // added game over screen
        DrawGameOverScreen(screenWidth, screenHeight, score, restartButton, &restartButtonScale); // added
         if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec(GetMousePosition(), restartButtonRect)) {
                gameState = GAME_STATE_HOME;
                lives = 5;
                score = 0;
                food.y = -50;
                food.x = GetRandomValue(50, GetScreenWidth() - 50);
                fallSpeed = 0.15f;
            }
        }
    }
        EndDrawing();
    }

    // Unload all textures before closing
    UnloadAllTextures(background, logo, startButton, soundButton, heart);
    UnloadTexture(popcat);
     UnloadTexture(restartButton); //added
    for (int i = 0; i < 10; i++) {
        UnloadTexture(foodTextures[i]);  // Unload all food textures
    }
    CloseAudioDevice();
    StopMusicStream(music);

    CloseWindow(); // Close the window
    return 0;
}

// Unload all textures before closing
void UnloadAllTextures(Texture2D background, Texture2D logo, Texture2D startButton, Texture2D soundButton, Texture2D heart) {
    UnloadTexture(background);
    UnloadTexture(logo);
    UnloadTexture(startButton);
    UnloadTexture(soundButton);
    UnloadTexture(heart);
}

// handles scaling when these buttons are hovered
void scaleWhenHover(Rectangle startButtonRect, Rectangle soundButtonRect, float *scaleStart, float *scaleSound) {
    // check for mouse hover on start button
    if (CheckCollisionPointRec(GetMousePosition(), startButtonRect)) {
        *scaleStart = 1.1f; // scale up when hovering over start button
    } else {
        *scaleStart = 1.0f; // reset scale when not hovering
    }

    // check for mouse hover on sound button
    if (CheckCollisionPointRec(GetMousePosition(), soundButtonRect)) {
        *scaleSound = 1.1f; // scale up when hovering over sound button
    } else {
        *scaleSound = 1.0f; // reset scale when not hovering
    }
}

// function to draw the cat sprite
void DrawSprite(Texture2D texture, int frameWidth, int frameHeight, int currentFrame, float spritePosX, float spritePosY, float scaleFactor) {
    Rectangle sourceRec = { frameWidth * currentFrame, 0.0f, frameWidth, frameHeight };
    Rectangle destRec = { spritePosX, spritePosY, frameWidth * scaleFactor, frameHeight * scaleFactor };
    DrawTexturePro(texture, sourceRec, destRec, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
}

// function to update frame animation of the cat
void UpdateAnimation(int *currentFrame, int *frameCounter, int frameSpeed, int totalFrames) {
    (*frameCounter)++;  
    if (*frameCounter >= frameSpeed) {  
        *currentFrame = (*currentFrame + 1) % totalFrames;  
        *frameCounter = 0;  
    }
}

// function to draw the food
void DrawFood(Texture2D textures[], Food food, float scaleFactor) {
    if (food.active) {
        // Draw the food based on the textureIndex
        DrawTextureEx(textures[food.textureIndex], (Vector2){food.x, food.y}, 0.0f, scaleFactor, WHITE);
    }
}

// Function to update the falling food
void UpdateFood(Food *food, float *fallSpeed, int screenHeight, int *lives, int score) { //added score
    if (food->active) {
        food->y += *fallSpeed;  

        // If the food goes off-screen, reset it to the top and randomize its x position
        if (food->y > screenHeight) {  
            food->y = -50;  
            food->x = GetRandomValue(50, GetScreenWidth() - 50);  
            food->textureIndex = GetRandomValue(0, 10);  // Randomly choose a texture (0-10)
            *lives -= 1;
        }
    }

    // Increase fall speed every 10 points, 0.15 is base speed
    *fallSpeed = 0.15f + (score / 10) * 0.02f;
}

// Function for user controls and moving the character 
void UpdateMovement(float *spritePosX, int frameWidth, float scaleFactor) {
    float spriteWidth = frameWidth * scaleFactor;  
    float screenWidth = GetScreenWidth();  

    if (IsKeyDown(KEY_RIGHT) && *spritePosX + spriteWidth < screenWidth) {
        *spritePosX += 0.3f;  
    } 
    if (IsKeyDown(KEY_LEFT) && *spritePosX > 0) {
        *spritePosX -= 0.3f;  
    }
}

// Function to check collision between cat and food
bool CheckCollision(float spritePosX, float spritePosY, int spriteWidth, int spriteHeight, Food food, int foodWidth, int foodHeight) {
    Rectangle catRect = { spritePosX, spritePosY, spriteWidth, spriteHeight };

    // this is the food collision box
    Rectangle foodRect = { food.x + 10, food.y + 10, foodWidth - 20, foodHeight - 20 };

    return CheckCollisionRecs(catRect, foodRect);
}

// Function to draw all food textures with a given scale and position
void DrawHomePageFood (Texture2D foodTextures[], int numTextures, float scaleFactor, float startX, float startY, float gapX) {
    // Loop through the textures array and draw
    for (int i = 0; i < numTextures; i++) {
        // Calculate the X position for each food item based on the index and the gap
        float posX = startX + (i * (foodTextures[i].width * scaleFactor + gapX));

        // Draw the food image at the calculated position with the given scale factor
        DrawTextureEx(foodTextures[i], (Vector2){ posX, startY }, 0.0f, scaleFactor, WHITE);  // Y-position is constant 
    }
}

void DrawLives(int lives, int screenWidth, int screenHeight, Texture2D heart) {
    int startX = 20; // Start X position for the first heart
    int gap = 40;    // Gap between hearts, reduced from 80 to 50
    float scale = 2.0f; // Scale factor for the hearts

    for (int i = 0; i < lives; i++) {
        // Calculate the position with the scaled gap
        Vector2 heartPosition = { startX + i * gap, 60 };
        // Use DrawTextureEx to draw the scaled heart
        DrawTextureEx(heart, heartPosition, 0.0f, scale, WHITE);
    }
}

void DrawGameOverScreen(int screenWidth, int screenHeight, int score, Texture2D restartButton, float *restartButtonScale) { // added
    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.8f));
    DrawText("Game Over", screenWidth / 2 - 100, screenHeight / 2 - 50, 40, WHITE);
    DrawText(TextFormat("Final Score: %d", score), screenWidth / 2 - 110, screenHeight / 2, 30, WHITE);

    //draw restart button
    Rectangle restartButtonRect = {screenWidth / 2 - restartButton.width / 2 * (*restartButtonScale), screenHeight / 2 + 75, restartButton.width * (*restartButtonScale), restartButton.height * (*restartButtonScale)}; // changed
    Vector2 restartButtonPosition = {restartButtonRect.x + restartButtonRect.width / 2 - restartButton.width * (*restartButtonScale) / 2,
                                   restartButtonRect.y + restartButtonRect.height / 2 - restartButton.height * (*restartButtonScale) / 2};
    DrawTextureEx(restartButton, restartButtonPosition, 0.0f, *restartButtonScale, WHITE);
}

void scaleGameOverButton(Rectangle restartButtonRect, float *restartButtonScale) { // added
     if (CheckCollisionPointRec(GetMousePosition(), restartButtonRect)) {
        *restartButtonScale = 0.7f;
    } else {
        *restartButtonScale = 0.6f;
    }
}