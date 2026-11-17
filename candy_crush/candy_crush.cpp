// Candy Crush - SFML 3 skeleton (window, input reading, drawing only)
// Game logic (load, select, swap, matches, fall, refill, score, save)
// is NOT here. Those are the team's own work: see the TODO markers.

#include <SFML/Graphics.hpp>
#include <optional>
#include <string>
#include <cstdlib>
#include <ctime>

// ---------------- Constants ----------------
const int   ROWS = 8, COLS = 8;
const float CELL = 70.f;                      // one cell is 70 x 70 pixels
const float BOARD_X = 20.f, BOARD_Y = 100.f;  // top-left corner of the board
const int   WIN_W = 600, WIN_H = 680;
const float HUD_H = 80.f;

// Game states
const int MENU = 0, PLAYING = 1, GAME_OVER = 2;

// Keys we care about (stored in pressedKey)
const int NO_KEY = 0, KEY_ESCAPE = 1, KEY_ENTER = 2, KEY_UP = 3, KEY_DOWN = 4;

// ---------------- Globals ----------------
int  board[ROWS][COLS];
int  score = 0, moves = 0, target = 0;
int  selRow = -1, selCol = -1;       // -1 means nothing selected
int  gameState = MENU;
int  menuChoice = 0;                 // 0 New Game, 1 Continue, 2 Exit
bool showNoSavedGame = false;        // show "No saved game" on the menu
bool playerWon = false;              // used by the end screen

// Input values, filled by handleInput
int  clickX = 0, clickY = 0;
bool clicked = false;
int  pressedKey = NO_KEY;

// SFML objects
sf::RenderWindow window(sf::VideoMode({ WIN_W, WIN_H }), "Candy Crush");
sf::Font font;

sf::Color colors[7] = {
    sf::Color(25, 25, 45),     // 0: empty
    sf::Color(255, 70, 100),   // 1: red
    sf::Color(255, 150, 0),    // 2: orange
    sf::Color(255, 210, 50),   // 3: yellow
    sf::Color(60, 220, 150),   // 4: green
    sf::Color(70, 120, 255),   // 5: blue
    sf::Color(170, 90, 255)    // 6: purple
};

// ---------------- Small drawing helpers ----------------

// Draws text at (x, y). If centered is true, x is the middle of the text.
void drawText(const std::string& str, float x, float y, unsigned size,
    sf::Color color, bool centered = false)
{
    sf::Text text(font, str, size);
    text.setFillColor(color);
    if (centered)
        x -= text.getLocalBounds().size.x / 2.f;
    text.setPosition({ x, y });
    window.draw(text);
}

// Draws one candy of the given number at row r, column c.
void drawCandy(int number, int r, int c, bool selected)
{
    if (number == 0) return;              // empty cell: no candy

    float x = BOARD_X + c * CELL + 7;
    float y = BOARD_Y + r * CELL + 7;

    sf::CircleShape candy(28.f);
    candy.setFillColor(colors[number]);
    candy.setPosition({ x, y });
    if (selected)
    {
        candy.setOutlineThickness(4.f);
        candy.setOutlineColor(sf::Color::White);
    }
    else
        candy.setOutlineThickness(0.f);
    window.draw(candy);

    // small shine so the candy looks glossy
    sf::CircleShape shine(7.f);
    shine.setFillColor(sf::Color(255, 255, 255, 120));
    shine.setPosition({ x + 10, y + 8 });
    window.draw(shine);
}

// ---------------- Drawing functions ----------------

// Dark square for every cell, then the candy on top
void drawBoard()
{
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            sf::RectangleShape square({ CELL, CELL });
            square.setPosition({ BOARD_X + c * CELL, BOARD_Y + r * CELL });
            square.setFillColor(sf::Color(20, 22, 48));
            square.setOutlineThickness(1.f);          // visible grid
            square.setOutlineColor(sf::Color(60, 65, 120));
            window.draw(square);

            bool selected = (r == selRow && c == selCol);
            drawCandy(board[r][c], r, c, selected);
        }
    }
}

