#include<iostream>
#include <SFML/Graphics.hpp>
#include <optional>
#include <string>
#include<fstream>
#include <cstdlib>
#include <ctime>

using namespace std;


const float CELL = 70.f;                        // one cell is 70 x 70 pixels
const float BOARD_X = 20.f, BOARD_Y = 100.f;    // top-left corner of the board
const int points = 10;

// Keys jo input se milti hain
const int NO_KEY = 0, ESCAPE = 1, ENTER = 2, UP = 3, DOWN = 4;

// ---------------- Colours ----------------
sf::Color colors[7] = { sf::Color(25, 25, 45),  // 0: empty
                        sf::Color::Red,         // 1
                        sf::Color(255, 150, 0), // 2: orange
                        sf::Color::Yellow,      // 3
                        sf::Color::Green,       // 4
                        sf::Color::Blue,        // 5
                        sf::Color(170, 90, 255) }; // 6: purple

// ---------------- Input ----------------
void readEvents(sf::RenderWindow& window, bool& clicked, int& clickX, int& clickY, int& pressedKey)
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
                clickX = mouse->position.x;
                clickY = mouse->position.y;
                clicked = true;
            }
        }
        else if (const auto* key = event->getIf<sf::Event::KeyPressed>())
        {
            if (key->code == sf::Keyboard::Key::Escape)
                pressedKey = ESCAPE;
            else if (key->code == sf::Keyboard::Key::Enter)
                pressedKey = ENTER;
            else if (key->code == sf::Keyboard::Key::Up)
                pressedKey = UP;
            else if (key->code == sf::Keyboard::Key::Down)
                pressedKey = DOWN;
        }
    }
}

// ---------------- Font aur text ----------------
bool loadFont(sf::Font& font)
{
    return font.openFromFile("arial.ttf");
}

void drawText(sf::RenderWindow& window, const sf::Font& font, const std::string& str,
    float x, float y, unsigned int size, sf::Color color)
{
    sf::Text text(font, str, size);
    text.setFillColor(color);
    text.setPosition({ x, y });
    window.draw(text);
}

void drawTextCentered(sf::RenderWindow& window, const sf::Font& font, const std::string& str,
    float y, unsigned int size, sf::Color color)
{
    sf::Text text(font, str, size);
    text.setFillColor(color);
    sf::FloatRect bounds = text.getLocalBounds();
    text.setPosition({ (600.f - bounds.size.x) / 2.f - bounds.position.x, y });
    window.draw(text);
}

// ---------------- Board ----------------
void drawBoard(sf::RenderWindow& window, int board[8][8], int selRow, int selCol)
{
    sf::RectangleShape square({ CELL, CELL });
    square.setFillColor(sf::Color(15, 15, 35));
    square.setOutlineThickness(1.f);
    square.setOutlineColor(sf::Color(60, 60, 110));

    sf::CircleShape candy(28.f);

    for (int r = 0; r < 8; r++)
    {
        for (int c = 0; c < 8; c++)
        {
            square.setPosition({ BOARD_X + c * CELL, BOARD_Y + r * CELL });
            window.draw(square);

            candy.setFillColor(colors[board[r][c]]);
            candy.setPosition({ BOARD_X + c * CELL + 7, BOARD_Y + r * CELL + 7 });
            if (r == selRow && c == selCol)
            {
                candy.setOutlineThickness(4.f);
                candy.setOutlineColor(sf::Color::White);
            }
            else
                candy.setOutlineThickness(0.f);
            window.draw(candy);
        }
    }
}

// ---------------- HUD ----------------
void drawHUD(sf::RenderWindow& window, const sf::Font& font, int score, int moves, int target)
{
    sf::RectangleShape hud({ 600.f, 80.f });
    hud.setFillColor(sf::Color(25, 25, 55));
    window.draw(hud);

    drawText(window, font, "SCORE", 40.f, 8.f, 16, sf::Color(150, 150, 190));
    drawText(window, font, std::to_string(score), 40.f, 35.f, 28, sf::Color::White);

    drawText(window, font, "MOVES", 260.f, 8.f, 16, sf::Color(150, 150, 190));
    drawText(window, font, std::to_string(moves), 260.f, 35.f, 28, sf::Color::Yellow);

    drawText(window, font, "TARGET", 460.f, 8.f, 16, sf::Color(150, 150, 190));
    drawText(window, font, std::to_string(target), 460.f, 35.f, 28, sf::Color::Green);
}

