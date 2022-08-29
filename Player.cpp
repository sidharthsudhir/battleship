#include "Player.h"
#include "Board.h"
#include "Game.h"
#include "globals.h"
#include <iostream>
#include <string>
#include <stack>

using namespace std;

void yCoord(Point p, vector<Point>& tv)
{
    auto iterator = tv.begin();
    
    for (; iterator != tv.end(); )
    {
        if (iterator->r == p.r && iterator->c == p.c)
        {
            iterator = tvv.erase(it);
        }
        else
        {
            iterator++;
        }
    }
}

//*********************************************************************
//  AwfulPlayer
//*********************************************************************

class AwfulPlayer : public Player
{
public:
    AwfulPlayer(string nm, const Game& g);
    virtual bool placeShips(Board& b);
    virtual Point recommendAttack();
    virtual void recordAttackResult(Point p, bool validShot, bool shotHit,
                                    bool shipDestroyed, int shipId);
    virtual void recordAttackByOpponent(Point p);
private:
    Point prevHitCoord;
};

AwfulPlayer::AwfulPlayer(string nm, const Game& g)
: Player(nm, g), prevHitCoord(0, 0)
{}

bool AwfulPlayer::placeShips(Board& b)
{
    for (int i = 0; i < game().nShips(); i++)
    {
        if ( !b.placeShip(Point(i,0), i, HORIZONTAL))
        {
            return false;
        }
    }
    return true;
}

Point AwfulPlayer::recommendAttack()
{
    if (prevHitCoord.c > 0)
    {
        prevHitCoord.c--;
    }
    else
    {
        prevHitCoord.c = game().cols() - 1;
        
        if (prevHitCoord.r > 0)
        {
            prevHitCoord.r--;
        }
        else
        {
            prevHitCoord.r = game().rows() - 1;
        }
    }
    return prevHitCoord;
}

void AwfulPlayer::recordAttackResult(Point /* p */, bool /* validShot */,
                                     bool /* shotHit */, bool /* shipDestroyed */,
                                     int /* shipId */)
{
}

void AwfulPlayer::recordAttackByOpponent(Point /* p */)
{
}

//*********************************************************************
//  HumanPlayer
//*********************************************************************

bool getLineWithTwoIntegers(int& r, int& c)
{
    bool result(cin >> r >> c);
    if (!result)
        cin.clear();  // clear error state so can do more input operations
    cin.ignore(10000, '\n');
    return result;
}

class HumanPlayer : public Player
{
public:
    HumanPlayer(string nm, const Game& g) : Player(nm, g) {}
    virtual ~HumanPlayer() {}
    virtual bool isHuman() const { return true; }
    virtual bool placeShips(Board& b);
    virtual Point recommendAttack();
    virtual void recordAttackResult(Point p, bool validShot, bool shotHit, bool shipDestroyed, int shipId) { }
    virtual void recordAttackByOpponent(Point p) { }
};

