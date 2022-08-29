#include "Board.h"
#include "Game.h"
#include "globals.h"
#include <iostream>
#include <vector>
#include <map>

using namespace std;

class BoardImpl
{
public:
    BoardImpl(const Game& g);
    void clear();
    void block();
    void unblock();
    bool placeShip(Point topOrLeft, int shipId, Direction dir);
    bool unplaceShip(Point topOrLeft, int shipId, Direction dir);
    void display(bool shotsOnly) const;
    bool attack(Point p, bool& shotHit, bool& shipDestroyed, int& shipId);
    bool allShipsDestroyed() const { return liveShips.empty(); }
    
private:
    vector< vector<char> > gameboard;
    
    const Game& m_game;
    
    map<int, int> liveShips;
};

BoardImpl::BoardImpl(const Game& g)
: m_game(g), liveShips({})
{
    gameboard.resize(g.rows());
    for (int i = 0; i < g.rows(); i++)
    {
        gameboard[i].resize(g.cols());
        
        for (int j = 0; j < g.cols(); j++)
        {
            gameboard[i][j] = '.';
        }
    }
}

void BoardImpl::clear()
{
    for (int i = 0; i < m_game.rows(); i++)
    {
        for (int j = 0; j < m_game.cols(); j++)
        {
            gameboard[i][j] = '.';
        }
    }
}

void BoardImpl::block()
{
    for (int i = 0; i < m_game.rows(); i++)
    {
        for (int j = 0; j < m_game.cols(); j++)
        {
            if (randInt(2) == 0)
            {
                gameboard[i][j] = 'X';
            }
        }
    }
}

void BoardImpl::unblock()
{
    for (int i = 0; i < m_game.rows(); i++)
    {
        for (int j = 0; j < m_game.cols(); j++)
        {
            if (gameboard[i][j] == 'X')
            {
                gameboard[i][j] = '.';
            }
        }
    }
}

bool BoardImpl::placeShip(Point topOrLeft, int shipId, Direction dir)
{
    if (shipId < 0 || shipId > m_game.nShips() - 1)
    {
        return false;
    }
    if (topOrLeft.r < 0 ||  topOrLeft.c < 0 || topOrLeft.r > m_game.rows() - 1 || topOrLeft.c > m_game.cols() - 1)
    {
        return false;
    }
    if (liveShips.find(shipId) != liveShips.end())
    {
        return false;
        // logic behind this is that if the ship is not the most recent one, we shouldn't be placing it
    }
    
    int length = m_game.shipLength(shipId);

    if (dir == HORIZONTAL)
    {
        if (topOrLeft.c + length > m_game.cols())
        {
            return false;
        }
        for (int i = 0; i < length; i++)
        {
            if (gameboard[topOrLeft.r][topOrLeft.c + i] != '.')
            {
                return false;
            }
        }
        for (int i = 0; i < length; i++)
        {
            gameboard[topOrLeft.r][topOrLeft.c + i] = shipId+'0';
        }
    }
    else
    {
        if (topOrLeft.r + length > m_game.rows())
        {
            return false;
        }
        for (int i = 0; i < length; i++)
        {
            if (gameboard[topOrLeft.r + i][topOrLeft.c] != '.')
            {
                return false;
            }
        }
        
        // loop to actually place each point of the ship (any type) onto the board;
        for (int i = 0; i < length; i++)
        {
            gameboard[topOrLeft.r + i][topOrLeft.c] = shipId+'0';
        }
    }
    // Add to ships in play and return true
    liveShips.insert(make_pair(shipId, m_game.shipLength(shipId)));
    return true;
}

