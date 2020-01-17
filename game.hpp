//
// game.hpp
// ~~~~~~~~
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
#include <fstream>
#include <unistd.h>
#define DECK_SIZE 312

using namespace std;
//----------------------------------------------------------------------
typedef enum {DIAMONDS,HEARTS,CLUBS,SPADES} SUITE; //The enum card suits
//----------------------------------------------------------------------
// Description: This class is used to create the cards used for the
// blackjack game.
class card{
public:
    int UID; //0-311
    int BJvalue; //1 or 11(A),2,3,4,5,6,7,8,9,10(Q,J,K)
    SUITE suite; //DIAMONDS,HEARTS,CLUBS,SPADES
    char face; //NULL or A,Q,J,K
};
//----------------------------------------------------------------------
// Description: This class is used to create the hands used for the
// blackjack game, using the cards.
class hand{
public:
    hand(){
        for(int i = 0; i < 20; i++){
            cardH[i] = 320;
        }
    }
    int cardH[20];
};
//----------------------------------------------------------------------
// Description: This class is used to create the players who play the
// blackjack game, they use hand.
class Player{
public:
    Player(){
        cout << "Creating a new player " << endl;
        balance = 100; //There current credit amount
        join_status = false; //If they have joined
        wager_status = false; //If they have wagered
        stand_status = false; //If they have stood
        first_status = false; //If this is their first card
        rejoin = false; //If they have rejoined
        started = false; //If they have started playing the game
        FirstGame = true; //If this is their first game
        win = false; //If they win
        done = false; //If they are standing and waiting for other players
        BlackJ = false; //If they have a BlackJack
        wager = 0; //There current wager
        Aces = 0; //How many Aces are in their hand
        Ppoints = 0; //How many points their cards are worth
        Pcount = 0; //Their Player count
        Pnum = 0; //Their Player Placement
    }
    virtual ~Player() {}
    virtual void deliver(const chat_message& msg) = 0;
    int wager, Pnum, Pcount, balance, Ppoints, Aces;
    bool join_status, wager_status, stand_status, first_status, FirstGame, win, done, started, BlackJ, rejoin;
    string name;
    hand H;
};
typedef shared_ptr<Player> P_ptr;
//----------------------------------------------------------------------
// Description: This class is used to create the dealer, they use hand.
class Dealer{
public:
    Dealer(){
        cout << "Creating a dealer" << endl;
        current_P = NULL; //The current Player
        first = false; //If this is the first card
        DFirst_Start = false; //If this is the first hand
        Game_Start = false; //If the hand is starting
        DBlackJ = false; //If the dealer got a BlackJack
        credits = 500; //The amount of dealer credits
        Dpoints = 0; //The amount of dealer points
        Dcount = 0; //The dealer count
        Plcount = 0; //The player count
        for(int i = 0; i < 6; i++){
            PlayerNumber[i][0] = 0;
            PlayerNumber[i][1] = i+1;
        }
        ACES = 0; //The amount of Aces the dealer has
        CardCount = 312; //The amount of cards in the deck
    }
    //Gets the next player
    void next_player(P_ptr P){
        current_P = P;
    }
    //Restocks dealer credits, waits 30 seconds
    void ReCredit(){
        usleep(3000000);
        credits = 500;
    }
    //Verifies if the shuffle was correct and accurate
    bool verifyDeck(){
        int i, j;
        int count = 0;
        int tester[2][DECK_SIZE];
        for(i = 0; i < DECK_SIZE; i++){
            tester[0][i] = i;
            tester[1][i] = 0;
        }
        for(i = 0; i < DECK_SIZE; i++){
            for(j = 0; j < DECK_SIZE; j++){
                if(_shuffledDeck[i] == tester[0][j]){
                    tester[1][j]++;
                    if(tester[1][j] > 1){
                        cout << "Deck Integrity Check Failed: Duplicate Card Found!\n";
                        return false;
                    }
                    break;
                }
            }
        }
        for(i = 0; i < DECK_SIZE; i++){
            if(tester[1][i] != 1){
                cout << "Deck Integrity Check Failed: Missing Card\n";
                return false;
            }
        }
        for(j = 0; j < DECK_SIZE; j++){
            if((_previousDeck[j] == _shuffledDeck[j]) || (_previousDeck[j-1] == _shuffledDeck[j]) || (_previousDeck[j+1] == _shuffledDeck[j])){
                count++;
            }
        }
        //TAT ADDED delete all the previous, previousDeck elements before replacement
        _previousDeck.clear();
        for(i=0; i<DECK_SIZE; i++){
            _previousDeck.push_back(_shuffledDeck[i]);
        }
        if(count < 6){
            return true;
        }
        else{
            return false;
        }
    }
    //Shuffles the current deck
    void shuffleDeck(){
        unsigned seed = 0;
        //TAT ADDED clear to dump current deck since its stored in previous if verifyDeck=T
        _shuffledDeck.clear();
        for(int i=0; i<DECK_SIZE; i++){
            _shuffledDeck.push_back(_previousDeck[i]);
        }
        shuffle(_shuffledDeck.begin(), _shuffledDeck.end(), default_random_engine(seed));
        CardCount = 312;
    }
    //Builds the card lookup to find the card values
    void buildCardLookup(){
        card C;
        int UID = 0;
        for(int d=0; d<6; d++){
            for(int h=0; h<13; h++){
                for(int s=0; s<4; s++){
                    if(h>0 && h<10){
                        C.UID = UID;
                        C.BJvalue = h + 1;
                        C.face = ' ';
                        if(s == 0){
                            C.suite = DIAMONDS;
                        }
                        else if(s == 1){
                            C.suite = HEARTS;
                        }
                        else if(s == 2){
                            C.suite = CLUBS;
                        }
                        else if(s == 3){
                            C.suite = SPADES;
                        }
                        cardLookup(C);
                    }else if(h==0){
                        C.UID = UID;
                        C.BJvalue = 11;
                        C.face = 'A';
                        if(s == 0){
                            C.suite = DIAMONDS;
                        }
                        else if(s == 1){
                            C.suite = HEARTS;
                        }
                        else if(s == 2){
                            C.suite = CLUBS;
                        }
                        else if(s == 3){
                            C.suite = SPADES;
                        }
                        cardLookup(C);
                    }else if(h==10){
                        C.UID = UID;
                        C.BJvalue = 10;
                        C.face = 'J';
                        if(s == 0){
                            C.suite = DIAMONDS;
                        }
                        else if(s == 1){
                            C.suite = HEARTS;
                        }
                        else if(s == 2){
                            C.suite = CLUBS;
                        }
                        else if(s == 3){
                            C.suite = SPADES;
                        }
                        cardLookup(C);
                    }else if(h==11){
                        C.UID = UID;
                        C.BJvalue = 10;
                        C.face = 'Q';
                        if(s == 0){
                            C.suite = DIAMONDS;
                        }
                        else if(s == 1){
                            C.suite = HEARTS;
                        }
                        else if(s == 2){
                            C.suite = CLUBS;
                        }
                        else if(s == 3){
                            C.suite = SPADES;
                        }
                        cardLookup(C);
                    }else if(h==12){
                        C.UID = UID;
                        C.BJvalue = 10;
                        C.face = 'K';
                        if(s == 0){
                            C.suite = DIAMONDS;
                        }
                        else if(s == 1){
                            C.suite = HEARTS;
                        }
                        else if(s == 2){
                            C.suite = CLUBS;
                        }
                        else if(s == 3){
                            C.suite = SPADES;
                        }
                        cardLookup(C);
                    }
                    UID++;
                }
            }
        }
    }
    //Creates the first deck
    void createDeck(){
        for(int i=0; i<DECK_SIZE; i++){
            _previousDeck.push_back(i);
        }
    }
    //Creates the test decks based on the option selected
    void createTestDeck(int Option){
        int Temp = 0;
        if(Option == 1){
            for(int i=DECK_SIZE-1; i>-1; i--){
                _previousDeck.push_back(i);
            }
            for(int i=DECK_SIZE-1; i>-1; i--){
                _shuffledDeck.push_back(i);
            }
        }
        else if(Option == 2){
            Temp = 310;
            for(int i=0; i<DECK_SIZE; i++){
                _previousDeck.push_back(i);
            }
            for(int i=0; i<DECK_SIZE; i++){
                if((i % 2) == 0){
                    _shuffledDeck.push_back(_previousDeck.at(i));
                }
                else{
                    _shuffledDeck.push_back(_previousDeck.at(i+Temp));
                    Temp = Temp - 4;
                }
            }
        }
        else if(Option == 3){
            for(int i=0; i<DECK_SIZE; i++){
                _previousDeck.push_back(i);
            }
            for(int i=0; i<DECK_SIZE; i++){
                _shuffledDeck.push_back(i);
            }
        }
        CardCount = 312;
    }
    //Deals dealer cards
    void dealCards(){
        if(first == true){
            int U;
            int size = _shuffledDeck.size()-1;
            U = _shuffledDeck[size];
            _shuffledDeck.erase(_shuffledDeck.end()-1);
            int UID = U;
            for(int i = 0; i < 20; i++){
                if(H.cardH[i] == 320){
                    H.cardH[i] = UID;
                    i = 20;
                }
            }
            CardCount = CardCount - 1;
            Dcount++;
        }
        else if(first == false){
            for(int j = 0; j < 2; j++){
                int U;
                int size = _shuffledDeck.size()-1;
                U = _shuffledDeck[size];
                _shuffledDeck.erase(_shuffledDeck.end()-1);
                int UID = U;
                for(int i = 0; i < 20; i++){
                    if(H.cardH[i] == 320){
                        H.cardH[i] = UID;
                        i = 20;
                    }
                }
            }
            CardCount = CardCount - 2;
            first = true;
            Dcount = 2;
        }
    }
    //Deals cards to the current player P
    void dealPlayer(P_ptr P){
        current_P = P;
        if(P->first_status == true){
            int U;
            int size = _shuffledDeck.size()-1;
            U = _shuffledDeck[size];
            _shuffledDeck.erase(_shuffledDeck.end()-1);
            int UID = U;
            for(int i = 0; i < 20; i++){
                if((current_P->H).cardH[i] == 320){
                    (current_P->H).cardH[i] = UID;
                    i = 20;
                }
            }
            CardCount = CardCount - 1;
            P->Pcount++;
        }
        else if(P->first_status == false){
            for(int j = 0; j < 2; j++){
                int U;
                int size = _shuffledDeck.size()-1;
                U = _shuffledDeck[size];
                _shuffledDeck.erase(_shuffledDeck.end()-1);
                int UID = U;
                for(int i = 0; i < 20; i++){
                    if((current_P->H).cardH[i] == 320){
                        (current_P->H).cardH[i] = UID;
                        i = 20;
                    }
                }
            }
            CardCount = CardCount - 2;
            P->first_status = true;
            P->Pcount = 2;
        }
    }
    //Gets a players, P, card, y, and returns its UID value
    int getCardP(int x, int y, P_ptr P){
        current_P = P;
        int UID = 320;
        for(int i = 0; i < 20; i++){
            if(i == y){
                UID = (current_P->H).cardH[i];
                i = 20;
            }
        }
        return UID;
    }
    //Takes a card, C, and puts it in the card lookup
    void cardLookup(card C){
        //cout << C.UID << ": " << C.BJvalue << ", " << C.suite << ", " << C.face << "\n";
        deck.push_back (C);
    }
    //Returns a card value using only the UID
    int cardvalue(int UID){
        int V = 0;
        deque<card>::iterator it = deck.begin();
        while (it != deck.end()){
            if(UID == (*it).UID){
                V = (*it).BJvalue;
                break;
            }
            *it++;
        }
        return V;
    }
    //Save the current deck into a file called "Audit.txt"
    void SaveFile(){
        ofstream SaveFile("Audit.txt");
        string OutPut = "Current Deck\n";
        int Temp = 320;
        string SUITE;
        for(int i = 0; i < CardCount; i++){
            Temp = _shuffledDeck.at(i);
            for(int j = 0; j < DECK_SIZE; j++){
                if(deck[j].UID == Temp){
                    if(deck[j].suite == 0){
                        SUITE = "Diamonds";
                    }
                    else if(deck[j].suite == 1){
                        SUITE = "Hearts";
                    }
                    else if(deck[j].suite == 2){
                        SUITE = "Clubs";
                    }
                    else{
                        SUITE = "Spades";
                    }
                    if(deck[j].face == ' '){
                        OutPut = OutPut + to_string(i) + ":\tUID = " + to_string(deck[j].UID) + "|\tValue = " + to_string(deck[j].BJvalue) + "|\tSuite=" + SUITE + "|\tFace = " + to_string(deck[j].BJvalue) + "\n";
                    }
                    else{
                        OutPut = OutPut + to_string(i) + ":\tUID = " + to_string(deck[j].UID) + "|\tValue = " + to_string(deck[j].BJvalue) + "|\tSuite=" + SUITE + "|\tFace = " + deck[j].face + "\n";
                    }
                }
            }
        }
        SaveFile << OutPut << endl;
        SaveFile.close();
    }
    deque<card> deck;
    P_ptr current_P;
    hand H;
    int credits, Dpoints, Dcount, Plcount, ACES, CardCount;
    int PlayerNumber[6][2];
    bool first, Game_Start, DBlackJ, DFirst_Start;
private:
    vector<int> _shuffledDeck;
    vector<int> _previousDeck;
};