// The 80 pixel strip at the top: score, moves, target
void drawHUD()
{
    sf::RectangleShape strip({ (float)WIN_W, HUD_H });
    strip.setFillColor(sf::Color(25, 28, 60));
    window.draw(strip);

    drawText("SCORE", 100, 12, 16, sf::Color(150, 150, 190), true);
    drawText(std::to_string(score), 100, 36, 28, sf::Color::White, true);

    drawText("MOVES", 300, 12, 16, sf::Color(150, 150, 190), true);
    drawText(std::to_string(moves), 300, 36, 28, sf::Color(255, 210, 50), true);

    drawText("TARGET", 500, 12, 16, sf::Color(150, 150, 190), true);
    drawText(std::to_string(target), 500, 36, 28, sf::Color(60, 220, 150), true);
}

void drawMenu()
{
    drawText("CANDY CRUSH", WIN_W / 2.f, 120, 52, sf::Color(255, 70, 100), true);

    std::string options[3] = { "New Game", "Continue", "Exit" };
    for (int i = 0; i < 3; i++)
    {
        sf::Color col = (i == menuChoice) ? sf::Color(255, 210, 50)
            : sf::Color(200, 200, 230);
        std::string label = (i == menuChoice) ? "> " + options[i] + " <" : options[i];
        drawText(label, WIN_W / 2.f, 280.f + i * 70, 34, col, true);
    }

    if (showNoSavedGame)
        drawText("No saved game", WIN_W / 2.f, 540, 24, sf::Color(255, 70, 100), true);

    drawText("Up / Down to move, Enter to choose", WIN_W / 2.f, 620, 16,
        sf::Color(150, 150, 190), true);
}

void drawEndScreen()
{
    if (playerWon)
        drawText("YOU WIN", WIN_W / 2.f, 220, 60, sf::Color(60, 220, 150), true);
    else
        drawText("GAME OVER", WIN_W / 2.f, 220, 60, sf::Color(255, 70, 100), true);

    drawText("Final score: " + std::to_string(score), WIN_W / 2.f, 330, 32,
        sf::Color::White, true);
    drawText("Press Enter", WIN_W / 2.f, 420, 26, sf::Color(255, 210, 50), true);
}

// ---------------- The three functions of the game loop ----------------

// 1. INPUT: only reads events and saves them in our own variables
void handleInput()
{
    clicked = false;
    pressedKey = NO_KEY;

    while (const std::optional event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
            window.close();
        else if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>())
        {
            if (mouse->button == sf::Mouse::Button::Left)
            {
                clickX = mouse->position.x;   // where the player clicked, in pixels
                clickY = mouse->position.y;
                clicked = true;
            }
        }
        else if (const auto* key = event->getIf<sf::Event::KeyPressed>())
        {
            if (key->code == sf::Keyboard::Key::Escape)     pressedKey = KEY_ESCAPE;
            else if (key->code == sf::Keyboard::Key::Enter) pressedKey = KEY_ENTER;
            else if (key->code == sf::Keyboard::Key::Up)    pressedKey = KEY_UP;
            else if (key->code == sf::Keyboard::Key::Down)  pressedKey = KEY_DOWN;
        }
    }
}

// 2. UPDATE: the game logic goes here (TEAM'S OWN WORK)
void updateGame()
{
    switch (gameState)
    {
    case MENU:
        // TODO: Up/Down changes menuChoice, Enter starts New Game / Continue / Exit
        break;
    case PLAYING:
        // TODO: use clicked, clickX, clickY, pressedKey
        //       selecting, swapping, matches, clearing, falling, refill,
        //       score, moves, win / lose, Esc to save
        break;
    case GAME_OVER:
        // TODO: Enter empties the save and goes back to the menu
        break;
    }
}

// 3. DRAW: clear, draw everything for the current state, display
void drawGame()
{
    window.clear(sf::Color(16, 18, 40));

    switch (gameState)
    {
    case MENU:
        drawMenu();
        break;
    case PLAYING:
        drawHUD();
        drawBoard();
        break;
    case GAME_OVER:
        drawEndScreen();
        break;
    }

    window.display();
}

// ---------------- main ----------------
int main()
{
    srand((unsigned)time(0));
    window.setFramerateLimit(60);   // same speed on every laptop

    // Font: change the path if needed (this one exists on Windows)
    if (!font.openFromFile("C:/Windows/Fonts/arial.ttf"))
        return 1;

    // TODO (team): loadLevel() should fill board, moves and target from level.txt
    gameState = PLAYING;
    while (window.isOpen())
    {
        handleInput();
        updateGame();
        drawGame();
    }
    return 0;
}