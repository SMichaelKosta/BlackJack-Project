//
// chat_message.hpp
// ~~~~~~~~~~~~~~~~
#ifndef CHAT_MESSAGE_HPP
#define CHAT_MESSAGE_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>
#include <deque>

using namespace std;
// Description: This class is used to pass data to the clients.
class game_state{
public:
    bool dealer_cards_valid; //True if the dealer's cards are valid
    int dealer_cards[20]; //List of the dealer's cards
    int dealer_points; //The sum of the dealer's card points
    int dealer_balance; //The sum of the dealer's credits
    bool player_cards_valid; //True if the player's cards are valid
    char player_name[6][25]; //List of the player's names
    int player_cards[6][20]; //List of the player's cards
    int player_points[6]; //The sum of the player's card points
    int player_wager[6]; //List of the player's wagers
    int player_balance[6]; //The sum of the player's credits
    int current_number; //Number passed to mark which hand goes where
    bool player_quit; //True if the player is quitting
};
// Description: This class is used to pass data to the server.
class client_action{
public:
    bool quit; //True if the player pressed quit
    bool hit; //True if the player pressed hit
    bool stand; //True if the player pressed stand
    bool join; //True if the player pressed join
    bool wager_valid; //True if the player pressed wager
    bool name_valid; //True if the player's name is valid
    char name[25]; //Contains the player's name
    char wager; //Contains the player's wager
};
// Description: This class is used to build the messages that are sent
// and recieved
class chat_message{
public:
    enum { header_length = 4 + sizeof(client_action) + sizeof(game_state) };
    enum { max_body_length = 512 };
    chat_message() : body_length_(0){
    }
    const char* data() const{
        return data_;
    }
    char* data(){
        return data_;
    }
    size_t length() const{
        return header_length + body_length_;
    }
    const char* body() const{
        return data_ + header_length;
    }
    char* body(){
        return data_ + header_length;
    }
    size_t body_length() const{
        return body_length_;
    }
    void body_length(size_t new_length){
        body_length_ = new_length;
        if (body_length_ > max_body_length)
            body_length_ = max_body_length;
    }
    bool decode_header(){
        char header[header_length + 1] = "";
        strncat(header, data_, 4);
        body_length_ = atoi(header);
        char *p = data_ + 4;
        memcpy(&ca,p,sizeof(client_action));
        memcpy(&gs,p+sizeof(client_action),sizeof(game_state));
        if (body_length_ > max_body_length){
            body_length_ = 0;
            return false;
        }
        return true;
    }
    void encode_header(){
        char header[4 + 1] = "";
        sprintf(header, "%4d", static_cast<int>(body_length_));
        memcpy(data_, header, 4);
        char *p = data_+4;
        memcpy(p,&ca,sizeof(client_action));
        memcpy(p+sizeof(client_action),&gs,sizeof(game_state));
    }
private:
    char data_[header_length + max_body_length];
    size_t body_length_;
public:
    client_action ca;
    game_state gs;
};

#endif // CHAT_MESSAGE_HPP
