#include "Game.h"
#include "Board.h"
#include "Player.h"
#include "globals.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <cctype>

using namespace std;

struct Ship
{
    string name; // will stay name
    int s_ID; // changing from id to s_ID
    int s_len; // changing from length to s_len
    char s_symb; // changing from symbol to s_symb
    Ship(string n, int i, int l, char s)
    {
        name = n;
        s_ID = i;
        s_len = l;
        s_symb = s;
    }
};

class GameImpl
{
private:
    int m_rows;
    int m_columns;
    vector<Ship *> shipVector;
public:
    GameImpl(int nRows, int nCols);
    int rows() const;
    int cols() const;
    bool isValid(Point p) const;
    Point randomPoint() const;
    bool addShip(int length, char symbol, string name);
    int nShips() const;
    int shipLength(int shipId) const;
    char shipSymbol(int shipId) const;
    string shipName(int shipId) const;
    Player* play(Player* p1, Player* p2, Board& b1, Board& b2, bool shouldPause);
    ~GameImpl();
};

void waitForEnter()
{
    cout << "Press enter to continue: ";
    cin.ignore(10000, '\n');
}

GameImpl::GameImpl(int nRows, int nCols)
{
    m_rows = nRows;
    m_columns = nCols;
}

GameImpl::~GameImpl()
{
    for (auto it = shipVector.begin(); it != shipVector.end(); )
    {
        delete *it;
        it = shipVector.erase(it);
    }
}

int GameImpl::rows() const
{
    return m_rows;
}

int GameImpl::cols() const
{
    return m_columns;
}

bool GameImpl::isValid(Point p) const
{
    return p.r >= 0  &&  p.r < rows()  &&  p.c >= 0  &&  p.c < cols();
}

Point GameImpl::randomPoint() const
{
    return Point(randInt(rows()), randInt(cols()));
}

bool GameImpl::addShip(int length, char symbol, string name)
{
    Ship* ns = new Ship(name, shipVector.size(), length, symbol);
    shipVector.push_back(ns);
    return true;
}

int GameImpl::nShips() const
{
    return shipVector.size();
}

int GameImpl::shipLength(int shipId) const
{
    return shipVector[shipId]->s_len;
}

char GameImpl::shipSymbol(int shipId) const
{
    return shipVector[shipId]->s_symb;
}

string GameImpl::shipName(int shipId) const
{
    return shipVector[shipId]->name;
}

Player* GameImpl::play(Player* p1, Player* p2, Board& b1, Board& b2, bool shouldPause)
{
    Player *temp1, *temp2;
    Board* cur_board;
    bool p1T = true;
    
    if (!p1->placeShips(b1))
    {
        return nullptr;
    }
    if (!p2->placeShips(b2))
    {
        return nullptr;
    }
    
    while (!b1.allShipsDestroyed() && !b2.allShipsDestroyed())
    {
        bool isHit = false;
        bool isDestroyed = false;
        bool isValidShot = false;
        int shipId = 0;
        
        if (p1T)
        {
            temp1 = p1;
            temp2 = p2;
            cur_board = &b2;
        }
        else
        {
            temp1 = p2;
            temp2 = p1;
            cur_board = &b1;
        }
        cout << temp1->name() << " tur now. Board is now displaying for " << temp2->name() << ":" << endl;
        cur_board->display(temp1->isHuman());
        Point attackCoord = temp1->recommendAttack();
        isValidShot = cur_board->attack(attackCoord, isHit, isDestroyed, shipId);
        temp1->recordAttackResult(attackCoord, isValidShot, isHit, isDestroyed, shipId);
        if (temp1->isHuman() && shipId == -1)
        {
            cout << temp1->name() << " wasted at (" << attackCoord.r << "," << attackCoord.c << ").";
            cout << endl;
        }
        else
        {
            cout << temp1->name() << " attacked (" << attackCoord.r << "," << attackCoord.c << ") and ";
            if (isHit && isDestroyed)
            {
                cout << "destroyed  " << this->shipName(shipId);
            }
            else if (isHit)
            {
                cout << "hit misc.";
            }
            else
            {
                cout << "missed";
            }
            cout << ", which means that :";
            cout << endl;
            cur_board->display(temp1->isHuman());
        }

        p1T = !p1T;
        
        // If one of the player's ships are destroyed break loop
        if (b1.allShipsDestroyed() || b2.allShipsDestroyed())
        {
            break;
        }
        // If pause parameter is true waitForEnter after each turn
        if (shouldPause)
        {
            waitForEnter();
        }
    }
    
    // Set t1 to the winnder
    if (b1.allShipsDestroyed())
        temp1 = p2;
    else
        temp1 = p1;
    
    // Output name and return winner
    cout << temp1->name() << " wins!" << endl;
    return temp1;
}