// ---------------- Menu ----------------
// selected: 0 = New Game, 1 = Continue, 2 = Exit
void drawMenu(sf::RenderWindow& window, const sf::Font& font, int selected, bool showNoSave)
{
    drawTextCentered(window, font, "CANDY CRUSH", 120.f, 56, sf::Color::White);

    const std::string items[3] = { "New Game", "Continue", "Exit" };
    for (int i = 0; i < 3; i++)
    {
        sf::Color col = (i == selected) ? sf::Color::Yellow : sf::Color(170, 170, 200);
        std::string label = (i == selected) ? "> " + items[i] + " <" : items[i];
        drawTextCentered(window, font, label, 280.f + i * 70.f, 36, col);
    }

    if (showNoSave)
        drawTextCentered(window, font, "No saved game", 520.f, 24, sf::Color::Red);
}

// ---------------- YOU WIN / GAME OVER ----------------
void drawEndScreen(sf::RenderWindow& window, const sf::Font& font, bool won, int score)
{
    if (won)
        drawTextCentered(window, font, "YOU WIN", 200.f, 64, sf::Color::Green);
    else
        drawTextCentered(window, font, "GAME OVER", 200.f, 64, sf::Color::Red);

    drawTextCentered(window, font, "Score: " + std::to_string(score), 320.f, 36, sf::Color::White);
    drawTextCentered(window, font, "Press Enter", 420.f, 28, sf::Color(170, 170, 200));
}

// ---------------- Bonus ----------------
void drawHint(sf::RenderWindow& window, int r1, int c1, int r2, int c2)
{
    sf::RectangleShape box({ CELL, CELL });
    box.setFillColor(sf::Color::Transparent);
    box.setOutlineThickness(-4.f);
    box.setOutlineColor(sf::Color(255, 210, 0));

    box.setPosition({ BOARD_X + c1 * CELL, BOARD_Y + r1 * CELL });
    window.draw(box);
    box.setPosition({ BOARD_X + c2 * CELL, BOARD_Y + r2 * CELL });
    window.draw(box);
}

void drawShuffling(sf::RenderWindow& window, const sf::Font& font)
{
    drawTextCentered(window, font, "Shuffling...", 340.f, 44, sf::Color::White);
}


// Windows and game data

sf::RenderWindow window(sf::VideoMode({ 600, 680 }), "Candy Crush");
sf::Font font;

int board[8][8];
int score = 0, moves = 0, target = 0;
int selRow = -1, selCol = -1;
int rows, cols;
int SecondRow = -1;
int SecondCol = -1;
bool clicked = false;
int clickX = 0, clickY = 0, pressedKey = NO_KEY;

// Game states
const int MENU = 0, PLAYING = 1, GAME_OVER = 2;
int gameState = MENU;           
int menuChoice = 0;             
bool showNoSave = false;        
bool won = false;               

void setupGame()
{
    window.setFramerateLimit(60); 
    loadFont(font);
}

//***task2***
int loadlevel()
{
    ifstream fin("levels.txt");
    if (!fin.is_open())
    {
        return -1;
    }
    fin >> rows >> cols;
    fin >> moves >> target;
    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            fin >> board[r][c];
        }
    }
    return 0;
}
// ***TASK 03***
int findMatches();
bool CheckAdjancy();
void SwapCandies();
void CheckMoves();
void CheckWinLose();
// This Function indicates the candy position converting pixels to rows and cols.
void GetCandyPosition(int& row, int& col) {
    row = (clickY - BOARD_Y) / CELL;
    col = (clickX - BOARD_X) / CELL;
}
// This is used for the candies selection.
void MouseClick() {
    // This is the condition that avoid click outside the Board
    if (clickX < BOARD_X || clickX >= BOARD_X + cols * CELL || clickY < BOARD_Y || clickY >= BOARD_Y + rows * CELL)
    {
        return;
    }
    int row, col;
    GetCandyPosition(row, col);
    // This is used for 1st Candy if 1st candy is to be selected.
    if (selRow == -1 && selCol == -1)
    {
        selRow = row;
        selCol = col;
    }
    else
    {
        // This Indicates for 2nd candy selection if 1st is already selected.
        SecondRow = row;
        SecondCol = col;
// If it is neighbour then we will swap else we will not swap and select a new candy
        if (CheckAdjancy())
        {
            SwapCandies();
        }
        else
        {
            selRow = row;
            selCol = col;
        }
    }
}


