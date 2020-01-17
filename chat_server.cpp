//
// chat_server.cpp
// ~~~~~~~~~~~~~~~
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
#include <thread>
#include "asio.hpp"
#include "table.hpp"
#define DECK_SIZE 312

using namespace std;
using asio::ip::tcp;
//--------------------------------------------------------------------------------------------------
// Description: This class is used to create and update the blackjack_player.
class blackjack_player : public Player, public enable_shared_from_this<blackjack_player>{
public:
    //Builds the player.
    blackjack_player(tcp::socket socket, blackjack_table& table) : S(move(socket)), T(table){
        cout << "Creating a blackjack_player " << endl;
    }
    //Joins the player to the table, starting their game.
    void start(){
        T.join(shared_from_this());
        do_read_header();
    }
    //Delivers messages to the player.
    void deliver(const chat_message& msg){
        bool write_in_progress = !write_msgs_.empty();
        write_msgs_.push_back(msg);
        if (!write_in_progress){
            do_write();
        }
    }
private:
    //Reads the message header and sends it to the body.
    void do_read_header(){
        auto self(shared_from_this());
        asio::async_read(S, asio::buffer(read_msg_.data(), chat_message::header_length),
                         [this, self](error_code ec, size_t){
            if (!ec && read_msg_.decode_header()){
                do_read_body();
            }
            else{
                T.leave(shared_from_this());
            }
        });
    }
    //Reads the message body. This is where the game is run, where the messages are built,
    //and where the player/dealer data is changed.
    void do_read_body(){
        auto self(shared_from_this());
        asio::async_read(S, asio::buffer(read_msg_.body(), read_msg_.body_length()),
                         [this, self](error_code ec, size_t){
            if (!ec && (T.D.credits >= 20) && (T.D.CardCount >= 20) && (T.ReturnBalance(self) > 0)){
                string m;               //The initialization of the output text string.
                int i, j;               //The initialization of the for loop ints.
                bool avoid = false;     //Used to avoid the first deal of each hand,
                                        //to prevent people from getting 2 cards.
                                        //Updates in the hit function.
                bool leaver = false;    //If the player successfully quits,
                                        //the leaver status is change to true.
                                        //It will run normally until the end.
                                        //It will only print to the current player,
                                        //then kick them out.
                bool joiner = false;    //If the player successfully joins,
                                        //the joiner status is change to true.
                                        //It will run normally until the end,
                                        //It will only print to the current player,
                bool rejoiner = false;  //If tht player successfully rejoins,
                                        //the rejoiner status is change to true.
                bool stander = false;   //If the player is currently standing,
                                        //the stander status is change to true.
                                        //It will keep the player from changing anything,
                                        //until everyone is standing.
                bool tryer = false;     //If the player presses a button they are not supposed to,
                                        //the tryer status is change to true.
                                        //It will keep the player from doing thing out of order.
                bool Stand_ALL = false; //If all players are standing,
                                        //Stand_ALL is changed to true.
                                        //It runs the end game section.
                read_msg_.body_length(0);
                read_msg_.gs.dealer_cards_valid = false;    //If the dealer has cards.
                read_msg_.gs.player_cards_valid = false;    //If the player has cards.
                read_msg_.gs.current_number = 7;            //The current player number, default 7.
                read_msg_.gs.player_quit = false;           //True if the player is leaving.
                //Prevents players, who are standing, from doing anything until every stands.
                if(((T.Sget(self) == true) && (T.all_players_stand() == false)) ||
                   ((T.DoneGet(self) == true) && (T.all_players_stand() == false))){
                    m = self->name + " please wait for all players to stand.";
                    strcpy(read_msg_.body(),m.c_str());
                    read_msg_.body_length(strlen(read_msg_.body()));
                    stander = true; //Stander Update, player is a stander.
                }
                //Runs for players who finished a game and press a button.
                else if((T.D.Game_Start == false) && (T.GetJoinStatus(self) == false) &&
                        (T.ReJoStatus(self) == true)){
                    //Runs for players who pressed the join button, they rejoin the table.
                    if((read_msg_.ca.join == true) && (read_msg_.ca.name_valid == true)){
                        m = self->name + " has rejoined.";
                        strcpy(read_msg_.body(),m.c_str());
                        read_msg_.body_length(strlen(read_msg_.body()));
                        T.UpdateJoin(self, true);   //Updates players join status to true.
                        T.ReJoUpdate(self, false);  //Updates players rejoin status to false.
                        T.SetStart(self, true);     //Updates players start status to true.
                        joiner = true;              //Joiner Update, player is a joiner.
                        rejoiner = true;            //Rejoiner Update, player is a rejoiner.
                    }
                    //Runs for players who pressed the quit button, they leave the table.
                    else if(read_msg_.ca.quit == true){
                        m = self->name + " has quit. The game will close in 10 seconds.";
                        T.D.Plcount--;  //Reduces the player count by 1.
                        strcpy(read_msg_.body(),m.c_str());
                        read_msg_.body_length(strlen(read_msg_.body()));
                        leaver = true;  //Leaver Update, player is a leaver.
                    }
                    //Runs for players who pressed the hit button.
                    else if(read_msg_.ca.hit == true){
                        m = "Please rejoin before you hit.";
                        strcpy(read_msg_.body(),m.c_str());
                        read_msg_.body_length(strlen(read_msg_.body()));
                        tryer = true;   //Tryer Update, player is a tryer.
                    }
                    //Runs for players who pressed the stand button.
                    else if(read_msg_.ca.stand == true){
                        m = "Please rejoin before you stand.";
                        strcpy(read_msg_.body(),m.c_str());
                        read_msg_.body_length(strlen(read_msg_.body()));
                        tryer = true;   //Tryer Update, player is a tryer.
                    }
                    //Runs for players who pressed the wager button.
                    else if(read_msg_.ca.wager_valid == true){
                        m = "Please rejoin before you wager.";
                        strcpy(read_msg_.body(),m.c_str());
                        read_msg_.body_length(strlen(read_msg_.body()));
                        tryer = true;   //Tryer Update, player is a tryer.
                    }
                }
                //Runs for new players press a button and if the game has not started yet.
                else if((T.D.Game_Start == false) && (T.GetJoinStatus(self) == false)){
                    //Runs for players who pressed the join button.
                    if((read_msg_.ca.join == true) && (read_msg_.ca.name_valid == true)){
                        //Runs if the player count is at the max, the player is kicked.
                        if(T.D.Plcount >= 6){
                            m = "You can't join, the game is at max capacity.\nYou can either join a different server, or rejoin after someone leaves the game.\nThe game will close in 10 seconds.";
                            strcpy(read_msg_.body(),m.c_str());
                            read_msg_.body_length(strlen(read_msg_.body()));
                            leaver = true;  //Leaver Update, player is a leaver.
                        }
                        //Runs if the player count is not at mac, the player joins.
                        else{
                            m = string(read_msg_.ca.name) + " has joined.";
                            if(T.GetFirstGame(self) == true){
                                m = m + " Standard Blackjack Rules:\nYou are trying to get as close to 21 points without going over 21.\nAll cards are worth their face value. Jacks, Queens, and Kings are worth 10 points.\nAces are either 11 or 1, which ever is better.You Wager credits between 1 and 5, 1 for the first round.\nYou can Hit, ask for a card, or Stand, when you don't want another card.";
                            }
                            string n = string(read_msg_.ca.name);
                            strcpy(read_msg_.body(),m.c_str());
                            read_msg_.body_length(strlen(read_msg_.body()));
                            self->name = string (read_msg_.ca.name);
                            T.D.Plcount++;              //Raises player count by 1.
                            T.SetName(n, self);         //Set the players name.
                            T.UpdateJoin(self, true);   //Updates players join status to true.
                            T.SetStart(self, true);     //Updates players start status to true.
                            joiner = true;              //Joiner Update, player is a joiner.
                        }
                    }
                    //Runs for players who pressed the join button, but with an invalid name.
                    else if((read_msg_.ca.join == true) && (read_msg_.ca.name_valid == false)){
                        m = "Name is not valid.";
                        strcpy(read_msg_.body(),m.c_str());
                        read_msg_.body_length(strlen(read_msg_.body()));
                        tryer = true;   //Tryer Update, player is a tryer.
                    }
                    //Runs for player who pressed the quit button, the leave the table.
                    else if(read_msg_.ca.quit == true){
                        m = "You have quit. The game will close in 10 seconds.";
                        strcpy(read_msg_.body(),m.c_str());
                        read_msg_.body_length(strlen(read_msg_.body()));
                        leaver = true;  //Leaver Update, player is a leaver.
                    }
                    //Runs for players who pressed the hit button.
                    else if(read_msg_.ca.hit == true){
                        m = "Please join before you hit.";
                        strcpy(read_msg_.body(),m.c_str());
                        read_msg_.body_length(strlen(read_msg_.body()));
                        tryer = true;   //Tryer Update, player is a tryer.
                    }
                    //Runs for players who pressed the stand button.
                    else if(read_msg_.ca.stand == true){
                        m = "Please join before you stand.";
                        strcpy(read_msg_.body(),m.c_str());
                        read_msg_.body_length(strlen(read_msg_.body()));
                        tryer = true;   //Tryer Update, player is a tryer.
                    }
                    //Runs for players who pressed the wager button.
                    else if(read_msg_.ca.wager_valid == true){
                        m = "Please join before you wager.";
                        strcpy(read_msg_.body(),m.c_str());
                        read_msg_.body_length(strlen(read_msg_.body()));
                        tryer = true;   //Tryer Update, player is a tryer.
                    }
                }
                //Runs for players who try to join during a hand.
                else if((T.D.Game_Start == true) && (T.GetJoinStatus(self) == false)){
                    m = "You can't join while a game is in progress.\nYou can either join a different server, or rejoin after someone leaves the game. The game will close in 10 seconds.";
                    strcpy(read_msg_.body(),m.c_str());
                    read_msg_.body_length(strlen(read_msg_.body()));
                    leaver = true; //Leaver Update, player is a leaver.
                }
                //Runs for players who have joined and pressed a button.
                else if((T.all_players_have_a_name() == true) && (T.all_players_joined() == true)
                        && (T.D.Game_Start == false) && (T.GetJoinStatus(self) == true) &&
                        (T.Wget(self) == false)){
                    //Runs for players who pressed the wager button.
                    if(read_msg_.ca.wager_valid == true){
                        char AW = read_msg_.ca.wager;
                        int WA = (int)AW - 48;
                        //First wager, success.
                        if(T.GetFirstGame(self) == true && WA == 1){
                            m = self->name + " has wagered 1 credit.";
                            T.SetWager(WA, self);
                            T.Wstatus(true, shared_from_this());
                        }
                        //First wager, failed.
                        else if(T.GetFirstGame(self) == true && WA != 1){
                            m = self->name + " can only wager 1 credit on their first round.";
                            strcpy(read_msg_.body(),m.c_str());
                            read_msg_.body_length(strlen(read_msg_.body()));
                            T.Wstatus(false, shared_from_this());
                            tryer = true;   //Tryer Update, player is a tryer.
                        }
                        //Any other wager, success.
                        else if((T.GetFirstGame(self) == false) && (WA == 1 || WA == 2 || WA == 3
                                                                    || WA == 4 || WA == 5)){
                            if(T.ReturnBalance(self) < WA){
                                m = self->name + " does not have enough credits to wager that much.";
                                strcpy(read_msg_.body(),m.c_str());
                                read_msg_.body_length(strlen(read_msg_.body()));
                                T.Wstatus(false, shared_from_this());
                                tryer = true;   //Tryer Update, player is a tryer.
                            }
                            else{
                                m = self->name + " has wager " + to_string(WA) + " credits.";
                                T.SetWager(WA, self);
                                T.Wstatus(true, shared_from_this());
                            }
                        }
                        //Any other wager, failed.
                        else if((T.GetFirstGame(self) == false) && (WA != 1 || WA != 2 || WA != 3
                                                                    || WA != 4 || WA != 5)){
                            m = self->name + " can only wager 1 to 5 credit.";
                            strcpy(read_msg_.body(),m.c_str());
                            read_msg_.body_length(strlen(read_msg_.body()));
                            T.Wstatus(false, shared_from_this());
                            tryer = true;   //Tryer Update, player is a tryer.
                        }
                    }
                    //Runs for players who pressed the hit button.
                    else if(read_msg_.ca.hit == true){
                        m = self->name + " please wager before hitting.";
                        strcpy(read_msg_.body(),m.c_str());
                        read_msg_.body_length(strlen(read_msg_.body()));
                        tryer = true; //Tryer Update, player is a tryer.
                    }
                    //Runs for players who pressed the stand button.
                    else if(read_msg_.ca.stand == true){
                        m = self->name + " please wager before standing.";
                        strcpy(read_msg_.body(),m.c_str());
                        read_msg_.body_length(strlen(read_msg_.body()));
                        tryer = true; //Tryer Update, player is a tryer.
                    }
                    //Runs for players who pressed the quit button.
                    else if(read_msg_.ca.quit == true){
                        m = self->name + " can't quit while a game is in progress.";
                        strcpy(read_msg_.body(),m.c_str());
                        read_msg_.body_length(strlen(read_msg_.body()));
                        tryer = true; //Tryer Update, player is a tryer.
                    }
                    //Runs for players who pressed the join button.
                    else if(read_msg_.ca.join == true){
                        m = self->name + " has already joined.";
                        strcpy(read_msg_.body(),m.c_str());
                        read_msg_.body_length(strlen(read_msg_.body()));
                        tryer = true; //Tryer Update, player is a tryer.
                    }
                }
                //Runs for players who have joined, wagered, and pressed a button.
                else if((T.D.Game_Start == true) && (T.GetJoinStatus(self) == true) &&
                        (T.Wget(self) == true) && (T.all_players_have_a_name() == true) &&
                        (T.all_players_joined() == true) && (T.all_players_have_wagered() == true)){
                    avoid = true;   //Avoid Update, player is avoiding.
                    //Runs for players who pressed the hit button, updates the card and the points.
                    if(read_msg_.ca.hit == true){
                        m = self->name + " has asked for a hit.";
                        strcpy(read_msg_.body(),m.c_str());
                        read_msg_.body_length(strlen(read_msg_.body()));
                        int PN = T.GetPlayerNum(self) - 1;
                        if(T.GetPoints(PN) == 21){
                            m = m + "\nYou shouldn't hit, you have 21 points.";
                            strcpy(read_msg_.body(),m.c_str());
                            read_msg_.body_length(strlen(read_msg_.body()));
                        }
                        else{
                            T.D.dealPlayer(self);
                            int UID[6][20];
                            for(i = 0; i < 20; i++){
                                UID[PN][i] = T.getCard(PN, i);
                            }
                            int X = 0;
                            for(i = (T.GetPCount(PN, self)-1); i < 20; i++){
                                if(UID[PN][i] != 320){
                                    T.Points(PN, T.D.cardvalue(UID[PN][i]));
                                    X = T.GetPoints(PN);
                                }
                            }
                            read_msg_.gs.player_points[PN] = X;
                            X = 0;
                        }
                    }
                    //Runs for players who pressed the stand button, updates stand status to true.
                    else if(read_msg_.ca.stand == true){
                        T.Sstatus(true, self);
                        if(T.all_players_stand() == true){
                            Stand_ALL = true;   //Stand_ALL Update, all player are standing.
                        }
                        else{
                            m = self->name + " please wait for all players to stand.";
                            strcpy(read_msg_.body(),m.c_str());
                            read_msg_.body_length(strlen(read_msg_.body()));
                            stander = true;     //Stander Update, player is a stander.
                        }
                    }
                    //Runs for players who pressed the quit button.
                    else if(read_msg_.ca.quit == true){
                        m = self->name + " can't quit while a game is in progress.";
                        strcpy(read_msg_.body(),m.c_str());
                        read_msg_.body_length(strlen(read_msg_.body()));
                        tryer = true;   //Tryer Update, player is a tryer.
                    }
                    //Runs for players who pressed the join button.
                    else if(read_msg_.ca.join == true){
                        m = self->name + " has already joined.";
                        strcpy(read_msg_.body(),m.c_str());
                        read_msg_.body_length(strlen(read_msg_.body()));
                        tryer = true;   //Tryer Update, player is a tryer.
                    }
                    //Runs for players who pressed the wager button.
                    else if(read_msg_.ca.wager_valid == true){
                        m = self->name + " has already wagered.";
                        strcpy(read_msg_.body(),m.c_str());
                        read_msg_.body_length(strlen(read_msg_.body()));
                        tryer = true;   //Tryer Update, player is a tryer.
                    }
                }
                //Runs when everyone has joined and wager, the game starts.
                if((T.all_players_have_a_name() == true) && (T.all_players_joined() == true) &&
                   (T.all_players_have_wagered() == true) && (T.GetJoinStatus(self) == true) &&
                   (T.D.Game_Start == false) && (stander == false)){
                    for(i = 1; i < 7; i++){
                        T.SetPlayerNumbers(i);
                    }
                    m = "\nThe Game has started\n" + m;
                    strcpy(read_msg_.body(),m.c_str());
                    read_msg_.body_length(strlen(read_msg_.body()));
                    T.D.Game_Start = true;
                }
                //Runs when the game starts, deals cards to players and updates their points.
                if((T.all_players_have_a_name() == true) && (T.all_players_joined() == true) &&
                   (T.all_players_have_wagered() == true) && (T.GetJoinStatus(self) == true) &&
                   (T.D.Game_Start == true) && (stander == false)){
                    int X = 0;
                    int WAT = 0;
                    for(i = 0; i < 6; i++){
                        string NA = T.GetName(i);
                        for(j = 0; j < 25; j++){
                            read_msg_.gs.player_name[i][j] = NA[j];
                        }
                    }
                    for(i = 0; i < 6; i++){
                        WAT = T.GetWager(i);
                        read_msg_.gs.player_wager[i] = WAT;
                        WAT = 0;
                    }
                    if((T.GetStart(self) == true) && (T.ReJoStatus(self) == false) &&
                       (T.all_players_start() == true) && (T.D.DFirst_Start == false)){
                        T.D.dealCards();
                        T.deal();
                        T.D.DFirst_Start = true;
                        T.SetStart(self, false);
                    }
                    read_msg_.gs.dealer_cards_valid = true;
                    read_msg_.gs.player_cards_valid = true;
                    int UID[6][20];
                    for(i = 0; i < 6; i++){
                        for(j = 0; j < 20; j++){
                            UID[i][j] = T.getCard(i, j);
                        }
                    }
                    for(i = 0; i < 20; i++){
                        if(T.D.H.cardH[i] != 320){
                            read_msg_.gs.dealer_cards[i] = T.D.H.cardH[i];
                        }
                        else{
                            read_msg_.gs.dealer_cards[i] = 320;
                        }
                        for(j = 0; j < 6; j++){
                            if(UID[j][i] != 320){
                                read_msg_.gs.player_cards[j][i] = UID[j][i];
                            }
                            else{
                                read_msg_.gs.player_cards[j][i] = 320;
                            }
                        }
                    }
                    if(avoid == false){
                        for(i = 0; i < 20; i++)
                        {
                            if(read_msg_.gs.dealer_points == 0 &&
                               read_msg_.gs.dealer_cards[i] != 320){
                                T.D.Dpoints = T.D.cardvalue(T.D.H.cardH[i]);
                                if(T.D.cardvalue(T.D.H.cardH[i]) == 11){
                                    T.D.ACES++;
                                }
                            }
                            else if(read_msg_.gs.dealer_cards[i] != 320){
                                T.D.Dpoints = T.D.Dpoints + T.D.cardvalue(T.D.H.cardH[i]);
                                if(T.D.cardvalue(T.D.H.cardH[i]) == 11){
                                    T.D.ACES++;
                                }
                            }
                            if(T.D.Dpoints == 21){
                                T.D.DBlackJ = true;
                            }
                            else if(T.D.Dpoints == 22){
                                T.D.Dpoints = T.D.Dpoints - 10;
                            }
                        }
                        for(j = 0; j < 6; j++){
                            for(i = 0; i < 20; i++){
                                if(read_msg_.gs.player_points[j] == 0 &&
                                   read_msg_.gs.player_cards[j][i] != 320){
                                    T.Points(j, T.D.cardvalue(UID[j][i]));
                                }
                                else if(read_msg_.gs.player_cards[j][i] != 320){
                                    T.Points(j, T.D.cardvalue(UID[j][i]));
                                    X = T.GetPoints(j);
                                }
                                if(X == 21){
                                    T.SetBlackJack(j);
                                }
                                else if(X == 22){
                                    X = X - 10;
                                }
                            }
                            read_msg_.gs.player_points[j] = X;
                            X = 0;
                        }
                        read_msg_.gs.dealer_points = T.D.Dpoints;
                    }
                    else{
                        for(j = 0; j < 6; j++){
                            for(i = 0; i < 20; i++){
                                if(read_msg_.gs.player_points[j] == 0 &&
                                   read_msg_.gs.player_cards[j][i] != 320){
                                }
                                else if(read_msg_.gs.player_cards[j][i] != 320){
                                    X = T.GetPoints(j);
                                }
                            }
                            if(X <= 21){
                                read_msg_.gs.player_points[j] = X;
                                X = 0;
                            }
                            else if(X > 21 && T.ACE(j)){
                                int A = T.SetACE(j);
                                for(i = 0; i < A; i++){
                                    X = X - 10;
                                    A--;
                                }
                                if(X <= 21){
                                    read_msg_.gs.player_points[j] = X;
                                    T.SetPoints(j, X);
                                    X = 0;
                                }
                                else{
                                    read_msg_.gs.player_points[j] = X;
                                    T.SetPoints(j, X);
                                    T.Break(j);
                                    if(T.all_players_stand() == true){
                                        Stand_ALL = true;
                                    }
                                }
                            }
                            else{
                                read_msg_.gs.player_points[j] = X;
                                X = 0;
                                T.Break(j);
                                if(T.all_players_stand() == true){
                                    Stand_ALL = true;
                                }
                            }
                        }
                        read_msg_.gs.dealer_points = T.D.Dpoints;
                    }
                    for(i = 0; i < 6; i++){
                        read_msg_.gs.player_balance[i] = T.GetBalance(i, self);
                    }
                    read_msg_.gs.dealer_balance = T.D.credits;
                }
                else if((T.all_players_have_a_name() == true) && (T.all_players_joined() == true)
                        && (T.all_players_have_wagered() == false)
                        && (T.GetJoinStatus(self) == true) && (T.D.Game_Start == false)
                        && (stander == false)){
                    m = m + "\nPlease wager and wait for everyone to wager.";
                    strcpy(read_msg_.body(),m.c_str());
                    read_msg_.body_length(strlen(read_msg_.body()));
                }
                else if((T.all_players_have_a_name() == false)
                        && (T.all_players_have_wagered() == false)
                        && (T.GetJoinStatus(self) == true) && T.D.Game_Start == false){
                    m = m + "\nPlease wait until everyone has joined.";
                    strcpy(read_msg_.body(),m.c_str());
                    read_msg_.body_length(strlen(read_msg_.body()));
                    tryer = true;
                }
                if(Stand_ALL == true){
                    while(T.D.Dpoints < 17){
                        T.D.dealCards();
                        for(i = 0; i < 20; i++){
                            if(T.D.H.cardH[i] != 320){
                                read_msg_.gs.dealer_cards[i] = T.D.H.cardH[i];
                            }
                            else{
                                read_msg_.gs.dealer_cards[i] = 320;
                            }
                        }
                        for(i = (T.D.Dcount-1); i < 20; i++){
                            if(T.D.H.cardH[i] != 320){
                                T.D.Dpoints = T.D.Dpoints + T.D.cardvalue(T.D.H.cardH[i]);
                                if(T.D.cardvalue(T.D.H.cardH[i]) == 11){
                                    T.D.ACES++;
                                }
                            }
                        }
                        if(T.D.Dpoints > 21 && T.D.ACES > 0){
                            int A = T.D.ACES;
                            for(i = 0; i < A; i++){
                                T.D.Dpoints = T.D.Dpoints - 10;
                                A--;
                            }
                        }
                    }
                    read_msg_.gs.dealer_points = T.D.Dpoints;
                    m = "End Game";
                    if(T.D.Dpoints > 21){
                        for(i = 0; i < T.D.Plcount; i++){
                            if(T.GetPoints(i) == 0){
                            }
                            else if(T.GetPoints(i) > 21){
                                m = m + "\n" + read_msg_.gs.player_name[i] + " lost";
                                T.Lost(i);
                            }
                            else if(T.GetBlackJack(i) == true){
                                m = m + "\n" + read_msg_.gs.player_name[i] + " got a BlackJack";
                                T.BJWin(i);
                            }
                            else if(T.GetPoints(i) <= 21){
                                m = m + "\n" + read_msg_.gs.player_name[i] + " won";
                                T.Win(i);
                            }
                        }
                    }
                    else{
                        for(i = 0; i < T.D.Plcount; i++){
                            if(T.GetPoints(i) == 0){
                            }
                            else if((T.GetPoints(i) > 21) || (T.GetPoints(i) < T.D.Dpoints)){
                                m = m + "\n" + read_msg_.gs.player_name[i] + " lost";
                                T.Lost(i);
                            }
                            else if(T.GetPoints(i) == T.D.Dpoints){
                                m = m + "\n" + read_msg_.gs.player_name[i] + " tied the dealer";
                                T.Tied(i);
                            }
                            else if((T.GetBlackJack(i) == true) && (T.GetPoints(i) > T.D.Dpoints)){
                                m = m + "\n" + read_msg_.gs.player_name[i] + " got a BlackJack";
                                T.BJWin(i);
                            }
                            else if((T.GetPoints(i) <= 21) && (T.GetPoints(i) > T.D.Dpoints)){
                                m = m + "\n" + read_msg_.gs.player_name[i] + " won";
                                T.Win(i);
                            }
                        }
                    }
                    T.D.Dpoints = 0;
                    T.D.Dcount = 0;
                    T.D.ACES = 0;
                    for(i = 0; i < 20; i++){
                        T.D.H.cardH[i] = 320;
                    }
                    int WAT = 0;
                    for(i = 0; i < 6; i++){
                        WAT = T.GetWager(i);
                        read_msg_.gs.player_wager[i] = WAT;
                        WAT = 0;
                    }
                    for(i = 0; i < 6; i++){
                        read_msg_.gs.player_balance[i] = T.GetBalance(i, self);
                    }
                    read_msg_.gs.dealer_balance = T.D.credits;
                    T.D.first = false;
                    T.D.Game_Start = false;
                    T.D.DBlackJ = false;
                    T.D.DFirst_Start = false;
                    m = m + "\nIf you would like to play the next hand, hit join. If you would like to leave, hit quit.";
                    strcpy(read_msg_.body(),m.c_str());
                    read_msg_.body_length(strlen(read_msg_.body()));
                    Stand_ALL = false;
                    for(i = 0; i < 6; i++){
                        T.D.PlayerNumber[i][0] = 0;
                    }
                    read_msg_.gs.current_number = 8;
                }
                //Leaver message deliver.
                if(leaver == true){
                    for(i = 0; i < 6; i++){
                        if(T.D.PlayerNumber[i][1] == T.GetPlayerNum(self)){
                            T.D.PlayerNumber[i][0] = 0;
                            i = 6;
                        }
                    }
                    read_msg_.gs.player_quit = true;
                    read_msg_.encode_header();
                    self->deliver(read_msg_);
                    do_read_header();
                    T.leave(self);
                }
                //Joiner message deliver.
                else if(joiner == true){
                    if(rejoiner == true && T.D.Plcount > 1){
                        for(i = 0; i < 6; i++){
                            if(T.D.PlayerNumber[i][1] == T.GetPlayerNum(self)
                               && T.D.PlayerNumber[i][0] != 1){
                                read_msg_.gs.current_number = T.D.PlayerNumber[i][1];
                                T.D.PlayerNumber[i][0] = 1;
                                i = 6;
                            }
                        }
                    }
                    else if(T.D.Plcount > 1){
                        for(i = T.D.Plcount-1; i < 6; i++){
                            if(T.D.PlayerNumber[i][0] == 0){
                                read_msg_.gs.current_number = T.D.PlayerNumber[i][1];
                                T.D.PlayerNumber[i][0] = 1;
                                i = 6;
                            }
                        }
                    }
                    else{
                        for(i = 0; i < 6; i++){
                            if(T.D.PlayerNumber[i][0] == 0){
                                read_msg_.gs.current_number = T.D.PlayerNumber[i][1];
                                T.D.PlayerNumber[i][0] = 1;
                                i = 6;
                            }
                        }
                    }
                    read_msg_.encode_header();
                    self->deliver(read_msg_);
                    do_read_header();
                }
                //Tryer message deliver.
                else if(tryer == true){
                    read_msg_.encode_header();
                    self->deliver(read_msg_);
                    do_read_header();
                }
                //Stander message deliver.
                else if(stander == true){
                    read_msg_.encode_header();
                    self->deliver(read_msg_);
                    do_read_header();
                }
                //Regular message deliver.
                else{
                    read_msg_.encode_header();
                    T.deliver(read_msg_);
                    do_read_header();
                }
            }
            //Runs if the dealer has less than 20 credits.
            else if(T.D.credits < 20){
                string m = "Please wait 30 seconds while the dealer gets more credits.";
                strcpy(read_msg_.body(),m.c_str());
                read_msg_.gs.dealer_cards_valid = false;
                read_msg_.gs.player_cards_valid = false;
                read_msg_.body_length(strlen(read_msg_.body()));
                read_msg_.encode_header();
                T.deliver(read_msg_);
                do_read_header();
                T.D.ReCredit();
            }
            //Runs if the deck has less than 20 cards, also you can choose to use a test deck.
            else if(T.D.CardCount < 20){
                string m = "Shuffling Deck, please wait.";
                strcpy(read_msg_.body(),m.c_str());
                read_msg_.gs.dealer_cards_valid = false;
                read_msg_.gs.player_cards_valid = false;
                read_msg_.body_length(strlen(read_msg_.body()));
                read_msg_.encode_header();
                T.deliver(read_msg_);
                do_read_header();
                bool check = false;
                bool Tdeck = false;
                bool Ydeck = false;
                bool Ndeck = false;
                string Y = "Yes";
                string N = "No";
                string Choice;
                int FIXED_CARDS = 0;
                cout << "Would you like to use a Test Deck? (Yes/No): ";
                getline(cin, Choice);
                while(Tdeck == false){
                    if(Choice.compare(Y) == 0){
                        while(Ydeck == false){
                            cout << "Which Test Deck would you like to use? (1/2/3): ";
                            cin >> FIXED_CARDS;
                            if(FIXED_CARDS == 1){
                                cout << "Deck 1, Unshuffled; selected.\n";
                                T.D.createTestDeck(FIXED_CARDS);
                                check = T.D.verifyDeck();
                                Tdeck = true;
                                Ydeck = true;
                            }
                            else if(FIXED_CARDS == 2){
                                cout << "Deck 2, Forward evens, Backward odds; selected.\n";
                                T.D.createTestDeck(FIXED_CARDS);
                                Tdeck = true;
                                Ydeck = true;
                            }
                            else if(FIXED_CARDS == 3){
                                cout << "Deck 3, Unshuffled Backwards; selected.\n";
                                T.D.createTestDeck(FIXED_CARDS);
                                Tdeck = true;
                                Ydeck = true;
                            }
                            else{
                                cout << "Invalid entry, please choose again.\n";
                                cout << "Which Test Deck would you like to use? (1/2/3): ";
                                cin >> FIXED_CARDS;
                            }
                        }
                    }
                    else if(Choice.compare(N) == 0){
                        cout << "No test deck will be used.\n";
                        Tdeck = true;
                        Ndeck = true;
                    }
                    else{
                        cout << "Invalid entry, please choose again.\n";
                        cout << "Would you like to use a Test Deck? (Yes/No): ";
                        getline(cin, Choice);
                    }
                }
                if(Ndeck == true){
                    T.D.shuffleDeck();
                    check = T.D.verifyDeck();
                    while(check == false){
                        T.D.shuffleDeck();
                        check = T.D.verifyDeck();
                    }
                }
            }
            //Runs if the player has zero credits.
            else if(T.ReturnBalance(self) <= 0){
                string m = "You do not have enough credits, you can't play anymore. You will be kicked from the table in 10 seconds.";
                read_msg_.gs.player_quit = true;
                strcpy(read_msg_.body(),m.c_str());
                read_msg_.gs.dealer_cards_valid = false;
                read_msg_.gs.player_cards_valid = false;
                read_msg_.body_length(strlen(read_msg_.body()));
                read_msg_.encode_header();
                self->deliver(read_msg_);
                do_read_header();
                T.leave(self);
            }
            else{
                T.leave(shared_from_this());
            }
        });
    }
    //Writes the message.
    void do_write(){
        auto self(shared_from_this());
        asio::async_write(S, asio::buffer(write_msgs_.front().data(), write_msgs_.front().length()),
                          [this, self](error_code ec, size_t){
            if (!ec){
                write_msgs_.pop_front();
                if (!write_msgs_.empty()){
                    do_write();
                }
                T.D.SaveFile();
            }
            else{
                T.leave(shared_from_this());
            }
        });
    }
    tcp::socket S;
    blackjack_table& T;
    chat_message read_msg_;
    chat_message_queue write_msgs_;
};
//--------------------------------------------------------------------------------------------------
// Description: This class is used to create the blackjack game.
class blackjack_game{
public:
    //Builds the game.
    blackjack_game(asio::io_context& io_context, const tcp::endpoint& endpoint) :
    acceptor_(io_context, endpoint){
        bool check = false; //Used to store the verification for the shuffle.
        bool Tdeck = false; //Used to continue or break the Test deck senario.
        bool Ydeck = false; //Used to continue or break the Yes Test deck senario.
        bool Ndeck = false; //Used, if No is selected, so that they get a normal deck shuffle.
        string Y = "Yes"; //String to test Yes input.
        string N = "No"; //String to test No input.
        string Choice; //Where the String input is stored.
        int FIXED_CARDS = 0; //Where the Test deck option is stored.
        cout << "Creating a blackjack_game" << endl;
        T.D.buildCardLookup();
        //Asks the Server monitor if they want to use a test deck.
        cout << "Would you like to use a Test Deck? (Yes/No): ";
        getline(cin, Choice);
        //While loop to check if they want to use a Test deck.
        while(Tdeck == false){
            if(Choice.compare(Y) == 0){
                //While loop to see which Test deck to use.
                while(Ydeck == false){
                    //If Yes was selected.
                    cout << "Which Test Deck would you like to use? (1/2/3): ";
                    cin >> FIXED_CARDS;
                    if(FIXED_CARDS == 1){
                        //The unshuffled deck, goes in order of the 52 cards,
                        //repeats 5 times, 6 decks.
                        cout << "Deck 1, Unshuffled; selected.\n";
                        T.D.createTestDeck(FIXED_CARDS);
                        check = T.D.verifyDeck();
                        Tdeck = true;
                        Ydeck = true;
                    }
                    else if(FIXED_CARDS == 2){
                        //Takes out the odd cards, flips them around,
                        //and puts them back into the deck.
                        cout << "Deck 2, Forward evens, Backward odds; selected.\n";
                        T.D.createTestDeck(FIXED_CARDS);
                        Tdeck = true;
                        Ydeck = true;
                    }
                    else if(FIXED_CARDS == 3){
                        //The unshuffled deck, but reversed.
                        cout << "Deck 3, Unshuffled Backwards; selected.\n";
                        T.D.createTestDeck(FIXED_CARDS);
                        Tdeck = true;
                        Ydeck = true;
                    }
                    else{
                        //This is if the entry wasn't a 1, 2, or 3.
                        cout << "Invalid entry, please choose again.\n";
                        cout << "Which Test Deck would you like to use? (1/2/3): ";
                        cin >> FIXED_CARDS;
                    }
                }
            }
            else if(Choice.compare(N) == 0){
                //If No was selected.
                cout << "No test deck will be used.\n";
                Tdeck = true;
                Ndeck = true;
            }
            else{
                //This is if the entry wasn't Yes or No.
                cout << "Invalid entry, please choose again.\n";
                cout << "Would you like to use a Test Deck? (Yes/No): ";
                getline(cin, Choice);
            }
        }
        if(Ndeck == true){
            //If No was selected, makes a random deck.
            T.D.createDeck();
            T.D.shuffleDeck();
            check = T.D.verifyDeck();
            while(check == false){
                T.D.shuffleDeck();
                check = T.D.verifyDeck();
            }
        }
        //Used to inform whoever started the server that, every time a card is dealt,
        //a file called "Audit.txt" recieves a list of the cards that are still not dealt.
        cout << "A file called Audit.txt will be created and updated every time the deck changes.\n";
        do_accept();
    }
private:
    //Tests to see if the player should be accepted.
    void do_accept(){
        acceptor_.async_accept([this](error_code ec, tcp::socket socket){
            if (!ec){
                make_shared<blackjack_player>(move(socket), T)->start();
            }
            do_accept();
        });
    }
    tcp::acceptor acceptor_;
    blackjack_table T;
};
//--------------------------------------------------------------------------------------------------
//Description: The main of the server code. Starts the server.
int main(int argc, char* argv[]){
    try{
        if (argc < 2){
            cerr << "Usage: blackjack_game <port> [<port> ...]\n";
            return 1;
        }
        asio::io_context io_context;
        list<blackjack_game> servers;
        for (int i = 1; i < argc; ++i){
            tcp::endpoint endpoint(tcp::v4(), atoi(argv[i]));
            servers.emplace_back(io_context, endpoint);
        }
        io_context.run();
    }
    catch (exception& e){
        cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