//******************** Game functions *******************************

// These functions for the most part simply delegate to GameImpl's functions.
// You probably don't want to change any of the code from this point down.

Game::Game(int nRows, int nCols)
{
    if (nRows < 1  ||  nRows > MAXROWS)
    {
        cout << "Number of rows must be >= 1 and <= " << MAXROWS << endl;
        exit(1);
    }
    if (nCols < 1  ||  nCols > MAXCOLS)
    {
        cout << "Number of columns must be >= 1 and <= " << MAXCOLS << endl;
        exit(1);
    }
    m_impl = new GameImpl(nRows, nCols);
}

Game::~Game()
{
    delete m_impl;
}

int Game::rows() const
{
    return m_impl->rows();
}

int Game::cols() const
{
    return m_impl->cols();
}

bool Game::isValid(Point p) const
{
    return m_impl->isValid(p);
}

Point Game::randomPoint() const
{
    return m_impl->randomPoint();
}

bool Game::addShip(int length, char symbol, string name)
{
    if (length < 1)
    {
        cout << "Bad ship length " << length << "; it must be >= 1" << endl;
        return false;
    }
    if (length > rows()  &&  length > cols())
    {
        cout << "Bad ship length " << length << "; it won't fit on the board"
        << endl;
        return false;
    }
    if (!isascii(symbol)  ||  !isprint(symbol))
    {
        cout << "Unprintable character with decimal value " << symbol
        << " must not be used as a ship symbol" << endl;
        return false;
    }
    if (symbol == 'X'  ||  symbol == '.'  ||  symbol == 'o')
    {
        cout << "Character " << symbol << " must not be used as a ship symbol"
        << endl;
        return false;
    }
    int totalOfLengths = 0;
    for (int s = 0; s < nShips(); s++)
    {
        totalOfLengths += shipLength(s);
        if (shipSymbol(s) == symbol)
        {
            cout << "Ship symbol " << symbol
            << " must not be used for more than one ship" << endl;
            return false;
        }
    }
    if (totalOfLengths + length > rows() * cols())
    {
        cout << "Board is too small to fit all ships" << endl;
        return false;
    }
    return m_impl->addShip(length, symbol, name);
}

int Game::nShips() const
{
    return m_impl->nShips();
}

int Game::shipLength(int shipId) const
{
    assert(shipId >= 0  &&  shipId < nShips());
    return m_impl->shipLength(shipId);
}

char Game::shipSymbol(int shipId) const
{
    assert(shipId >= 0  &&  shipId < nShips());
    return m_impl->shipSymbol(shipId);
}

string Game::shipName(int shipId) const
{
    assert(shipId >= 0  &&  shipId < nShips());
    return m_impl->shipName(shipId);
}

Player* Game::play(Player* p1, Player* p2, bool shouldPause)
{
    if (p1 == nullptr  ||  p2 == nullptr  ||  nShips() == 0)
        return nullptr;
    Board b1(*this);
    Board b2(*this);
    return m_impl->play(p1, p2, b1, b2, shouldPause);
}