bool CheckAdjancy()
{
    if ((selRow==SecondRow&&(selCol==SecondCol+1 || selCol+1 == SecondCol))||(selCol==SecondCol&&(selRow==SecondRow+1||selRow+1==SecondRow)))
    {
        return true;
    }
    return false;
}
void SwapCandies()
{
   
    if (CheckAdjancy())
    {
        int temp;
        temp = board[selRow][selCol];
        board[selRow][selCol] = board[SecondRow][SecondCol];
        board[SecondRow][SecondCol] = temp;
        // Checking for if matches doesnot exists.
        if (findMatches() == 0)
        {
            temp = board[selRow][selCol];
            board[selRow][selCol] = board[SecondRow][SecondCol];
            board[SecondRow][SecondCol] = temp;
        }
        selRow = -1;
        selCol = -1;
    }
    if (findMatches() > 0)
    {
        CheckMoves();
    }
    CheckWinLose();
}

//***Task4***
bool marked[8][8];
int findMatches()
{
    int count = 0;
    //making all elements of marked false
    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            marked[r][c] = false;
        }
    }
    // checking for all possible matches in a row
    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols - 2; c++)
        {
            if (board[r][c]!= 0 &&board[r][c] == board[r][c + 1] && board[r][c+1] == board[r][c+2])
            {
                marked[r][c] = true;
                marked[r][c+1] = true;
                marked[r][c + 2] = true;
            }
        }
    }

    //checking for all possible matches in a column
    for (int r = 0; r < rows - 2; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            if (board[r][c]!=0 && board[r][c] == board[r+1][c] && board[r+1][c]==board[r+2][c])
            {
                marked[r][c] = true;
                marked[r+1][c] = true;
                marked[r + 2][c] = true;
            }
        }
    }

    //counting all the marked candies
    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            if (marked[r][c])
                count++;
        }
    }
    return count;
}
// *** Task 5 ***

//clearin the candies from the positions of marked array
void clearCandies()
{
    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            if (marked[r][c])
                board[r][c] = 0;
                score += points;
        }
    }
}

void fallingCandies()
{
    //going one col at a time
    for (int c = 0; c < cols; c++)
    {
        int writeRow = 7;
        for (int r = rows - 1; r >= 0; r--)
        {
            if (board[r][c] != 0)
            {
                board[writeRow][c] = board[r][c];
                writeRow--;
            }
        }
        //making all the cells above writerow empty
        for (int r = 0; r <= writeRow; r++)
        {
            board[r][c] = 0;
        }
    }
}
void refillCandies()
{
    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            if (board[r][c] == 0)
                board[r][c] = rand() % 6 + 1;
        }
    }
}

// ***Task06***
void CheckMoves()
{
        moves--;
}
void CheckWinLose()
{
    if (score >= target )
    {
        won = true;
        gameState = GAME_OVER;
    }
    else if (moves <= 0 )
    {
        won = false;
        gameState = GAME_OVER;
        
    }
}
void handleInput()
{
    readEvents(window, clicked, clickX, clickY, pressedKey);

    switch (gameState)
    {
    case MENU:
        if (pressedKey == UP && menuChoice > 0)
            menuChoice--;
        if (pressedKey == DOWN && menuChoice < 2)
            menuChoice++;
        if (pressedKey == ENTER)
        {
            if (menuChoice == 0)
            {
                
                loadlevel();
                cout << findMatches();
                gameState = PLAYING;
            }
            else if (menuChoice == 1)
            {
                // TODO: Continue: savegame.txt load karein.
                // Agar save na ho to showNoSave = true;
            }
            else
                window.close();
        }
        break;

    case PLAYING:
        // TODO: click se select/swap, Esc se save
        if (clicked)
        {
            MouseClick();
        }
        break;

    case GAME_OVER:
        if (pressedKey == ENTER)
            gameState = MENU;
        break;
    }
}

void updateGame()
{
    // TODO: swap, match, fall, refill, score, win/loss
    
}

void drawGame()
{
    window.clear(sf::Color(15, 15, 35));

    switch (gameState)
    {
    case MENU:
        drawMenu(window, font, menuChoice, showNoSave);
        break;
    case PLAYING:
        drawHUD(window, font, score, moves, target);
        drawBoard(window, board, selRow, selCol);
        break;
    case GAME_OVER:
        drawEndScreen(window, font, won, score);
        break;
    }

    window.display();
}

// =====================================================================
int main()
{
    srand(time(0));
    setupGame();

    while (window.isOpen())
    {
        handleInput();
        updateGame();
        drawGame();
    }
    return 0;
}