bool BoardImpl::unplaceShip(Point topOrLeft, int shipId, Direction dir)
{
    if (shipId < 0 || shipId > m_game.nShips() - 1)
    {
        return false;
    }
    
    // multiple checks to see if it is within the boundaries of the gameboard we created
    if (dir == VERTICAL && topOrLeft.r + m_game.shipLength(shipId) > m_game.rows())
    {
        return false;
    }
    if (dir == HORIZONTAL && topOrLeft.c + m_game.shipLength(shipId) > m_game.cols())
    {
        return false;
    }
    if (topOrLeft.r < 0 || topOrLeft.r > m_game.rows() - 1 || topOrLeft.c < 0 || topOrLeft.c > m_game.cols() - 1)
    {
        return false;
    }

    if (liveShips.find(shipId) == liveShips.end())
    {
        return false;
    }
    
    int length = m_game.shipLength(shipId);

    if (dir == HORIZONTAL)
    {
        for (int i = 0; i < length; i++)
        {
            if (gameboard[topOrLeft.r][topOrLeft.c + i] - 48 != shipId)
            {
                return false;
            }
        }
        for (int i = 0; i < length; i++)
        {
            gameboard[topOrLeft.r][topOrLeft.c + i] = '.';
        }
    }
    else
    {
        for (int i = 0; i < length; i++)
        {
            if (gameboard[topOrLeft.r + i][topOrLeft.c] - 48 != shipId)
            {
                return false;
            }
        }
        for (int i = 0; i < length; i++)
        {
            gameboard[topOrLeft.r + i][topOrLeft.c] = '.';
        }
    }
    liveShips.erase(shipId);
    return true;
}

void BoardImpl::display(bool shotsOnly) const
{
    cout << "  ";
    
    for (int i = 0; i < m_game.cols(); i++)
    {
        cout << i;
    }
    cout << endl;
    //above lines are for printing the first row
    
    for (int i = 0; i < m_game.rows(); i++)
    {
        cout << i << " "; // this line prints out the row number at start of each row
        
        for (int j = 0; j < m_game.cols(); j++)
        {
            if (gameboard[i][j] == '.' || gameboard[i][j] == 'o' || gameboard[i][j] == 'X')
            {
                cout << gameboard[i][j];
            }
            
            else
            {
                if (shotsOnly)
                {
                    cout << '.';
                }
                
                else
                {
                    cout << m_game.shipSymbol(gameboard[i][j] - 48);
                }
            }
        }
        cout << endl;
    }
    // the neseted loops above are for printing out the rest of the board including the ships, attacked+missed spots, and empty spots
}

bool BoardImpl::attack(Point p, bool& shotHit, bool& shipDestroyed, int& shipId)
{
    if (p.r < 0 || p.r > m_game.rows() || p.c < 0 || p.c > m_game.rows())
    {
        shipId = -1;
        return false;
    }
    
    if (gameboard[p.r][p.c] == 'o' || gameboard[p.r][p.c] == 'X')
    {
        shipId = -1;
        return false;
    }

    else if (gameboard[p.r][p.c] == '.')
    {
        shotHit = false;
        shipDestroyed = false;
        gameboard[p.r][p.c] = 'o';
    }
    else
    {
        shotHit = true;
        shipId = gameboard[p.r][p.c] - 48;
        
        gameboard[p.r][p.c] = 'X';
        liveShips[shipId]--;
        
        if (liveShips[shipId] == 0)
        {
            shipDestroyed = true;
            liveShips.erase(shipId);
        }
    }
    return true;
}

//******************** Board functions ********************************

// These functions simply delegate to BoardImpl's functions.
// You probably don't want to change any of this code.

Board::Board(const Game& g)
{
    m_impl = new BoardImpl(g);
}

Board::~Board()
{
    delete m_impl;
}

void Board::clear()
{
    m_impl->clear();
}

void Board::block()
{
    return m_impl->block();
}

void Board::unblock()
{
    return m_impl->unblock();
}

bool Board::placeShip(Point topOrLeft, int shipId, Direction dir)
{
    return m_impl->placeShip(topOrLeft, shipId, dir);
}

bool Board::unplaceShip(Point topOrLeft, int shipId, Direction dir)
{
    return m_impl->unplaceShip(topOrLeft, shipId, dir);
}

void Board::display(bool shotsOnly) const
{
    m_impl->display(shotsOnly);
}

bool Board::attack(Point p, bool& shotHit, bool& shipDestroyed, int& shipId)
{
    return m_impl->attack(p, shotHit, shipDestroyed, shipId);
}

bool Board::allShipsDestroyed() const
{
    return m_impl->allShipsDestroyed();
}