bool HumanPlayer::placeShips(Board& b)
{
    string dction;
    bool allow = false;
    
    int row = -1;
    int column = -1;
    
    Direction dir = HORIZONTAL;
    
    for (int i = 0; i < game().nShips(); i++)
    {
        cout << Player::name() << " there are " << game().nShips() - i << " ships not been placed.";
        cout << endl;
        
        b.display(false);

        while (!allow)
        {
            cout << "Type either h or v for direction of ship " << game().shipName(i) << " ( current ship length - " << game().shipLength(i) << "): ";
            cin >> dction;
            
            if (dction != "h" && dction != "v")
            {
                cout << "Direction must be h or v." << endl;
            }
            else
            {
                allow = true;
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        
        allow = false;
        if (dction == "v")
        {
            dir = VERTICAL;
        }
        else
        {
            dir = HORIZONTAL;
        }

        while (!allow)
        {
            cout << "Enter coordinates of top/left point of the ship's cell (e.g. 9 2): ";
            cin >> row >> column;
            cin.clear();
            if (!b.placeShip(Point(row, column), i, dir))
            {
                cout << "Cannot place ship at this point..." << endl;
            }
            else
            {
                allow = true;
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        allow = false;
    }
    return true;
}

Point HumanPlayer::recommendAttack()
{
    int row = 0, column = 0;
    while (1)
    {
        cout << "Input attacking coordinates (e.g. 1 2): ";
        cin >> row >> column;
        if (!cin.good())
        {
            cin.clear();
            cin.ignore(10000, '\n');
            //cout << endl;
            cout << "Enter two ints in the format shown above: " << endl;
            continue;
        }
        break;
    }
    Point tp(row, column);
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return tp;
}

//*********************************************************************
//  MediocrePlayer
//*********************************************************************

class MediocrePlayer : public Player
{
public:
    MediocrePlayer(string nm, const Game& g);
    virtual bool placeShips(Board& b);
    virtual Point recommendAttack();
    virtual void recordAttackResult(Point p, bool validShot, bool shotHit, bool shipDestroyed, int shipId);
    virtual void recordAttackByOpponent(Point p) { }
    ~MediocrePlayer() { }
    
    // functions written specifically for Mediocre player
    bool mp_placeShipSpec(Board& b, int s_remaining, Point p, Direction d, int id, bool traceback, vector<Point> included, vector<Direction> dirs);
    Point selectCoord();
    void selectCoords(Point p);
    
private:
    Point prevHitCoord;

    // determines random attacking on a coord vs algorithmic calculation on attack
    int randOrNot;

    vector<Point> coords;
    bool gSelCoords;
    vector<Point> selectedCoordVector;
    vector< vector<char> >historyBoard;
};

MediocrePlayer::MediocrePlayer(string nm, const Game& g)
: Player(nm, g), randOrNot(1), prevHitCoord(0, 0), selectedCoordVector({}), gSelCoords(false)
{
   historyBoard.resize(game().rows());
    
    for (int i = 0; i < game().rows(); i++)
    {
       historyBoard[i].resize(game().cols());
        
        for (int j = 0; j < game().cols(); j++)
        {
            coords.push_back(Point(i,j));
            historyBoard[i][j] = '.';
        }
    }
    
}

bool MediocrePlayer::placeShips(Board& b)
{
    bool allow = false;
    int s = 0;
    
    // blocking placing then unblocking like mentioned in the spec
    while (!allow && s < 50)
    {
        b.block();
        allow = mp_placeShipSpec(b, game().nShips(), Point(0,0), HORIZONTAL, 0, false, {}, {});
        b.unblock();
        s++;
    }
    return allow;
}

bool MediocrePlayer::mp_placeShipSpec(Board& b, int s_remaining, Point p, Direction d, int id, bool traceback, vector<Point> included, vector<Direction> dirs)
{
    bool allow;
    
    if (s_remaining == 0)
    {
        return true;
    }
    
    if (p.c > game().cols()-1)
    {
        p.c = 0;
        p.r++;
    }
    
    if (p.r > game().rows()-1 && p.c == 0)
    {
        traceback = true;
    }
    if (included.size() == 0 && traceback)
    {
        return false;
    }
    
    if (!traceback)
    {
        allow = b.placeShip(p, id, d);
        
        if (allow)
        {
            // add both direction and point into respective vectors
            included.push_back(p);
            dirs.push_back(d);
            return mp_placeShipSpec(b, s_remaining-1, Point(0, 0), HORIZONTAL, id+1, traceback, included, dirs);
        }
        else
        {
            if (d == HORIZONTAL)
            {
                d = VERTICAL;
                return mp_placeShipSpec(b, s_remaining, p, d, id, traceback, included, dirs);
            }
            else
            {
                return mp_placeShipSpec(b, s_remaining, Point(p.r, p.c+1), HORIZONTAL, id, traceback, included, dirs);
            }
        }
    }
    else
    {
        int row = included.back().r;
        int column = included.back().c;
        
        Direction dction = dirs.back();
        
        included.pop_back();
        dirs.pop_back();
        
        allow = b.unplaceShip(Point(row, column), id-1, dction);
        traceback = false;
        
        if (dction == HORIZONTAL)
        {
            return mp_placeShipSpec(b, s_remaining + 1, Point(row, column), VERTICAL, id - 1, traceback, included, dirs);
        }
        else
        {
            return mp_placeShipSpec(b, s_remaining + 1, Point(row, column + 1), HORIZONTAL, id - 1, traceback, included, dirs);
        }
    }
}

Point MediocrePlayer::recommendAttack()
{
    if (randOrNot == 1)
    {
        // random attack
        int m = randInt(coords.size());
        Point tp(coords[m].r, coords[m].c);
        yCoord(tp, coords);
        return tp;
    }
    else
    {
        // selected attack
        Point tp = selectCoord();
        yCoord(tp, coords);
        return tp;
    }
}

void MediocrePlayer::recordAttackResult(Point p, bool validShot, bool shotHit, bool shipDestroyed, int shipId)
{
    if (shotHit)
    {
       historyBoard[p.r][p.c] = 'X';
    }
    else
    {
       historyBoard[p.r][p.c] = 'o';
    }
    
    if (randOrNot == 1)
    {
        if (shotHit && !shipDestroyed)
        {
            randOrNot = 2;
            prevHitCoord = p;
            gSelCoords = true;
        }
    }
    else
    {
        if (shotHit && shipDestroyed)
        {
            randOrNot = 1;
        }
    }
}

Point MediocrePlayer::selectCoord()
{
    if (gSelCoords)
    {
        selectCoords(prevHitCoord);
    }
    
    int m = randInt(selectedCoordVector.size());
    
    Point tp(selectedCoordVector[m].r, selectedCoordVector[m].c);
    yCoord(tp, selectedCoordVector);
    
    if (selectedCoordVector.empty())
    {
        randOrNot = 1;
    }
    
    return tp;
}

void MediocrePlayer::selectCoords(Point p)
{
    selectedCoordVector.clear();

    for (int m = 1; m < 5; m++)
    {
        if (p.r - m >= 0 &&historyBoard[p.r - m][p.c] == '.')
        {
            Point tp(p.r - m, p.c);
            selectedCoordVector.push_back(tp);
        }
        if (p.r + m <= game().rows() - 1 &&historyBoard[p.r + m][p.c] == '.')
        {
            Point tp(p.r + m, p.c);
            selectedCoordVector.push_back(tp);
        }
        if (p.c - m >= 0 &&historyBoard[p.r][p.c - m] == '.')
        {
            Point tp(p.r, p.c - m);
            selectedCoordVector.push_back(tp);
        }
        if (p.c + m <= game().cols() - 1 &&historyBoard[p.r][p.c + m] == '.')
        {
            Point tp(p.r, p.c + m);
            selectedCoordVector.push_back(tp);
        }
    }
    gSelCoords = false;
}


//*********************************************************************
//  GoodPlayer
//*********************************************************************

class GoodPlayer : public Player
{
public:
    
    GoodPlayer(string nm, const Game& g);
    virtual bool placeShips(Board& b);
    virtual Point recommendAttack();
    virtual void recordAttackResult(Point p, bool validShot, bool shotHit, bool shipDestroyed, int shipId);
    virtual void recordAttackByOpponent(Point p) { }
    void incCoordsToAttack(Point p);
    ~GoodPlayer() {}
    
private:
    vector<Point> coords;
    int state;
    stack<Point> pointsToAttack;
    vector< vector<char> >historyBoard;
};

GoodPlayer::GoodPlayer(string nm, const Game& g)
: Player(nm, g), state(1)
{
    historyBoard.resize(game().rows());
    
    for (int i = 0; i < game().rows(); i++)
    {
       historyBoard[i].resize(game().cols());
        
        for (int j = 0; j < game().cols(); j++)
        {
            coords.push_back(Point(i,j));
            historyBoard[i][j] = '.';
        }
    }
}

bool GoodPlayer::placeShips(Board& b)
{
    int shipID = 0;
    bool allow;
    
    int s_remaining = game().nShips();
    
    while (s_remaining > 0)
    {
        int m = randInt(coords.size());
        Point tp(coords[m].r, coords[m].c);
        allow = b.placeShip(tp, shipID, HORIZONTAL);
        if (!allow)
        {
            allow = b.placeShip(tp, shipID, VERTICAL);
        }
        if (allow)
        {
            yCoord(tp, coords);
            s_remaining--;
            shipID++;
        }
    }

    coords.clear();
    for (int i = 0; i < game().rows(); i++)
    {
        for (int j = 0; j < game().cols(); j++)
        {
            Point tp(i, j);
            coords.push_back(tp);
        }
    }
    return true;
}

Point GoodPlayer::recommendAttack()
{
    if (state == 1)
    {
        int m = randInt(coords.size());
        Point tp(coords[m].r, coords[m].c);
        yCoord(tp, coords);
        return tp;
    }
    else
    {
        Point tp;
        if (!pointsToAttack.empty())
        {
            tp = pointsToAttack.top();
        }
        
        pointsToAttack.pop();
        yCoord(tp, coords);
        return tp;
    }
}

void GoodPlayer::recordAttackResult(Point p, bool validShot, bool shotHit, bool shipDestroyed, int shipId)
{
    if (shotHit)
    {
        historyBoard[p.r][p.c] = 'X';
        incCoordsToAttack(p);
    }
    else
    {
        historyBoard[p.r][p.c] = 'o';
    }
    
    if (state == 1)
    {
        if (shotHit)
        {
            state = 2;
        }
    }
    else
    {
        if (pointsToAttack.empty())
        {
            state = 1;
        }
    }
}

void GoodPlayer::incCoordsToAttack(Point p)
{
    // check all 4 points around attacked point, adn then add to stack.
    if (p.r - 1 >= 0 && historyBoard[p.r - 1][p.c] == '.')
    {
        historyBoard[p.r - 1][p.c] = 'a';
        pointsToAttack.push(Point(p.r - 1, p.c));
    }
    if (p.r + 1 <= game().rows() - 1 && historyBoard[p.r + 1][p.c] == '.')
    {
        historyBoard[p.r + 1][p.c] = 'a';
        pointsToAttack.push(Point(p.r + 1, p.c));
    }
    if (p.c - 1 >= 0 && historyBoard[p.r][p.c - 1] == '.')
    {
        historyBoard[p.r][p.c - 1] = 'a';
        pointsToAttack.push(Point(p.r, p.c - 1));
    }
    if (p.c + 1 <= game().cols() - 1 && historyBoard[p.r][p.c + 1] == '.')
    {
        historyBoard[p.r][p.c + 1] = 'a';
        pointsToAttack.push(Point(p.r, p.c + 1));
    }
}

//*********************************************************************
//  createPlayer
//*********************************************************************

Player* createPlayer(string type, string nm, const Game& g)
{
    static string types[] = {
        "human", "awful", "mediocre", "good"
    };
    
    int pos;
    for (pos = 0; pos != sizeof(types)/sizeof(types[0])  &&
         type != types[pos]; pos++)
        ;
    switch (pos)
    {
        case 0:  return new HumanPlayer(nm, g);
        case 1:  return new AwfulPlayer(nm, g);
        case 2:  return new MediocrePlayer(nm, g);
        case 3:  return new GoodPlayer(nm, g);
        default: return nullptr;
    }
}
