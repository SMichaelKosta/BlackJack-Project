//
// table.hpp
// ~~~~~~~~~
#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <random>
#include <algorithm>
#include <string>
#include <vector>
#include <unistd.h>
#include "chat_message.hpp"
#include "game.hpp"
#define DECK_SIZE 312

using namespace std;
typedef deque<chat_message> chat_message_queue;
//----------------------------------------------------------------------
// Description: This class is used to create the table used for the
// blackjack game.
class blackjack_table{
public:
    //Used to insert a player that joins, takes a player P.
    void join(P_ptr P){
        cout << "joining the table" << endl;
        Ps.insert(P);
    }
    //Used to erase a player that leave, take a plaery P.
    void leave(P_ptr P){
        Ps.erase(P);
    }
    //Used to deliver a message to a player, take a chat_message msg.
    void deliver(const chat_message& msg){
        recent_msgs_.push_back(msg);
        while (recent_msgs_.size() > max_recent_msgs)
            recent_msgs_.pop_front();
        for (auto P: Ps)
            P->deliver(msg);
    }
    //Used to check if all players have a name, returns true if true
    bool all_players_have_a_name(){
        bool retval = true;
        for (auto P: Ps){
            retval = retval && (P->name > "");
        }
        return retval;
    }
    //Used to check if all players have joined, returns true if true
    bool all_players_joined(){
        bool retval = true;
        for (auto P: Ps){
            if(P->join_status == false){
                retval = false;
                break;
            }
        }
        return retval;
    }
    //Used to check if all players have started, returns true if true
    bool all_players_start(){
        bool retval = true;
        for (auto P: Ps){
            if(P->started == false){
                retval = false;
                break;
            }
        }
        return retval;
    }
    //Used to check if all players have wagered, returns true if true
    bool all_players_have_wagered(){
        bool retval = true;
        for (auto P: Ps){
            if(P->wager_status == false){
                retval = false;
                break;
            }
        }
        return retval;
    }
    //Used to check if all players have stood, returns true if true
    bool all_players_stand(){
        bool retval = true;
        for (auto P: Ps){
            if(P->stand_status == false){
                retval = false;
                break;
            }
        }
        return retval;
    }
    //Used to deal cards to the players
    void deal(){
        set<P_ptr> :: iterator itr;
        for(itr = Ps.begin(); itr != Ps.end(); ++itr){
            D.dealPlayer(*itr);
        }
    }
    //Used to get the card value from a player, it takes ints x and y
    //and returns the cards UID number, int UID.
    int getCard(int x, int y){
        int countX = 0;
        int UID = 320;
        set<P_ptr> :: iterator itr;
        for(itr = Ps.begin(); itr != Ps.end(); ++itr){
            if(countX == x){
                UID = D.getCardP(x, y, *itr);
            }
            countX++;
        }
        return UID;
    }
    //Used to set a players stand_status, it takes a bool for the new
    //status and sets it to player P's stand_status.
    void Sstatus(bool S, P_ptr P){
        P->stand_status = S;
    }
    //Used to get a players stand_status, it takes a player P and
    //returns their stand status as a bool, Sta.
    bool Sget(P_ptr P){
        bool Sta;
        Sta = P->stand_status;
        return Sta;
    }
    //Used to set a players wager_status, it takes a bool for the new
    //status and sets it to player P's wager_status.
    void Wstatus(bool W, P_ptr P){
        P->wager_status = W;
    }
    //Used to get a players wager_status, it takes a player P and
    //returns their wager status as a bool, Wag.
    bool Wget(P_ptr P){
        bool Wag;
        Wag = P->wager_status;
        return Wag;
    }
    //Used to edit a Players points, takes int j, the location of
    //the player, and X, if the points are an Ace.
    void Points(int j, int X){
        int countX = 0;
        set<P_ptr> :: iterator itr;
        for(itr = Ps.begin(); itr != Ps.end(); ++itr){
            if(countX == j){
                if(X == 11){
                    (*itr)->Aces++;
                }
                (*itr)->Ppoints = (*itr)->Ppoints + X;
            }
            countX++;
        }
    }
    //Used to set a Players points, takes int j, the location of
    //the player, and X, if the points are an Ace.
    void SetPoints(int j, int X){
        int countX = 0;
        set<P_ptr> :: iterator itr;
        for(itr = Ps.begin(); itr != Ps.end(); ++itr){
            if(countX == j){
                (*itr)->Ppoints = X;
            }
            countX++;
        }
    }
    //Used to get a Players points, takes j, the location of the
    //player, and returns their points.
    int GetPoints(int j){
        int countX = 0;
        int points;
        set<P_ptr> :: iterator itr;
        for(itr = Ps.begin(); itr != Ps.end(); ++itr){
            if(countX == j){
                points = (*itr)->Ppoints;
            }
            countX++;
        }
        return points;
    }
    //Used to set Player, P's, name to string, n.
    void SetName(string n, P_ptr P){
        P->name = n;
    }
    //Uses int i to get a Player's name and return it as a string n.
    string GetName(int i){
        int countX = 0;
        string n;
        set<P_ptr> :: iterator itr;
        for(itr = Ps.begin(); itr != Ps.end(); ++itr){
            if(countX == i){
                n = (*itr)->name;
            }
            countX++;
        }
        return n;
    }
    //Used to set Player, P's, wager to int, W.
    void SetWager(int W, P_ptr P){
        P->wager = W;
        P->FirstGame = false;
    }
    //Used to get Player,P's, FirstGame status and return it
    //as FIRST.
    bool GetFirstGame(P_ptr P){
        bool FIRST = P->FirstGame;
        return FIRST;
    }
    //Uses int j to get a Player's wager and return it as an int wager.
    int GetWager(int j){
        int countX = 0;
        int wager = 0;
        set<P_ptr> :: iterator itr;
        for(itr = Ps.begin(); itr != Ps.end(); ++itr){
            if(countX == j){
                wager = (*itr)->wager;
            }
            countX++;
        }
        return wager;
    }
    //Uses j to get a Player, P's, player count and return it as COUNT.
    int GetPCount(int j, P_ptr P){
        int countX = 0;
        int COUNT = 0;
        set<P_ptr> :: iterator itr;
        for(itr = Ps.begin(); itr != Ps.end(); ++itr){
            if(countX == j){
                COUNT = (*itr)->Pcount;
            }
            countX++;
        }
        return COUNT;
    }
    //Uses j to locate a Player and return if they have an Ace.
    //returns true if they do.
    bool ACE(int j){
        int countX = 0;
        int num = 0;
        bool ace = false;
        set<P_ptr> :: iterator itr;
        for(itr = Ps.begin(); itr != Ps.end(); ++itr){
            if(countX == j){
                num = (*itr)->Aces;
                if(num != 0){
                    ace = true;
                }
            }
            countX++;
        }
        return ace;
    }
    //Uses j to get the amount of Aces a Player has and return it
    //as num.
    int SetACE(int j){
        int countX = 0;
        int num = 0;
        set<P_ptr> :: iterator itr;
        for(itr = Ps.begin(); itr != Ps.end(); ++itr){
            if(countX == j){
                num = (*itr)->Aces;
            }
            countX++;
        }
        return num;
    }
    //Uses j to find the Player's who's points went over 21. Then it
    //sets that player to a stand possition.
    void Break(int j){
        int countX = 0;
        set<P_ptr> :: iterator itr;
        for(itr = Ps.begin(); itr != Ps.end(); ++itr){
            if(countX == j){
                (*itr)->done = true;
                (*itr)->stand_status = true;
                (*itr)->win = false;
            }
            countX++;
        }
    }
    //Uses j to find the Player's who lost. Adjust their balance.
    //Then reset them to a new game status.
    void Lost(int j){
        int countX = 0;
        set<P_ptr> :: iterator itr;
        for(itr = Ps.begin(); itr != Ps.end(); ++itr){
            if(countX == j){
                (*itr)->done = false;
                (*itr)->stand_status = false;
                (*itr)->join_status = false;
                (*itr)->win = false;
                (*itr)->balance = (*itr)->balance - (*itr)->wager;
                D.credits = D.credits + (*itr)->wager;
                (*itr)->wager = 0;
                (*itr)->wager_status = false;
                (*itr)->first_status = false;
                (*itr)->started = true;
                (*itr)->FirstGame = false;
                (*itr)->Aces = 0;
                (*itr)->Ppoints = 0;
                (*itr)->BlackJ = false;
                for(int i = 0; i < 20; i++){
                    ((*itr)->H).cardH[i] = 320;
                }
                (*itr)->rejoin = true;
            }
            countX++;
        }
    }
    //Uses j to find the Player's who won. Adjust their balance.
    //Then reset them to a new game status.
    void Win(int j){
        int countX = 0;
        set<P_ptr> :: iterator itr;
        for(itr = Ps.begin(); itr != Ps.end(); ++itr){
            if(countX == j){
                (*itr)->done = false;
                (*itr)->stand_status = false;
                (*itr)->join_status = false;
                (*itr)->win = false;
                (*itr)->balance = (*itr)->balance + (*itr)->wager;
                D.credits = D.credits - (*itr)->wager;
                (*itr)->wager = 0;
                (*itr)->wager_status = false;
                (*itr)->first_status = false;
                (*itr)->started = true;
                (*itr)->FirstGame = false;
                (*itr)->Aces = 0;
                (*itr)->Ppoints = 0;
                (*itr)->BlackJ = false;
                for(int i = 0; i < 20; i++){
                    ((*itr)->H).cardH[i] = 320;
                }
                (*itr)->rejoin = true;
            }
            countX++;
        }
    }
    //Uses j to find the Player's who go a blackjack.
    //Adjust their balance. Then reset them to a new game status.
    void BJWin(int j){
        int countX = 0;
        set<P_ptr> :: iterator itr;
        for(itr = Ps.begin(); itr != Ps.end(); ++itr){
            if(countX == j){
                (*itr)->done = false;
                (*itr)->stand_status = false;
                (*itr)->join_status = false;
                (*itr)->win = false;
                (*itr)->balance = (*itr)->balance + (2*((*itr)->wager));
                D.credits = D.credits - (2*((*itr)->wager));
                (*itr)->wager = 0;
                (*itr)->wager_status = false;
                (*itr)->first_status = false;
                (*itr)->started = true;
                (*itr)->FirstGame = false;
                (*itr)->Aces = 0;
                (*itr)->Ppoints = 0;
                (*itr)->BlackJ = false;
                for(int i = 0; i < 20; i++){
                    ((*itr)->H).cardH[i] = 320;
                }
                (*itr)->rejoin = true;
            }
            countX++;
        }
    }
    //Uses j to find the Player's who tied the dealer.
    //Adjust their balance. Then reset them to a new game status.
    void Tied(int j){
        int countX = 0;
        set<P_ptr> :: iterator itr;
        for(itr = Ps.begin(); itr != Ps.end(); ++itr){
            if(countX == j){
                (*itr)->done = false;
                (*itr)->stand_status = false;
                (*itr)->join_status = false;
                (*itr)->win = false;
                (*itr)->wager = 0;
                (*itr)->wager_status = false;
                (*itr)->first_status = false;
                (*itr)->started = true;
                (*itr)->FirstGame = false;
                (*itr)->Aces = 0;
                (*itr)->Ppoints = 0;
                (*itr)->BlackJ = false;
                for(int i = 0; i < 20; i++){
                    ((*itr)->H).cardH[i] = 320;
                }
                (*itr)->rejoin = true;
            }
            countX++;
        }
    }
    //Uses a Player P and returns their Done status.
    bool DoneGet(P_ptr P){
        bool Done;
        Done = P->done;
        return Done;
    }
    //Uses a Player P and a bool start to change a players started
    //status.
    void SetStart(P_ptr P, bool start){
        P->started = start;
    }
    //Uses a Player P and returns their started status.
    bool GetStart(P_ptr P){
        bool Start;
        Start = P->started;
        return Start;
    }
    //Uses int j to set the current players player number.
    void SetPlayerNumbers(int j){
        int countX = 1;
        set<P_ptr> :: iterator itr;
        for(itr = Ps.begin(); itr != Ps.end(); ++itr){
            if(countX == j){
                (*itr)->Pnum = j;
            }
            countX++;
        }
    }
    //Uses a Player P and returns their player number.
    int GetPlayerNum(P_ptr P){
        int NUM;
        NUM = P->Pnum;
        return NUM;
    }
    //Uses a Player P and int j and returns their balance.
    int GetBalance(int j, P_ptr P){
        int countX = 0;
        int BAL = 0;
        set<P_ptr> :: iterator itr;
        for(itr = Ps.begin(); itr != Ps.end(); ++itr){
            if(countX == j){
                BAL = (*itr)->balance;
            }
            countX++;
        }
        return BAL;
    }
    //Uses a Player P and returns their balance.
    int ReturnBalance(P_ptr P){
        int Bal;
        Bal = P->balance;
        return Bal;
    }
    //Uses int j to set the current players BlackJ status to true.
    void SetBlackJack(int j){
        int countX = 0;
        set<P_ptr> :: iterator itr;
        for(itr = Ps.begin(); itr != Ps.end(); ++itr){
            if(countX == j){
                (*itr)->BlackJ = true;
            }
            countX++;
        }
    }
    //Uses int j to get the current players BlackJ status.
    bool GetBlackJack(int j){
        int countX = 0;
        bool BJT;
        set<P_ptr> :: iterator itr;
        for(itr = Ps.begin(); itr != Ps.end(); ++itr){
            if(countX == j){
                BJT = (*itr)->BlackJ;
            }
            countX++;
        }
        return BJT;
    }
    //Uses Player P to get the current players rejoin status.
    bool ReJoStatus(P_ptr P){
        bool ReJo;
        ReJo = P->rejoin;
        return ReJo;
    }
    //Uses a Player P and a bool S to change a players rejoin
    //status.
    void ReJoUpdate(P_ptr P, bool S){
        P->rejoin = S;
    }
    //Uses Player P to get the current players join status.
    bool GetJoinStatus(P_ptr P){
        bool Jo;
        Jo = P->join_status;
        return Jo;
    }
    //Uses a Player P and a bool S to change a players join
    //status.
    void UpdateJoin(P_ptr P, bool S){
        P->join_status = S;
    }
    Dealer D;
private:
    set<P_ptr> Ps;
    enum { max_recent_msgs = 100 };
    chat_message_queue recent_msgs_;
};
