//
// gtk_client.cpp
// ~~~~~~~~~~~~~~
#include <iostream>
#include <cstdlib>
#include <stdlib.h>
#include <deque>
#include <iostream>
#include <thread>
#include <fstream>
#include <assert.h>
#include "asio.hpp"
#include "chat_message.hpp"
#include <gtk/gtk.h>

GtkWidget *fromView  = NULL;        //Text from the chat server.
GtkWidget *dealerCards = NULL;      //Dealer output text info.
GtkWidget *playerCards = NULL;      //Player output text info.
GtkWidget *dcardrow;                //Dealer row of cards.
GtkWidget *pcardrow;                //Player row of cards.
GtkWidget *D1,*D2,*D3,*D4,*D5,*D6;  //Individual dealer cards.
GtkWidget *P1,*P2,*P3,*P4,*P5,*P6;  //Individual player cards.

int current = 7;                    //Player number, default 7.

using namespace std;
using asio::ip::tcp;

typedef deque<chat_message> chat_message_queue;
//Description: Used to out put the client text and change the client pictures.
class chat_client{
public:
    chat_client(asio::io_context& io_context, const tcp::resolver::results_type& endpoints) : io_context_(io_context), socket_(io_context){
        do_connect(endpoints);
    }
    void write(const chat_message& msg)
    {
        asio::post(io_context_, [this, msg](){
            bool write_in_progress = !write_msgs_.empty();
            write_msgs_.push_back(msg);
            if (!write_in_progress){
                do_write();
            }
        });
    }
    void close(){
        asio::post(io_context_, [this]() { socket_.close(); });
    }
private:
    void do_connect(const tcp::resolver::results_type& endpoints){
        asio::async_connect(socket_, endpoints, [this](error_code ec, tcp::endpoint){
            if (!ec){
                do_read_header();
            }
        });
    }
    void do_read_header(){
        asio::async_read(socket_, asio::buffer(read_msg_.data(), chat_message::header_length), [this](error_code ec, size_t){
            if (!ec && read_msg_.decode_header()){
                do_read_body();
            }
            else{
                socket_.close();
            }
        });
    }
    void do_read_body(){
        asio::async_read(socket_, asio::buffer(read_msg_.body(), read_msg_.body_length()), [this](error_code ec, size_t){
            if(current == 7 && read_msg_.gs.current_number != 7){
                current = read_msg_.gs.current_number;
            }
            if (!ec){
                gdk_threads_enter();
                {
                    char outline[read_msg_.body_length() + 2];
                    outline[0] = '\n';
                    outline[read_msg_.body_length() + 1] = '\0';
                    memcpy ( &outline[1], read_msg_.body(), read_msg_.body_length() );
                    gtk_label_set_text (GTK_LABEL(fromView), outline);
                    string p = "Players\n";
                    //Builds the players info text.
                    for(int i = 0; i < 6; i++){
                        if(read_msg_.gs.player_balance[i] != 0){
                            p = p + (read_msg_.gs.player_name[i]) + ": Wager: " + to_string(read_msg_.gs.player_wager[i]) + " | Credits: " + to_string(read_msg_.gs.player_balance[i]) + " | Hand Value: " +  to_string(read_msg_.gs.player_points[i]) + '\t';
                        }
                    }
                    //Builds the dealers info text.
                    string d = "Dealer: Credits: " + to_string(read_msg_.gs.dealer_balance) + " | Hand Value: " + to_string(read_msg_.gs.dealer_points);
                    //Changes the player card images, sorry it's long.
                    if (read_msg_.gs.player_cards_valid){
                        gtk_label_set_text (GTK_LABEL(playerCards),p.c_str());
                        for(int j = 0; j < 6; j++){
                            int ID = read_msg_.gs.player_cards[current-1][j];
                            if(j == 0){
                                if(ID == 320)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"No_Card.png");
                                else if(ID == 0 || ID == 52 || ID == 104 || ID == 156 || ID == 208 || ID == 260)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"AD.jpg");
                                else if(ID == 1 || ID == 53 || ID == 105 || ID == 157 || ID == 209 || ID == 261)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"AH.jpg");
                                else if(ID == 2 || ID == 54 || ID == 106 || ID == 158 || ID == 210 || ID == 262)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"AC.jpg");
                                else if(ID == 3 || ID == 55 || ID == 107 || ID == 159 || ID == 211 || ID == 263)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"AS.jpg");
                                else if(ID == 4 || ID == 56 || ID == 108 || ID == 160 || ID == 212 || ID == 264)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"2D.jpg");
                                else if(ID == 5 || ID == 57 || ID == 109 || ID == 161 || ID == 213 || ID == 265)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"2H.jpg");
                                else if(ID == 6 || ID == 58 || ID == 110 || ID == 162 || ID == 214 || ID == 266)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"2C.jpg");
                                else if(ID == 7 || ID == 59 || ID == 111 || ID == 163 || ID == 215 || ID == 267)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"2S.jpg");
                                else if(ID == 8 || ID == 60 || ID == 112 || ID == 164 || ID == 216 || ID == 268)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"3D.jpg");
                                else if(ID == 9 || ID == 61 || ID == 113 || ID == 165 || ID == 217 || ID == 269)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"3H.jpg");
                                else if(ID == 10 || ID == 62 || ID == 114 || ID == 166 || ID == 218 || ID == 270)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"3C.jpg");
                                else if(ID == 11 || ID == 63 || ID == 115 || ID == 167 || ID == 219 || ID == 271)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"3S.jpg");
                                else if(ID == 12 || ID == 64 || ID == 116 || ID == 168 || ID == 220 || ID == 272)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"4D.jpg");
                                else if(ID == 13 || ID == 65 || ID == 117 || ID == 169 || ID == 221 || ID == 273)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"4H.jpg");
                                else if(ID == 14 || ID == 66 || ID == 118 || ID == 170 || ID == 222 || ID == 274)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"4C.jpg");
                                else if(ID == 15 || ID == 67 || ID == 119 || ID == 171 || ID == 223 || ID == 275)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"4S.jpg");
                                else if(ID == 16 || ID == 68 || ID == 120 || ID == 172 || ID == 224 || ID == 276)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"5D.jpg");
                                else if(ID == 17 || ID == 69 || ID == 121 || ID == 173 || ID == 225 || ID == 277)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"5H.jpg");
                                else if(ID == 18 || ID == 70 || ID == 122 || ID == 174 || ID == 226 || ID == 278)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"5C.jpg");
                                else if(ID == 19 || ID == 71 || ID == 123 || ID == 175 || ID == 227 || ID == 279)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"5S.jpg");
                                else if(ID == 20 || ID == 72 || ID == 124 || ID == 176 || ID == 228 || ID == 280)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"6D.jpg");
                                else if(ID == 21 || ID == 73 || ID == 125 || ID == 177 || ID == 229 || ID == 281)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"6H.jpg");
                                else if(ID == 22 || ID == 74 || ID == 126 || ID == 178 || ID == 230 || ID == 282)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"6C.jpg");
                                else if(ID == 23 || ID == 75 || ID == 127 || ID == 179 || ID == 231 || ID == 283)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"6S.jpg");
                                else if(ID == 24 || ID == 76 || ID == 128 || ID == 180 || ID == 232 || ID == 284)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"7D.jpg");
                                else if(ID == 25 || ID == 77 || ID == 129 || ID == 181 || ID == 233 || ID == 285)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"7H.jpg");
                                else if(ID == 26 || ID == 78 || ID == 130 || ID == 182 || ID == 234 || ID == 286)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"7C.jpg");
                                else if(ID == 27 || ID == 79 || ID == 131 || ID == 183 || ID == 235 || ID == 287)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"7S.jpg");
                                else if(ID == 28 || ID == 80 || ID == 132 || ID == 184 || ID == 236 || ID == 288)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"8D.jpg");
                                else if(ID == 29 || ID == 81 || ID == 133 || ID == 185 || ID == 237 || ID == 289)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"8H.jpg");
                                else if(ID == 30 || ID == 82 || ID == 134 || ID == 186 || ID == 238 || ID == 290)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"8C.jpg");
                                else if(ID == 31 || ID == 83 || ID == 135 || ID == 187 || ID == 239 || ID == 291)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"8S.jpg");
                                else if(ID == 32 || ID == 84 || ID == 136 || ID == 188 || ID == 240 || ID == 292)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"9D.jpg");
                                else if(ID == 33 || ID == 85 || ID == 137 || ID == 189 || ID == 241 || ID == 293)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"9H.jpg");
                                else if(ID == 34 || ID == 86 || ID == 138 || ID == 190 || ID == 242 || ID == 294)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"9C.jpg");
                                else if(ID == 35 || ID == 87 || ID == 139 || ID == 191 || ID == 243 || ID == 295)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"9S.jpg");
                                else if(ID == 36 || ID == 88 || ID == 140 || ID == 192 || ID == 244 || ID == 296)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"10D.jpg");
                                else if(ID == 37 || ID == 89 || ID == 141 || ID == 193 || ID == 245 || ID == 297)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"10H.jpg");
                                else if(ID == 38 || ID == 90 || ID == 142 || ID == 194 || ID == 246 || ID == 298)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"10C.jpg");
                                else if(ID == 39 || ID == 91 || ID == 143 || ID == 195 || ID == 247 || ID == 299)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"10S.jpg");
                                else if(ID == 40 || ID == 92 || ID == 144 || ID == 196 || ID == 248 || ID == 300)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"JD.jpg");
                                else if(ID == 41 || ID == 93 || ID == 145 || ID == 197 || ID == 249 || ID == 301)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"JH.jpg");
                                else if(ID == 42 || ID == 94 || ID == 146 || ID == 198 || ID == 250 || ID == 302)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"JC.jpg");
                                else if(ID == 43 || ID == 95 || ID == 147 || ID == 199 || ID == 251 || ID == 303)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"JS.jpg");
                                else if(ID == 44 || ID == 96 || ID == 148 || ID == 200 || ID == 252 || ID == 304)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"QD.jpg");
                                else if(ID == 45 || ID == 97 || ID == 149 || ID == 201 || ID == 253 || ID == 305)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"QH.jpg");
                                else if(ID == 46 || ID == 98 || ID == 150 || ID == 202 || ID == 254 || ID == 306)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"QC.jpg");
                                else if(ID == 47 || ID == 99 || ID == 151 || ID == 203 || ID == 255 || ID == 307)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"QS.jpg");
                                else if(ID == 48 || ID == 100 || ID == 152 || ID == 204 || ID == 256 || ID == 308)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"KD.jpg");
                                else if(ID == 49 || ID == 101 || ID == 153 || ID == 205 || ID == 257 || ID == 309)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"KH.jpg");
                                else if(ID == 50 || ID == 102 || ID == 154 || ID == 206 || ID == 258 || ID == 310)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"KC.jpg");
                                else if(ID == 51 || ID == 103 || ID == 155 || ID == 207 || ID == 259 || ID == 311)
                                    gtk_image_set_from_file(GTK_IMAGE(P1),"KS.jpg");
                            }
                            else if(j == 1){
                                if(ID == 320)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"No_Card.png");
                                else if(ID == 0 || ID == 52 || ID == 104 || ID == 156 || ID == 208 || ID == 260)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"AD.jpg");
                                else if(ID == 1 || ID == 53 || ID == 105 || ID == 157 || ID == 209 || ID == 261)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"AH.jpg");
                                else if(ID == 2 || ID == 54 || ID == 106 || ID == 158 || ID == 210 || ID == 262)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"AC.jpg");
                                else if(ID == 3 || ID == 55 || ID == 107 || ID == 159 || ID == 211 || ID == 263)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"AS.jpg");
                                else if(ID == 4 || ID == 56 || ID == 108 || ID == 160 || ID == 212 || ID == 264)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"2D.jpg");
                                else if(ID == 5 || ID == 57 || ID == 109 || ID == 161 || ID == 213 || ID == 265)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"2H.jpg");
                                else if(ID == 6 || ID == 58 || ID == 110 || ID == 162 || ID == 214 || ID == 266)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"2C.jpg");
                                else if(ID == 7 || ID == 59 || ID == 111 || ID == 163 || ID == 215 || ID == 267)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"2S.jpg");
                                else if(ID == 8 || ID == 60 || ID == 112 || ID == 164 || ID == 216 || ID == 268)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"3D.jpg");
                                else if(ID == 9 || ID == 61 || ID == 113 || ID == 165 || ID == 217 || ID == 269)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"3H.jpg");
                                else if(ID == 10 || ID == 62 || ID == 114 || ID == 166 || ID == 218 || ID == 270)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"3C.jpg");
                                else if(ID == 11 || ID == 63 || ID == 115 || ID == 167 || ID == 219 || ID == 271)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"3S.jpg");
                                else if(ID == 12 || ID == 64 || ID == 116 || ID == 168 || ID == 220 || ID == 272)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"4D.jpg");
                                else if(ID == 13 || ID == 65 || ID == 117 || ID == 169 || ID == 221 || ID == 273)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"4H.jpg");
                                else if(ID == 14 || ID == 66 || ID == 118 || ID == 170 || ID == 222 || ID == 274)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"4C.jpg");
                                else if(ID == 15 || ID == 67 || ID == 119 || ID == 171 || ID == 223 || ID == 275)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"4S.jpg");
                                else if(ID == 16 || ID == 68 || ID == 120 || ID == 172 || ID == 224 || ID == 276)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"5D.jpg");
                                else if(ID == 17 || ID == 69 || ID == 121 || ID == 173 || ID == 225 || ID == 277)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"5H.jpg");
                                else if(ID == 18 || ID == 70 || ID == 122 || ID == 174 || ID == 226 || ID == 278)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"5C.jpg");
                                else if(ID == 19 || ID == 71 || ID == 123 || ID == 175 || ID == 227 || ID == 279)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"5S.jpg");
                                else if(ID == 20 || ID == 72 || ID == 124 || ID == 176 || ID == 228 || ID == 280)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"6D.jpg");
                                else if(ID == 21 || ID == 73 || ID == 125 || ID == 177 || ID == 229 || ID == 281)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"6H.jpg");
                                else if(ID == 22 || ID == 74 || ID == 126 || ID == 178 || ID == 230 || ID == 282)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"6C.jpg");
                                else if(ID == 23 || ID == 75 || ID == 127 || ID == 179 || ID == 231 || ID == 283)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"6S.jpg");
                                else if(ID == 24 || ID == 76 || ID == 128 || ID == 180 || ID == 232 || ID == 284)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"7D.jpg");
                                else if(ID == 25 || ID == 77 || ID == 129 || ID == 181 || ID == 233 || ID == 285)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"7H.jpg");
                                else if(ID == 26 || ID == 78 || ID == 130 || ID == 182 || ID == 234 || ID == 286)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"7C.jpg");
                                else if(ID == 27 || ID == 79 || ID == 131 || ID == 183 || ID == 235 || ID == 287)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"7S.jpg");
                                else if(ID == 28 || ID == 80 || ID == 132 || ID == 184 || ID == 236 || ID == 288)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"8D.jpg");
                                else if(ID == 29 || ID == 81 || ID == 133 || ID == 185 || ID == 237 || ID == 289)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"8H.jpg");
                                else if(ID == 30 || ID == 82 || ID == 134 || ID == 186 || ID == 238 || ID == 290)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"8C.jpg");
                                else if(ID == 31 || ID == 83 || ID == 135 || ID == 187 || ID == 239 || ID == 291)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"8S.jpg");
                                else if(ID == 32 || ID == 84 || ID == 136 || ID == 188 || ID == 240 || ID == 292)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"9D.jpg");
                                else if(ID == 33 || ID == 85 || ID == 137 || ID == 189 || ID == 241 || ID == 293)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"9H.jpg");
                                else if(ID == 34 || ID == 86 || ID == 138 || ID == 190 || ID == 242 || ID == 294)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"9C.jpg");
                                else if(ID == 35 || ID == 87 || ID == 139 || ID == 191 || ID == 243 || ID == 295)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"9S.jpg");
                                else if(ID == 36 || ID == 88 || ID == 140 || ID == 192 || ID == 244 || ID == 296)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"10D.jpg");
                                else if(ID == 37 || ID == 89 || ID == 141 || ID == 193 || ID == 245 || ID == 297)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"10H.jpg");
                                else if(ID == 38 || ID == 90 || ID == 142 || ID == 194 || ID == 246 || ID == 298)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"10C.jpg");
                                else if(ID == 39 || ID == 91 || ID == 143 || ID == 195 || ID == 247 || ID == 299)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"10S.jpg");
                                else if(ID == 40 || ID == 92 || ID == 144 || ID == 196 || ID == 248 || ID == 300)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"JD.jpg");
                                else if(ID == 41 || ID == 93 || ID == 145 || ID == 197 || ID == 249 || ID == 301)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"JH.jpg");
                                else if(ID == 42 || ID == 94 || ID == 146 || ID == 198 || ID == 250 || ID == 302)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"JC.jpg");
                                else if(ID == 43 || ID == 95 || ID == 147 || ID == 199 || ID == 251 || ID == 303)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"JS.jpg");
                                else if(ID == 44 || ID == 96 || ID == 148 || ID == 200 || ID == 252 || ID == 304)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"QD.jpg");
                                else if(ID == 45 || ID == 97 || ID == 149 || ID == 201 || ID == 253 || ID == 305)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"QH.jpg");
                                else if(ID == 46 || ID == 98 || ID == 150 || ID == 202 || ID == 254 || ID == 306)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"QC.jpg");
                                else if(ID == 47 || ID == 99 || ID == 151 || ID == 203 || ID == 255 || ID == 307)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"QS.jpg");
                                else if(ID == 48 || ID == 100 || ID == 152 || ID == 204 || ID == 256 || ID == 308)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"KD.jpg");
                                else if(ID == 49 || ID == 101 || ID == 153 || ID == 205 || ID == 257 || ID == 309)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"KH.jpg");
                                else if(ID == 50 || ID == 102 || ID == 154 || ID == 206 || ID == 258 || ID == 310)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"KC.jpg");
                                else if(ID == 51 || ID == 103 || ID == 155 || ID == 207 || ID == 259 || ID == 311)
                                    gtk_image_set_from_file(GTK_IMAGE(P2),"KS.jpg");
                            }
                            else if(j == 2){
                                if(ID == 320)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"No_Card.png");
                                else if(ID == 0 || ID == 52 || ID == 104 || ID == 156 || ID == 208 || ID == 260)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"AD.jpg");
                                else if(ID == 1 || ID == 53 || ID == 105 || ID == 157 || ID == 209 || ID == 261)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"AH.jpg");
                                else if(ID == 2 || ID == 54 || ID == 106 || ID == 158 || ID == 210 || ID == 262)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"AC.jpg");
                                else if(ID == 3 || ID == 55 || ID == 107 || ID == 159 || ID == 211 || ID == 263)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"AS.jpg");
                                else if(ID == 4 || ID == 56 || ID == 108 || ID == 160 || ID == 212 || ID == 264)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"2D.jpg");
                                else if(ID == 5 || ID == 57 || ID == 109 || ID == 161 || ID == 213 || ID == 265)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"2H.jpg");
                                else if(ID == 6 || ID == 58 || ID == 110 || ID == 162 || ID == 214 || ID == 266)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"2C.jpg");
                                else if(ID == 7 || ID == 59 || ID == 111 || ID == 163 || ID == 215 || ID == 267)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"2S.jpg");
                                else if(ID == 8 || ID == 60 || ID == 112 || ID == 164 || ID == 216 || ID == 268)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"3D.jpg");
                                else if(ID == 9 || ID == 61 || ID == 113 || ID == 165 || ID == 217 || ID == 269)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"3H.jpg");
                                else if(ID == 10 || ID == 62 || ID == 114 || ID == 166 || ID == 218 || ID == 270)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"3C.jpg");
                                else if(ID == 11 || ID == 63 || ID == 115 || ID == 167 || ID == 219 || ID == 271)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"3S.jpg");
                                else if(ID == 12 || ID == 64 || ID == 116 || ID == 168 || ID == 220 || ID == 272)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"4D.jpg");
                                else if(ID == 13 || ID == 65 || ID == 117 || ID == 169 || ID == 221 || ID == 273)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"4H.jpg");
                                else if(ID == 14 || ID == 66 || ID == 118 || ID == 170 || ID == 222 || ID == 274)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"4C.jpg");
                                else if(ID == 15 || ID == 67 || ID == 119 || ID == 171 || ID == 223 || ID == 275)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"4S.jpg");
                                else if(ID == 16 || ID == 68 || ID == 120 || ID == 172 || ID == 224 || ID == 276)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"5D.jpg");
                                else if(ID == 17 || ID == 69 || ID == 121 || ID == 173 || ID == 225 || ID == 277)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"5H.jpg");
                                else if(ID == 18 || ID == 70 || ID == 122 || ID == 174 || ID == 226 || ID == 278)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"5C.jpg");
                                else if(ID == 19 || ID == 71 || ID == 123 || ID == 175 || ID == 227 || ID == 279)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"5S.jpg");
                                else if(ID == 20 || ID == 72 || ID == 124 || ID == 176 || ID == 228 || ID == 280)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"6D.jpg");
                                else if(ID == 21 || ID == 73 || ID == 125 || ID == 177 || ID == 229 || ID == 281)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"6H.jpg");
                                else if(ID == 22 || ID == 74 || ID == 126 || ID == 178 || ID == 230 || ID == 282)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"6C.jpg");
                                else if(ID == 23 || ID == 75 || ID == 127 || ID == 179 || ID == 231 || ID == 283)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"6S.jpg");
                                else if(ID == 24 || ID == 76 || ID == 128 || ID == 180 || ID == 232 || ID == 284)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"7D.jpg");
                                else if(ID == 25 || ID == 77 || ID == 129 || ID == 181 || ID == 233 || ID == 285)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"7H.jpg");
                                else if(ID == 26 || ID == 78 || ID == 130 || ID == 182 || ID == 234 || ID == 286)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"7C.jpg");
                                else if(ID == 27 || ID == 79 || ID == 131 || ID == 183 || ID == 235 || ID == 287)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"7S.jpg");
                                else if(ID == 28 || ID == 80 || ID == 132 || ID == 184 || ID == 236 || ID == 288)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"8D.jpg");
                                else if(ID == 29 || ID == 81 || ID == 133 || ID == 185 || ID == 237 || ID == 289)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"8H.jpg");
                                else if(ID == 30 || ID == 82 || ID == 134 || ID == 186 || ID == 238 || ID == 290)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"8C.jpg");
                                else if(ID == 31 || ID == 83 || ID == 135 || ID == 187 || ID == 239 || ID == 291)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"8S.jpg");
                                else if(ID == 32 || ID == 84 || ID == 136 || ID == 188 || ID == 240 || ID == 292)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"9D.jpg");
                                else if(ID == 33 || ID == 85 || ID == 137 || ID == 189 || ID == 241 || ID == 293)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"9H.jpg");
                                else if(ID == 34 || ID == 86 || ID == 138 || ID == 190 || ID == 242 || ID == 294)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"9C.jpg");
                                else if(ID == 35 || ID == 87 || ID == 139 || ID == 191 || ID == 243 || ID == 295)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"9S.jpg");
                                else if(ID == 36 || ID == 88 || ID == 140 || ID == 192 || ID == 244 || ID == 296)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"10D.jpg");
                                else if(ID == 37 || ID == 89 || ID == 141 || ID == 193 || ID == 245 || ID == 297)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"10H.jpg");
                                else if(ID == 38 || ID == 90 || ID == 142 || ID == 194 || ID == 246 || ID == 298)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"10C.jpg");
                                else if(ID == 39 || ID == 91 || ID == 143 || ID == 195 || ID == 247 || ID == 299)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"10S.jpg");
                                else if(ID == 40 || ID == 92 || ID == 144 || ID == 196 || ID == 248 || ID == 300)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"JD.jpg");
                                else if(ID == 41 || ID == 93 || ID == 145 || ID == 197 || ID == 249 || ID == 301)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"JH.jpg");
                                else if(ID == 42 || ID == 94 || ID == 146 || ID == 198 || ID == 250 || ID == 302)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"JC.jpg");
                                else if(ID == 43 || ID == 95 || ID == 147 || ID == 199 || ID == 251 || ID == 303)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"JS.jpg");
                                else if(ID == 44 || ID == 96 || ID == 148 || ID == 200 || ID == 252 || ID == 304)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"QD.jpg");
                                else if(ID == 45 || ID == 97 || ID == 149 || ID == 201 || ID == 253 || ID == 305)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"QH.jpg");
                                else if(ID == 46 || ID == 98 || ID == 150 || ID == 202 || ID == 254 || ID == 306)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"QC.jpg");
                                else if(ID == 47 || ID == 99 || ID == 151 || ID == 203 || ID == 255 || ID == 307)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"QS.jpg");
                                else if(ID == 48 || ID == 100 || ID == 152 || ID == 204 || ID == 256 || ID == 308)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"KD.jpg");
                                else if(ID == 49 || ID == 101 || ID == 153 || ID == 205 || ID == 257 || ID == 309)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"KH.jpg");
                                else if(ID == 50 || ID == 102 || ID == 154 || ID == 206 || ID == 258 || ID == 310)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"KC.jpg");
                                else if(ID == 51 || ID == 103 || ID == 155 || ID == 207 || ID == 259 || ID == 311)
                                    gtk_image_set_from_file(GTK_IMAGE(P3),"KS.jpg");
                            }
                            else if(j == 3){
                                if(ID == 320)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"No_Card.png");
                                else if(ID == 0 || ID == 52 || ID == 104 || ID == 156 || ID == 208 || ID == 260)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"AD.jpg");
                                else if(ID == 1 || ID == 53 || ID == 105 || ID == 157 || ID == 209 || ID == 261)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"AH.jpg");
                                else if(ID == 2 || ID == 54 || ID == 106 || ID == 158 || ID == 210 || ID == 262)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"AC.jpg");
                                else if(ID == 3 || ID == 55 || ID == 107 || ID == 159 || ID == 211 || ID == 263)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"AS.jpg");
                                else if(ID == 4 || ID == 56 || ID == 108 || ID == 160 || ID == 212 || ID == 264)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"2D.jpg");
                                else if(ID == 5 || ID == 57 || ID == 109 || ID == 161 || ID == 213 || ID == 265)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"2H.jpg");
                                else if(ID == 6 || ID == 58 || ID == 110 || ID == 162 || ID == 214 || ID == 266)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"2C.jpg");
                                else if(ID == 7 || ID == 59 || ID == 111 || ID == 163 || ID == 215 || ID == 267)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"2S.jpg");
                                else if(ID == 8 || ID == 60 || ID == 112 || ID == 164 || ID == 216 || ID == 268)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"3D.jpg");
                                else if(ID == 9 || ID == 61 || ID == 113 || ID == 165 || ID == 217 || ID == 269)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"3H.jpg");
                                else if(ID == 10 || ID == 62 || ID == 114 || ID == 166 || ID == 218 || ID == 270)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"3C.jpg");
                                else if(ID == 11 || ID == 63 || ID == 115 || ID == 167 || ID == 219 || ID == 271)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"3S.jpg");
                                else if(ID == 12 || ID == 64 || ID == 116 || ID == 168 || ID == 220 || ID == 272)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"4D.jpg");
                                else if(ID == 13 || ID == 65 || ID == 117 || ID == 169 || ID == 221 || ID == 273)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"4H.jpg");
                                else if(ID == 14 || ID == 66 || ID == 118 || ID == 170 || ID == 222 || ID == 274)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"4C.jpg");
                                else if(ID == 15 || ID == 67 || ID == 119 || ID == 171 || ID == 223 || ID == 275)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"4S.jpg");
                                else if(ID == 16 || ID == 68 || ID == 120 || ID == 172 || ID == 224 || ID == 276)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"5D.jpg");
                                else if(ID == 17 || ID == 69 || ID == 121 || ID == 173 || ID == 225 || ID == 277)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"5H.jpg");
                                else if(ID == 18 || ID == 70 || ID == 122 || ID == 174 || ID == 226 || ID == 278)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"5C.jpg");
                                else if(ID == 19 || ID == 71 || ID == 123 || ID == 175 || ID == 227 || ID == 279)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"5S.jpg");
                                else if(ID == 20 || ID == 72 || ID == 124 || ID == 176 || ID == 228 || ID == 280)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"6D.jpg");
                                else if(ID == 21 || ID == 73 || ID == 125 || ID == 177 || ID == 229 || ID == 281)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"6H.jpg");
                                else if(ID == 22 || ID == 74 || ID == 126 || ID == 178 || ID == 230 || ID == 282)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"6C.jpg");
                                else if(ID == 23 || ID == 75 || ID == 127 || ID == 179 || ID == 231 || ID == 283)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"6S.jpg");
                                else if(ID == 24 || ID == 76 || ID == 128 || ID == 180 || ID == 232 || ID == 284)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"7D.jpg");
                                else if(ID == 25 || ID == 77 || ID == 129 || ID == 181 || ID == 233 || ID == 285)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"7H.jpg");
                                else if(ID == 26 || ID == 78 || ID == 130 || ID == 182 || ID == 234 || ID == 286)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"7C.jpg");
                                else if(ID == 27 || ID == 79 || ID == 131 || ID == 183 || ID == 235 || ID == 287)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"7S.jpg");
                                else if(ID == 28 || ID == 80 || ID == 132 || ID == 184 || ID == 236 || ID == 288)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"8D.jpg");
                                else if(ID == 29 || ID == 81 || ID == 133 || ID == 185 || ID == 237 || ID == 289)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"8H.jpg");
                                else if(ID == 30 || ID == 82 || ID == 134 || ID == 186 || ID == 238 || ID == 290)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"8C.jpg");
                                else if(ID == 31 || ID == 83 || ID == 135 || ID == 187 || ID == 239 || ID == 291)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"8S.jpg");
                                else if(ID == 32 || ID == 84 || ID == 136 || ID == 188 || ID == 240 || ID == 292)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"9D.jpg");
                                else if(ID == 33 || ID == 85 || ID == 137 || ID == 189 || ID == 241 || ID == 293)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"9H.jpg");
                                else if(ID == 34 || ID == 86 || ID == 138 || ID == 190 || ID == 242 || ID == 294)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"9C.jpg");
                                else if(ID == 35 || ID == 87 || ID == 139 || ID == 191 || ID == 243 || ID == 295)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"9S.jpg");
                                else if(ID == 36 || ID == 88 || ID == 140 || ID == 192 || ID == 244 || ID == 296)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"10D.jpg");
                                else if(ID == 37 || ID == 89 || ID == 141 || ID == 193 || ID == 245 || ID == 297)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"10H.jpg");
                                else if(ID == 38 || ID == 90 || ID == 142 || ID == 194 || ID == 246 || ID == 298)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"10C.jpg");
                                else if(ID == 39 || ID == 91 || ID == 143 || ID == 195 || ID == 247 || ID == 299)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"10S.jpg");
                                else if(ID == 40 || ID == 92 || ID == 144 || ID == 196 || ID == 248 || ID == 300)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"JD.jpg");
                                else if(ID == 41 || ID == 93 || ID == 145 || ID == 197 || ID == 249 || ID == 301)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"JH.jpg");
                                else if(ID == 42 || ID == 94 || ID == 146 || ID == 198 || ID == 250 || ID == 302)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"JC.jpg");
                                else if(ID == 43 || ID == 95 || ID == 147 || ID == 199 || ID == 251 || ID == 303)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"JS.jpg");
                                else if(ID == 44 || ID == 96 || ID == 148 || ID == 200 || ID == 252 || ID == 304)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"QD.jpg");
                                else if(ID == 45 || ID == 97 || ID == 149 || ID == 201 || ID == 253 || ID == 305)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"QH.jpg");
                                else if(ID == 46 || ID == 98 || ID == 150 || ID == 202 || ID == 254 || ID == 306)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"QC.jpg");
                                else if(ID == 47 || ID == 99 || ID == 151 || ID == 203 || ID == 255 || ID == 307)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"QS.jpg");
                                else if(ID == 48 || ID == 100 || ID == 152 || ID == 204 || ID == 256 || ID == 308)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"KD.jpg");
                                else if(ID == 49 || ID == 101 || ID == 153 || ID == 205 || ID == 257 || ID == 309)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"KH.jpg");
                                else if(ID == 50 || ID == 102 || ID == 154 || ID == 206 || ID == 258 || ID == 310)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"KC.jpg");
                                else if(ID == 51 || ID == 103 || ID == 155 || ID == 207 || ID == 259 || ID == 311)
                                    gtk_image_set_from_file(GTK_IMAGE(P4),"KS.jpg");
                            }
                            else if(j == 4){
                                if(ID == 320)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"No_Card.png");
                                else if(ID == 0 || ID == 52 || ID == 104 || ID == 156 || ID == 208 || ID == 260)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"AD.jpg");
                                else if(ID == 1 || ID == 53 || ID == 105 || ID == 157 || ID == 209 || ID == 261)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"AH.jpg");
                                else if(ID == 2 || ID == 54 || ID == 106 || ID == 158 || ID == 210 || ID == 262)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"AC.jpg");
                                else if(ID == 3 || ID == 55 || ID == 107 || ID == 159 || ID == 211 || ID == 263)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"AS.jpg");
                                else if(ID == 4 || ID == 56 || ID == 108 || ID == 160 || ID == 212 || ID == 264)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"2D.jpg");
                                else if(ID == 5 || ID == 57 || ID == 109 || ID == 161 || ID == 213 || ID == 265)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"2H.jpg");
                                else if(ID == 6 || ID == 58 || ID == 110 || ID == 162 || ID == 214 || ID == 266)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"2C.jpg");
                                else if(ID == 7 || ID == 59 || ID == 111 || ID == 163 || ID == 215 || ID == 267)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"2S.jpg");
                                else if(ID == 8 || ID == 60 || ID == 112 || ID == 164 || ID == 216 || ID == 268)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"3D.jpg");
                                else if(ID == 9 || ID == 61 || ID == 113 || ID == 165 || ID == 217 || ID == 269)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"3H.jpg");
                                else if(ID == 10 || ID == 62 || ID == 114 || ID == 166 || ID == 218 || ID == 270)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"3C.jpg");
                                else if(ID == 11 || ID == 63 || ID == 115 || ID == 167 || ID == 219 || ID == 271)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"3S.jpg");
                                else if(ID == 12 || ID == 64 || ID == 116 || ID == 168 || ID == 220 || ID == 272)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"4D.jpg");
                                else if(ID == 13 || ID == 65 || ID == 117 || ID == 169 || ID == 221 || ID == 273)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"4H.jpg");
                                else if(ID == 14 || ID == 66 || ID == 118 || ID == 170 || ID == 222 || ID == 274)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"4C.jpg");
                                else if(ID == 15 || ID == 67 || ID == 119 || ID == 171 || ID == 223 || ID == 275)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"4S.jpg");
                                else if(ID == 16 || ID == 68 || ID == 120 || ID == 172 || ID == 224 || ID == 276)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"5D.jpg");
                                else if(ID == 17 || ID == 69 || ID == 121 || ID == 173 || ID == 225 || ID == 277)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"5H.jpg");
                                else if(ID == 18 || ID == 70 || ID == 122 || ID == 174 || ID == 226 || ID == 278)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"5C.jpg");
                                else if(ID == 19 || ID == 71 || ID == 123 || ID == 175 || ID == 227 || ID == 279)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"5S.jpg");
                                else if(ID == 20 || ID == 72 || ID == 124 || ID == 176 || ID == 228 || ID == 280)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"6D.jpg");
                                else if(ID == 21 || ID == 73 || ID == 125 || ID == 177 || ID == 229 || ID == 281)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"6H.jpg");
                                else if(ID == 22 || ID == 74 || ID == 126 || ID == 178 || ID == 230 || ID == 282)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"6C.jpg");
                                else if(ID == 23 || ID == 75 || ID == 127 || ID == 179 || ID == 231 || ID == 283)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"6S.jpg");
                                else if(ID == 24 || ID == 76 || ID == 128 || ID == 180 || ID == 232 || ID == 284)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"7D.jpg");
                                else if(ID == 25 || ID == 77 || ID == 129 || ID == 181 || ID == 233 || ID == 285)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"7H.jpg");
                                else if(ID == 26 || ID == 78 || ID == 130 || ID == 182 || ID == 234 || ID == 286)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"7C.jpg");
                                else if(ID == 27 || ID == 79 || ID == 131 || ID == 183 || ID == 235 || ID == 287)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"7S.jpg");
                                else if(ID == 28 || ID == 80 || ID == 132 || ID == 184 || ID == 236 || ID == 288)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"8D.jpg");
                                else if(ID == 29 || ID == 81 || ID == 133 || ID == 185 || ID == 237 || ID == 289)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"8H.jpg");
                                else if(ID == 30 || ID == 82 || ID == 134 || ID == 186 || ID == 238 || ID == 290)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"8C.jpg");
                                else if(ID == 31 || ID == 83 || ID == 135 || ID == 187 || ID == 239 || ID == 291)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"8S.jpg");
                                else if(ID == 32 || ID == 84 || ID == 136 || ID == 188 || ID == 240 || ID == 292)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"9D.jpg");
                                else if(ID == 33 || ID == 85 || ID == 137 || ID == 189 || ID == 241 || ID == 293)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"9H.jpg");
                                else if(ID == 34 || ID == 86 || ID == 138 || ID == 190 || ID == 242 || ID == 294)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"9C.jpg");
                                else if(ID == 35 || ID == 87 || ID == 139 || ID == 191 || ID == 243 || ID == 295)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"9S.jpg");
                                else if(ID == 36 || ID == 88 || ID == 140 || ID == 192 || ID == 244 || ID == 296)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"10D.jpg");
                                else if(ID == 37 || ID == 89 || ID == 141 || ID == 193 || ID == 245 || ID == 297)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"10H.jpg");
                                else if(ID == 38 || ID == 90 || ID == 142 || ID == 194 || ID == 246 || ID == 298)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"10C.jpg");
                                else if(ID == 39 || ID == 91 || ID == 143 || ID == 195 || ID == 247 || ID == 299)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"10S.jpg");
                                else if(ID == 40 || ID == 92 || ID == 144 || ID == 196 || ID == 248 || ID == 300)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"JD.jpg");
                                else if(ID == 41 || ID == 93 || ID == 145 || ID == 197 || ID == 249 || ID == 301)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"JH.jpg");
                                else if(ID == 42 || ID == 94 || ID == 146 || ID == 198 || ID == 250 || ID == 302)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"JC.jpg");
                                else if(ID == 43 || ID == 95 || ID == 147 || ID == 199 || ID == 251 || ID == 303)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"JS.jpg");
                                else if(ID == 44 || ID == 96 || ID == 148 || ID == 200 || ID == 252 || ID == 304)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"QD.jpg");
                                else if(ID == 45 || ID == 97 || ID == 149 || ID == 201 || ID == 253 || ID == 305)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"QH.jpg");
                                else if(ID == 46 || ID == 98 || ID == 150 || ID == 202 || ID == 254 || ID == 306)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"QC.jpg");
                                else if(ID == 47 || ID == 99 || ID == 151 || ID == 203 || ID == 255 || ID == 307)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"QS.jpg");
                                else if(ID == 48 || ID == 100 || ID == 152 || ID == 204 || ID == 256 || ID == 308)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"KD.jpg");
                                else if(ID == 49 || ID == 101 || ID == 153 || ID == 205 || ID == 257 || ID == 309)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"KH.jpg");
                                else if(ID == 50 || ID == 102 || ID == 154 || ID == 206 || ID == 258 || ID == 310)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"KC.jpg");
                                else if(ID == 51 || ID == 103 || ID == 155 || ID == 207 || ID == 259 || ID == 311)
                                    gtk_image_set_from_file(GTK_IMAGE(P5),"KS.jpg");
                            }
                            else{
                                if(ID == 320)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"No_Card.png");
                                else if(ID == 0 || ID == 52 || ID == 104 || ID == 156 || ID == 208 || ID == 260)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"AD.jpg");
                                else if(ID == 1 || ID == 53 || ID == 105 || ID == 157 || ID == 209 || ID == 261)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"AH.jpg");
                                else if(ID == 2 || ID == 54 || ID == 106 || ID == 158 || ID == 210 || ID == 262)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"AC.jpg");
                                else if(ID == 3 || ID == 55 || ID == 107 || ID == 159 || ID == 211 || ID == 263)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"AS.jpg");
                                else if(ID == 4 || ID == 56 || ID == 108 || ID == 160 || ID == 212 || ID == 264)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"2D.jpg");
                                else if(ID == 5 || ID == 57 || ID == 109 || ID == 161 || ID == 213 || ID == 265)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"2H.jpg");
                                else if(ID == 6 || ID == 58 || ID == 110 || ID == 162 || ID == 214 || ID == 266)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"2C.jpg");
                                else if(ID == 7 || ID == 59 || ID == 111 || ID == 163 || ID == 215 || ID == 267)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"2S.jpg");
                                else if(ID == 8 || ID == 60 || ID == 112 || ID == 164 || ID == 216 || ID == 268)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"3D.jpg");
                                else if(ID == 9 || ID == 61 || ID == 113 || ID == 165 || ID == 217 || ID == 269)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"3H.jpg");
                                else if(ID == 10 || ID == 62 || ID == 114 || ID == 166 || ID == 218 || ID == 270)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"3C.jpg");
                                else if(ID == 11 || ID == 63 || ID == 115 || ID == 167 || ID == 219 || ID == 271)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"3S.jpg");
                                else if(ID == 12 || ID == 64 || ID == 116 || ID == 168 || ID == 220 || ID == 272)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"4D.jpg");
                                else if(ID == 13 || ID == 65 || ID == 117 || ID == 169 || ID == 221 || ID == 273)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"4H.jpg");
                                else if(ID == 14 || ID == 66 || ID == 118 || ID == 170 || ID == 222 || ID == 274)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"4C.jpg");
                                else if(ID == 15 || ID == 67 || ID == 119 || ID == 171 || ID == 223 || ID == 275)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"4S.jpg");
                                else if(ID == 16 || ID == 68 || ID == 120 || ID == 172 || ID == 224 || ID == 276)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"5D.jpg");
                                else if(ID == 17 || ID == 69 || ID == 121 || ID == 173 || ID == 225 || ID == 277)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"5H.jpg");
                                else if(ID == 18 || ID == 70 || ID == 122 || ID == 174 || ID == 226 || ID == 278)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"5C.jpg");
                                else if(ID == 19 || ID == 71 || ID == 123 || ID == 175 || ID == 227 || ID == 279)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"5S.jpg");
                                else if(ID == 20 || ID == 72 || ID == 124 || ID == 176 || ID == 228 || ID == 280)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"6D.jpg");
                                else if(ID == 21 || ID == 73 || ID == 125 || ID == 177 || ID == 229 || ID == 281)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"6H.jpg");
                                else if(ID == 22 || ID == 74 || ID == 126 || ID == 178 || ID == 230 || ID == 282)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"6C.jpg");
                                else if(ID == 23 || ID == 75 || ID == 127 || ID == 179 || ID == 231 || ID == 283)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"6S.jpg");
                                else if(ID == 24 || ID == 76 || ID == 128 || ID == 180 || ID == 232 || ID == 284)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"7D.jpg");
                                else if(ID == 25 || ID == 77 || ID == 129 || ID == 181 || ID == 233 || ID == 285)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"7H.jpg");
                                else if(ID == 26 || ID == 78 || ID == 130 || ID == 182 || ID == 234 || ID == 286)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"7C.jpg");
                                else if(ID == 27 || ID == 79 || ID == 131 || ID == 183 || ID == 235 || ID == 287)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"7S.jpg");
                                else if(ID == 28 || ID == 80 || ID == 132 || ID == 184 || ID == 236 || ID == 288)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"8D.jpg");
                                else if(ID == 29 || ID == 81 || ID == 133 || ID == 185 || ID == 237 || ID == 289)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"8H.jpg");
                                else if(ID == 30 || ID == 82 || ID == 134 || ID == 186 || ID == 238 || ID == 290)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"8C.jpg");
                                else if(ID == 31 || ID == 83 || ID == 135 || ID == 187 || ID == 239 || ID == 291)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"8S.jpg");
                                else if(ID == 32 || ID == 84 || ID == 136 || ID == 188 || ID == 240 || ID == 292)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"9D.jpg");
                                else if(ID == 33 || ID == 85 || ID == 137 || ID == 189 || ID == 241 || ID == 293)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"9H.jpg");
                                else if(ID == 34 || ID == 86 || ID == 138 || ID == 190 || ID == 242 || ID == 294)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"9C.jpg");
                                else if(ID == 35 || ID == 87 || ID == 139 || ID == 191 || ID == 243 || ID == 295)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"9S.jpg");
                                else if(ID == 36 || ID == 88 || ID == 140 || ID == 192 || ID == 244 || ID == 296)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"10D.jpg");
                                else if(ID == 37 || ID == 89 || ID == 141 || ID == 193 || ID == 245 || ID == 297)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"10H.jpg");
                                else if(ID == 38 || ID == 90 || ID == 142 || ID == 194 || ID == 246 || ID == 298)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"10C.jpg");
                                else if(ID == 39 || ID == 91 || ID == 143 || ID == 195 || ID == 247 || ID == 299)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"10S.jpg");
                                else if(ID == 40 || ID == 92 || ID == 144 || ID == 196 || ID == 248 || ID == 300)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"JD.jpg");
                                else if(ID == 41 || ID == 93 || ID == 145 || ID == 197 || ID == 249 || ID == 301)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"JH.jpg");
                                else if(ID == 42 || ID == 94 || ID == 146 || ID == 198 || ID == 250 || ID == 302)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"JC.jpg");
                                else if(ID == 43 || ID == 95 || ID == 147 || ID == 199 || ID == 251 || ID == 303)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"JS.jpg");
                                else if(ID == 44 || ID == 96 || ID == 148 || ID == 200 || ID == 252 || ID == 304)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"QD.jpg");
                                else if(ID == 45 || ID == 97 || ID == 149 || ID == 201 || ID == 253 || ID == 305)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"QH.jpg");
                                else if(ID == 46 || ID == 98 || ID == 150 || ID == 202 || ID == 254 || ID == 306)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"QC.jpg");
                                else if(ID == 47 || ID == 99 || ID == 151 || ID == 203 || ID == 255 || ID == 307)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"QS.jpg");
                                else if(ID == 48 || ID == 100 || ID == 152 || ID == 204 || ID == 256 || ID == 308)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"KD.jpg");
                                else if(ID == 49 || ID == 101 || ID == 153 || ID == 205 || ID == 257 || ID == 309)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"KH.jpg");
                                else if(ID == 50 || ID == 102 || ID == 154 || ID == 206 || ID == 258 || ID == 310)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"KC.jpg");
                                else if(ID == 51 || ID == 103 || ID == 155 || ID == 207 || ID == 259 || ID == 311)
                                    gtk_image_set_from_file(GTK_IMAGE(P6),"KS.jpg");
                            }
                        }
                    }
                    else
                        gtk_label_set_text (GTK_LABEL(playerCards),"waiting");
                    //Updates the dealers card images, again, sorry it's long.
                    if (read_msg_.gs.dealer_cards_valid){
                        gtk_label_set_text (GTK_LABEL(dealerCards),d.c_str() );
                        for(int i = 0; i < 6; i++){
                            int ID = read_msg_.gs.dealer_cards[i];
                            if(i == 0){
                                if(ID == 320)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"No_Card.png");
                                else if(ID == 0 || ID == 52 || ID == 104 || ID == 156 || ID == 208 || ID == 260)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"AD.jpg");
                                else if(ID == 1 || ID == 53 || ID == 105 || ID == 157 || ID == 209 || ID == 261)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"AH.jpg");
                                else if(ID == 2 || ID == 54 || ID == 106 || ID == 158 || ID == 210 || ID == 262)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"AC.jpg");
                                else if(ID == 3 || ID == 55 || ID == 107 || ID == 159 || ID == 211 || ID == 263)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"AS.jpg");
                                else if(ID == 4 || ID == 56 || ID == 108 || ID == 160 || ID == 212 || ID == 264)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"2D.jpg");
                                else if(ID == 5 || ID == 57 || ID == 109 || ID == 161 || ID == 213 || ID == 265)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"2H.jpg");
                                else if(ID == 6 || ID == 58 || ID == 110 || ID == 162 || ID == 214 || ID == 266)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"2C.jpg");
                                else if(ID == 7 || ID == 59 || ID == 111 || ID == 163 || ID == 215 || ID == 267)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"2S.jpg");
                                else if(ID == 8 || ID == 60 || ID == 112 || ID == 164 || ID == 216 || ID == 268)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"3D.jpg");
                                else if(ID == 9 || ID == 61 || ID == 113 || ID == 165 || ID == 217 || ID == 269)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"3H.jpg");
                                else if(ID == 10 || ID == 62 || ID == 114 || ID == 166 || ID == 218 || ID == 270)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"3C.jpg");
                                else if(ID == 11 || ID == 63 || ID == 115 || ID == 167 || ID == 219 || ID == 271)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"3S.jpg");
                                else if(ID == 12 || ID == 64 || ID == 116 || ID == 168 || ID == 220 || ID == 272)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"4D.jpg");
                                else if(ID == 13 || ID == 65 || ID == 117 || ID == 169 || ID == 221 || ID == 273)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"4H.jpg");
                                else if(ID == 14 || ID == 66 || ID == 118 || ID == 170 || ID == 222 || ID == 274)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"4C.jpg");
                                else if(ID == 15 || ID == 67 || ID == 119 || ID == 171 || ID == 223 || ID == 275)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"4S.jpg");
                                else if(ID == 16 || ID == 68 || ID == 120 || ID == 172 || ID == 224 || ID == 276)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"5D.jpg");
                                else if(ID == 17 || ID == 69 || ID == 121 || ID == 173 || ID == 225 || ID == 277)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"5H.jpg");
                                else if(ID == 18 || ID == 70 || ID == 122 || ID == 174 || ID == 226 || ID == 278)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"5C.jpg");
                                else if(ID == 19 || ID == 71 || ID == 123 || ID == 175 || ID == 227 || ID == 279)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"5S.jpg");
                                else if(ID == 20 || ID == 72 || ID == 124 || ID == 176 || ID == 228 || ID == 280)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"6D.jpg");
                                else if(ID == 21 || ID == 73 || ID == 125 || ID == 177 || ID == 229 || ID == 281)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"6H.jpg");
                                else if(ID == 22 || ID == 74 || ID == 126 || ID == 178 || ID == 230 || ID == 282)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"6C.jpg");
                                else if(ID == 23 || ID == 75 || ID == 127 || ID == 179 || ID == 231 || ID == 283)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"6S.jpg");
                                else if(ID == 24 || ID == 76 || ID == 128 || ID == 180 || ID == 232 || ID == 284)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"7D.jpg");
                                else if(ID == 25 || ID == 77 || ID == 129 || ID == 181 || ID == 233 || ID == 285)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"7H.jpg");
                                else if(ID == 26 || ID == 78 || ID == 130 || ID == 182 || ID == 234 || ID == 286)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"7C.jpg");
                                else if(ID == 27 || ID == 79 || ID == 131 || ID == 183 || ID == 235 || ID == 287)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"7S.jpg");
                                else if(ID == 28 || ID == 80 || ID == 132 || ID == 184 || ID == 236 || ID == 288)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"8D.jpg");
                                else if(ID == 29 || ID == 81 || ID == 133 || ID == 185 || ID == 237 || ID == 289)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"8H.jpg");
                                else if(ID == 30 || ID == 82 || ID == 134 || ID == 186 || ID == 238 || ID == 290)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"8C.jpg");
                                else if(ID == 31 || ID == 83 || ID == 135 || ID == 187 || ID == 239 || ID == 291)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"8S.jpg");
                                else if(ID == 32 || ID == 84 || ID == 136 || ID == 188 || ID == 240 || ID == 292)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"9D.jpg");
                                else if(ID == 33 || ID == 85 || ID == 137 || ID == 189 || ID == 241 || ID == 293)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"9H.jpg");
                                else if(ID == 34 || ID == 86 || ID == 138 || ID == 190 || ID == 242 || ID == 294)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"9C.jpg");
                                else if(ID == 35 || ID == 87 || ID == 139 || ID == 191 || ID == 243 || ID == 295)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"9S.jpg");
                                else if(ID == 36 || ID == 88 || ID == 140 || ID == 192 || ID == 244 || ID == 296)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"10D.jpg");
                                else if(ID == 37 || ID == 89 || ID == 141 || ID == 193 || ID == 245 || ID == 297)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"10H.jpg");
                                else if(ID == 38 || ID == 90 || ID == 142 || ID == 194 || ID == 246 || ID == 298)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"10C.jpg");
                                else if(ID == 39 || ID == 91 || ID == 143 || ID == 195 || ID == 247 || ID == 299)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"10S.jpg");
                                else if(ID == 40 || ID == 92 || ID == 144 || ID == 196 || ID == 248 || ID == 300)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"JD.jpg");
                                else if(ID == 41 || ID == 93 || ID == 145 || ID == 197 || ID == 249 || ID == 301)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"JH.jpg");
                                else if(ID == 42 || ID == 94 || ID == 146 || ID == 198 || ID == 250 || ID == 302)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"JC.jpg");
                                else if(ID == 43 || ID == 95 || ID == 147 || ID == 199 || ID == 251 || ID == 303)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"JS.jpg");
                                else if(ID == 44 || ID == 96 || ID == 148 || ID == 200 || ID == 252 || ID == 304)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"QD.jpg");
                                else if(ID == 45 || ID == 97 || ID == 149 || ID == 201 || ID == 253 || ID == 305)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"QH.jpg");
                                else if(ID == 46 || ID == 98 || ID == 150 || ID == 202 || ID == 254 || ID == 306)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"QC.jpg");
                                else if(ID == 47 || ID == 99 || ID == 151 || ID == 203 || ID == 255 || ID == 307)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"QS.jpg");
                                else if(ID == 48 || ID == 100 || ID == 152 || ID == 204 || ID == 256 || ID == 308)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"KD.jpg");
                                else if(ID == 49 || ID == 101 || ID == 153 || ID == 205 || ID == 257 || ID == 309)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"KH.jpg");
                                else if(ID == 50 || ID == 102 || ID == 154 || ID == 206 || ID == 258 || ID == 310)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"KC.jpg");
                                else if(ID == 51 || ID == 103 || ID == 155 || ID == 207 || ID == 259 || ID == 311)
                                    gtk_image_set_from_file(GTK_IMAGE(D1),"KS.jpg");
                            }
                            else if(i == 1){
                                if(ID == 320)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"No_Card.png");
                                else if(ID == 0 || ID == 52 || ID == 104 || ID == 156 || ID == 208 || ID == 260)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"AD.jpg");
                                else if(ID == 1 || ID == 53 || ID == 105 || ID == 157 || ID == 209 || ID == 261)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"AH.jpg");
                                else if(ID == 2 || ID == 54 || ID == 106 || ID == 158 || ID == 210 || ID == 262)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"AC.jpg");
                                else if(ID == 3 || ID == 55 || ID == 107 || ID == 159 || ID == 211 || ID == 263)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"AS.jpg");
                                else if(ID == 4 || ID == 56 || ID == 108 || ID == 160 || ID == 212 || ID == 264)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"2D.jpg");
                                else if(ID == 5 || ID == 57 || ID == 109 || ID == 161 || ID == 213 || ID == 265)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"2H.jpg");
                                else if(ID == 6 || ID == 58 || ID == 110 || ID == 162 || ID == 214 || ID == 266)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"2C.jpg");
                                else if(ID == 7 || ID == 59 || ID == 111 || ID == 163 || ID == 215 || ID == 267)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"2S.jpg");
                                else if(ID == 8 || ID == 60 || ID == 112 || ID == 164 || ID == 216 || ID == 268)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"3D.jpg");
                                else if(ID == 9 || ID == 61 || ID == 113 || ID == 165 || ID == 217 || ID == 269)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"3H.jpg");
                                else if(ID == 10 || ID == 62 || ID == 114 || ID == 166 || ID == 218 || ID == 270)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"3C.jpg");
                                else if(ID == 11 || ID == 63 || ID == 115 || ID == 167 || ID == 219 || ID == 271)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"3S.jpg");
                                else if(ID == 12 || ID == 64 || ID == 116 || ID == 168 || ID == 220 || ID == 272)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"4D.jpg");
                                else if(ID == 13 || ID == 65 || ID == 117 || ID == 169 || ID == 221 || ID == 273)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"4H.jpg");
                                else if(ID == 14 || ID == 66 || ID == 118 || ID == 170 || ID == 222 || ID == 274)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"4C.jpg");
                                else if(ID == 15 || ID == 67 || ID == 119 || ID == 171 || ID == 223 || ID == 275)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"4S.jpg");
                                else if(ID == 16 || ID == 68 || ID == 120 || ID == 172 || ID == 224 || ID == 276)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"5D.jpg");
                                else if(ID == 17 || ID == 69 || ID == 121 || ID == 173 || ID == 225 || ID == 277)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"5H.jpg");
                                else if(ID == 18 || ID == 70 || ID == 122 || ID == 174 || ID == 226 || ID == 278)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"5C.jpg");
                                else if(ID == 19 || ID == 71 || ID == 123 || ID == 175 || ID == 227 || ID == 279)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"5S.jpg");
                                else if(ID == 20 || ID == 72 || ID == 124 || ID == 176 || ID == 228 || ID == 280)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"6D.jpg");
                                else if(ID == 21 || ID == 73 || ID == 125 || ID == 177 || ID == 229 || ID == 281)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"6H.jpg");
                                else if(ID == 22 || ID == 74 || ID == 126 || ID == 178 || ID == 230 || ID == 282)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"6C.jpg");
                                else if(ID == 23 || ID == 75 || ID == 127 || ID == 179 || ID == 231 || ID == 283)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"6S.jpg");
                                else if(ID == 24 || ID == 76 || ID == 128 || ID == 180 || ID == 232 || ID == 284)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"7D.jpg");
                                else if(ID == 25 || ID == 77 || ID == 129 || ID == 181 || ID == 233 || ID == 285)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"7H.jpg");
                                else if(ID == 26 || ID == 78 || ID == 130 || ID == 182 || ID == 234 || ID == 286)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"7C.jpg");
                                else if(ID == 27 || ID == 79 || ID == 131 || ID == 183 || ID == 235 || ID == 287)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"7S.jpg");
                                else if(ID == 28 || ID == 80 || ID == 132 || ID == 184 || ID == 236 || ID == 288)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"8D.jpg");
                                else if(ID == 29 || ID == 81 || ID == 133 || ID == 185 || ID == 237 || ID == 289)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"8H.jpg");
                                else if(ID == 30 || ID == 82 || ID == 134 || ID == 186 || ID == 238 || ID == 290)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"8C.jpg");
                                else if(ID == 31 || ID == 83 || ID == 135 || ID == 187 || ID == 239 || ID == 291)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"8S.jpg");
                                else if(ID == 32 || ID == 84 || ID == 136 || ID == 188 || ID == 240 || ID == 292)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"9D.jpg");
                                else if(ID == 33 || ID == 85 || ID == 137 || ID == 189 || ID == 241 || ID == 293)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"9H.jpg");
                                else if(ID == 34 || ID == 86 || ID == 138 || ID == 190 || ID == 242 || ID == 294)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"9C.jpg");
                                else if(ID == 35 || ID == 87 || ID == 139 || ID == 191 || ID == 243 || ID == 295)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"9S.jpg");
                                else if(ID == 36 || ID == 88 || ID == 140 || ID == 192 || ID == 244 || ID == 296)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"10D.jpg");
                                else if(ID == 37 || ID == 89 || ID == 141 || ID == 193 || ID == 245 || ID == 297)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"10H.jpg");
                                else if(ID == 38 || ID == 90 || ID == 142 || ID == 194 || ID == 246 || ID == 298)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"10C.jpg");
                                else if(ID == 39 || ID == 91 || ID == 143 || ID == 195 || ID == 247 || ID == 299)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"10S.jpg");
                                else if(ID == 40 || ID == 92 || ID == 144 || ID == 196 || ID == 248 || ID == 300)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"JD.jpg");
                                else if(ID == 41 || ID == 93 || ID == 145 || ID == 197 || ID == 249 || ID == 301)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"JH.jpg");
                                else if(ID == 42 || ID == 94 || ID == 146 || ID == 198 || ID == 250 || ID == 302)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"JC.jpg");
                                else if(ID == 43 || ID == 95 || ID == 147 || ID == 199 || ID == 251 || ID == 303)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"JS.jpg");
                                else if(ID == 44 || ID == 96 || ID == 148 || ID == 200 || ID == 252 || ID == 304)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"QD.jpg");
                                else if(ID == 45 || ID == 97 || ID == 149 || ID == 201 || ID == 253 || ID == 305)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"QH.jpg");
                                else if(ID == 46 || ID == 98 || ID == 150 || ID == 202 || ID == 254 || ID == 306)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"QC.jpg");
                                else if(ID == 47 || ID == 99 || ID == 151 || ID == 203 || ID == 255 || ID == 307)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"QS.jpg");
                                else if(ID == 48 || ID == 100 || ID == 152 || ID == 204 || ID == 256 || ID == 308)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"KD.jpg");
                                else if(ID == 49 || ID == 101 || ID == 153 || ID == 205 || ID == 257 || ID == 309)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"KH.jpg");
                                else if(ID == 50 || ID == 102 || ID == 154 || ID == 206 || ID == 258 || ID == 310)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"KC.jpg");
                                else if(ID == 51 || ID == 103 || ID == 155 || ID == 207 || ID == 259 || ID == 311)
                                    gtk_image_set_from_file(GTK_IMAGE(D2),"KS.jpg");
                            }
                            else if(i == 2){
                                if(ID == 320)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"No_Card.png");
                                else if(ID == 0 || ID == 52 || ID == 104 || ID == 156 || ID == 208 || ID == 260)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"AD.jpg");
                                else if(ID == 1 || ID == 53 || ID == 105 || ID == 157 || ID == 209 || ID == 261)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"AH.jpg");
                                else if(ID == 2 || ID == 54 || ID == 106 || ID == 158 || ID == 210 || ID == 262)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"AC.jpg");
                                else if(ID == 3 || ID == 55 || ID == 107 || ID == 159 || ID == 211 || ID == 263)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"AS.jpg");
                                else if(ID == 4 || ID == 56 || ID == 108 || ID == 160 || ID == 212 || ID == 264)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"2D.jpg");
                                else if(ID == 5 || ID == 57 || ID == 109 || ID == 161 || ID == 213 || ID == 265)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"2H.jpg");
                                else if(ID == 6 || ID == 58 || ID == 110 || ID == 162 || ID == 214 || ID == 266)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"2C.jpg");
                                else if(ID == 7 || ID == 59 || ID == 111 || ID == 163 || ID == 215 || ID == 267)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"2S.jpg");
                                else if(ID == 8 || ID == 60 || ID == 112 || ID == 164 || ID == 216 || ID == 268)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"3D.jpg");
                                else if(ID == 9 || ID == 61 || ID == 113 || ID == 165 || ID == 217 || ID == 269)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"3H.jpg");
                                else if(ID == 10 || ID == 62 || ID == 114 || ID == 166 || ID == 218 || ID == 270)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"3C.jpg");
                                else if(ID == 11 || ID == 63 || ID == 115 || ID == 167 || ID == 219 || ID == 271)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"3S.jpg");
                                else if(ID == 12 || ID == 64 || ID == 116 || ID == 168 || ID == 220 || ID == 272)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"4D.jpg");
                                else if(ID == 13 || ID == 65 || ID == 117 || ID == 169 || ID == 221 || ID == 273)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"4H.jpg");
                                else if(ID == 14 || ID == 66 || ID == 118 || ID == 170 || ID == 222 || ID == 274)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"4C.jpg");
                                else if(ID == 15 || ID == 67 || ID == 119 || ID == 171 || ID == 223 || ID == 275)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"4S.jpg");
                                else if(ID == 16 || ID == 68 || ID == 120 || ID == 172 || ID == 224 || ID == 276)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"5D.jpg");
                                else if(ID == 17 || ID == 69 || ID == 121 || ID == 173 || ID == 225 || ID == 277)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"5H.jpg");
                                else if(ID == 18 || ID == 70 || ID == 122 || ID == 174 || ID == 226 || ID == 278)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"5C.jpg");
                                else if(ID == 19 || ID == 71 || ID == 123 || ID == 175 || ID == 227 || ID == 279)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"5S.jpg");
                                else if(ID == 20 || ID == 72 || ID == 124 || ID == 176 || ID == 228 || ID == 280)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"6D.jpg");
                                else if(ID == 21 || ID == 73 || ID == 125 || ID == 177 || ID == 229 || ID == 281)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"6H.jpg");
                                else if(ID == 22 || ID == 74 || ID == 126 || ID == 178 || ID == 230 || ID == 282)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"6C.jpg");
                                else if(ID == 23 || ID == 75 || ID == 127 || ID == 179 || ID == 231 || ID == 283)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"6S.jpg");
                                else if(ID == 24 || ID == 76 || ID == 128 || ID == 180 || ID == 232 || ID == 284)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"7D.jpg");
                                else if(ID == 25 || ID == 77 || ID == 129 || ID == 181 || ID == 233 || ID == 285)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"7H.jpg");
                                else if(ID == 26 || ID == 78 || ID == 130 || ID == 182 || ID == 234 || ID == 286)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"7C.jpg");
                                else if(ID == 27 || ID == 79 || ID == 131 || ID == 183 || ID == 235 || ID == 287)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"7S.jpg");
                                else if(ID == 28 || ID == 80 || ID == 132 || ID == 184 || ID == 236 || ID == 288)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"8D.jpg");
                                else if(ID == 29 || ID == 81 || ID == 133 || ID == 185 || ID == 237 || ID == 289)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"8H.jpg");
                                else if(ID == 30 || ID == 82 || ID == 134 || ID == 186 || ID == 238 || ID == 290)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"8C.jpg");
                                else if(ID == 31 || ID == 83 || ID == 135 || ID == 187 || ID == 239 || ID == 291)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"8S.jpg");
                                else if(ID == 32 || ID == 84 || ID == 136 || ID == 188 || ID == 240 || ID == 292)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"9D.jpg");
                                else if(ID == 33 || ID == 85 || ID == 137 || ID == 189 || ID == 241 || ID == 293)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"9H.jpg");
                                else if(ID == 34 || ID == 86 || ID == 138 || ID == 190 || ID == 242 || ID == 294)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"9C.jpg");
                                else if(ID == 35 || ID == 87 || ID == 139 || ID == 191 || ID == 243 || ID == 295)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"9S.jpg");
                                else if(ID == 36 || ID == 88 || ID == 140 || ID == 192 || ID == 244 || ID == 296)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"10D.jpg");
                                else if(ID == 37 || ID == 89 || ID == 141 || ID == 193 || ID == 245 || ID == 297)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"10H.jpg");
                                else if(ID == 38 || ID == 90 || ID == 142 || ID == 194 || ID == 246 || ID == 298)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"10C.jpg");
                                else if(ID == 39 || ID == 91 || ID == 143 || ID == 195 || ID == 247 || ID == 299)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"10S.jpg");
                                else if(ID == 40 || ID == 92 || ID == 144 || ID == 196 || ID == 248 || ID == 300)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"JD.jpg");
                                else if(ID == 41 || ID == 93 || ID == 145 || ID == 197 || ID == 249 || ID == 301)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"JH.jpg");
                                else if(ID == 42 || ID == 94 || ID == 146 || ID == 198 || ID == 250 || ID == 302)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"JC.jpg");
                                else if(ID == 43 || ID == 95 || ID == 147 || ID == 199 || ID == 251 || ID == 303)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"JS.jpg");
                                else if(ID == 44 || ID == 96 || ID == 148 || ID == 200 || ID == 252 || ID == 304)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"QD.jpg");
                                else if(ID == 45 || ID == 97 || ID == 149 || ID == 201 || ID == 253 || ID == 305)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"QH.jpg");
                                else if(ID == 46 || ID == 98 || ID == 150 || ID == 202 || ID == 254 || ID == 306)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"QC.jpg");
                                else if(ID == 47 || ID == 99 || ID == 151 || ID == 203 || ID == 255 || ID == 307)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"QS.jpg");
                                else if(ID == 48 || ID == 100 || ID == 152 || ID == 204 || ID == 256 || ID == 308)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"KD.jpg");
                                else if(ID == 49 || ID == 101 || ID == 153 || ID == 205 || ID == 257 || ID == 309)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"KH.jpg");
                                else if(ID == 50 || ID == 102 || ID == 154 || ID == 206 || ID == 258 || ID == 310)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"KC.jpg");
                                else if(ID == 51 || ID == 103 || ID == 155 || ID == 207 || ID == 259 || ID == 311)
                                    gtk_image_set_from_file(GTK_IMAGE(D3),"KS.jpg");
                            }
                            else if(i == 3){
                                if(ID == 320)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"No_Card.png");
                                else if(ID == 0 || ID == 52 || ID == 104 || ID == 156 || ID == 208 || ID == 260)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"AD.jpg");
                                else if(ID == 1 || ID == 53 || ID == 105 || ID == 157 || ID == 209 || ID == 261)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"AH.jpg");
                                else if(ID == 2 || ID == 54 || ID == 106 || ID == 158 || ID == 210 || ID == 262)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"AC.jpg");
                                else if(ID == 3 || ID == 55 || ID == 107 || ID == 159 || ID == 211 || ID == 263)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"AS.jpg");
                                else if(ID == 4 || ID == 56 || ID == 108 || ID == 160 || ID == 212 || ID == 264)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"2D.jpg");
                                else if(ID == 5 || ID == 57 || ID == 109 || ID == 161 || ID == 213 || ID == 265)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"2H.jpg");
                                else if(ID == 6 || ID == 58 || ID == 110 || ID == 162 || ID == 214 || ID == 266)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"2C.jpg");
                                else if(ID == 7 || ID == 59 || ID == 111 || ID == 163 || ID == 215 || ID == 267)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"2S.jpg");
                                else if(ID == 8 || ID == 60 || ID == 112 || ID == 164 || ID == 216 || ID == 268)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"3D.jpg");
                                else if(ID == 9 || ID == 61 || ID == 113 || ID == 165 || ID == 217 || ID == 269)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"3H.jpg");
                                else if(ID == 10 || ID == 62 || ID == 114 || ID == 166 || ID == 218 || ID == 270)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"3C.jpg");
                                else if(ID == 11 || ID == 63 || ID == 115 || ID == 167 || ID == 219 || ID == 271)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"3S.jpg");
                                else if(ID == 12 || ID == 64 || ID == 116 || ID == 168 || ID == 220 || ID == 272)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"4D.jpg");
                                else if(ID == 13 || ID == 65 || ID == 117 || ID == 169 || ID == 221 || ID == 273)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"4H.jpg");
                                else if(ID == 14 || ID == 66 || ID == 118 || ID == 170 || ID == 222 || ID == 274)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"4C.jpg");
                                else if(ID == 15 || ID == 67 || ID == 119 || ID == 171 || ID == 223 || ID == 275)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"4S.jpg");
                                else if(ID == 16 || ID == 68 || ID == 120 || ID == 172 || ID == 224 || ID == 276)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"5D.jpg");
                                else if(ID == 17 || ID == 69 || ID == 121 || ID == 173 || ID == 225 || ID == 277)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"5H.jpg");
                                else if(ID == 18 || ID == 70 || ID == 122 || ID == 174 || ID == 226 || ID == 278)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"5C.jpg");
                                else if(ID == 19 || ID == 71 || ID == 123 || ID == 175 || ID == 227 || ID == 279)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"5S.jpg");
                                else if(ID == 20 || ID == 72 || ID == 124 || ID == 176 || ID == 228 || ID == 280)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"6D.jpg");
                                else if(ID == 21 || ID == 73 || ID == 125 || ID == 177 || ID == 229 || ID == 281)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"6H.jpg");
                                else if(ID == 22 || ID == 74 || ID == 126 || ID == 178 || ID == 230 || ID == 282)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"6C.jpg");
                                else if(ID == 23 || ID == 75 || ID == 127 || ID == 179 || ID == 231 || ID == 283)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"6S.jpg");
                                else if(ID == 24 || ID == 76 || ID == 128 || ID == 180 || ID == 232 || ID == 284)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"7D.jpg");
                                else if(ID == 25 || ID == 77 || ID == 129 || ID == 181 || ID == 233 || ID == 285)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"7H.jpg");
                                else if(ID == 26 || ID == 78 || ID == 130 || ID == 182 || ID == 234 || ID == 286)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"7C.jpg");
                                else if(ID == 27 || ID == 79 || ID == 131 || ID == 183 || ID == 235 || ID == 287)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"7S.jpg");
                                else if(ID == 28 || ID == 80 || ID == 132 || ID == 184 || ID == 236 || ID == 288)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"8D.jpg");
                                else if(ID == 29 || ID == 81 || ID == 133 || ID == 185 || ID == 237 || ID == 289)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"8H.jpg");
                                else if(ID == 30 || ID == 82 || ID == 134 || ID == 186 || ID == 238 || ID == 290)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"8C.jpg");
                                else if(ID == 31 || ID == 83 || ID == 135 || ID == 187 || ID == 239 || ID == 291)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"8S.jpg");
                                else if(ID == 32 || ID == 84 || ID == 136 || ID == 188 || ID == 240 || ID == 292)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"9D.jpg");
                                else if(ID == 33 || ID == 85 || ID == 137 || ID == 189 || ID == 241 || ID == 293)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"9H.jpg");
                                else if(ID == 34 || ID == 86 || ID == 138 || ID == 190 || ID == 242 || ID == 294)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"9C.jpg");
                                else if(ID == 35 || ID == 87 || ID == 139 || ID == 191 || ID == 243 || ID == 295)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"9S.jpg");
                                else if(ID == 36 || ID == 88 || ID == 140 || ID == 192 || ID == 244 || ID == 296)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"10D.jpg");
                                else if(ID == 37 || ID == 89 || ID == 141 || ID == 193 || ID == 245 || ID == 297)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"10H.jpg");
                                else if(ID == 38 || ID == 90 || ID == 142 || ID == 194 || ID == 246 || ID == 298)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"10C.jpg");
                                else if(ID == 39 || ID == 91 || ID == 143 || ID == 195 || ID == 247 || ID == 299)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"10S.jpg");
                                else if(ID == 40 || ID == 92 || ID == 144 || ID == 196 || ID == 248 || ID == 300)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"JD.jpg");
                                else if(ID == 41 || ID == 93 || ID == 145 || ID == 197 || ID == 249 || ID == 301)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"JH.jpg");
                                else if(ID == 42 || ID == 94 || ID == 146 || ID == 198 || ID == 250 || ID == 302)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"JC.jpg");
                                else if(ID == 43 || ID == 95 || ID == 147 || ID == 199 || ID == 251 || ID == 303)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"JS.jpg");
                                else if(ID == 44 || ID == 96 || ID == 148 || ID == 200 || ID == 252 || ID == 304)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"QD.jpg");
                                else if(ID == 45 || ID == 97 || ID == 149 || ID == 201 || ID == 253 || ID == 305)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"QH.jpg");
                                else if(ID == 46 || ID == 98 || ID == 150 || ID == 202 || ID == 254 || ID == 306)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"QC.jpg");
                                else if(ID == 47 || ID == 99 || ID == 151 || ID == 203 || ID == 255 || ID == 307)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"QS.jpg");
                                else if(ID == 48 || ID == 100 || ID == 152 || ID == 204 || ID == 256 || ID == 308)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"KD.jpg");
                                else if(ID == 49 || ID == 101 || ID == 153 || ID == 205 || ID == 257 || ID == 309)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"KH.jpg");
                                else if(ID == 50 || ID == 102 || ID == 154 || ID == 206 || ID == 258 || ID == 310)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"KC.jpg");
                                else if(ID == 51 || ID == 103 || ID == 155 || ID == 207 || ID == 259 || ID == 311)
                                    gtk_image_set_from_file(GTK_IMAGE(D4),"KS.jpg");
                            }
                            else if(i == 4){
                                if(ID == 320)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"No_Card.png");
                                else if(ID == 0 || ID == 52 || ID == 104 || ID == 156 || ID == 208 || ID == 260)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"AD.jpg");
                                else if(ID == 1 || ID == 53 || ID == 105 || ID == 157 || ID == 209 || ID == 261)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"AH.jpg");
                                else if(ID == 2 || ID == 54 || ID == 106 || ID == 158 || ID == 210 || ID == 262)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"AC.jpg");
                                else if(ID == 3 || ID == 55 || ID == 107 || ID == 159 || ID == 211 || ID == 263)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"AS.jpg");
                                else if(ID == 4 || ID == 56 || ID == 108 || ID == 160 || ID == 212 || ID == 264)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"2D.jpg");
                                else if(ID == 5 || ID == 57 || ID == 109 || ID == 161 || ID == 213 || ID == 265)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"2H.jpg");
                                else if(ID == 6 || ID == 58 || ID == 110 || ID == 162 || ID == 214 || ID == 266)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"2C.jpg");
                                else if(ID == 7 || ID == 59 || ID == 111 || ID == 163 || ID == 215 || ID == 267)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"2S.jpg");
                                else if(ID == 8 || ID == 60 || ID == 112 || ID == 164 || ID == 216 || ID == 268)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"3D.jpg");
                                else if(ID == 9 || ID == 61 || ID == 113 || ID == 165 || ID == 217 || ID == 269)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"3H.jpg");
                                else if(ID == 10 || ID == 62 || ID == 114 || ID == 166 || ID == 218 || ID == 270)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"3C.jpg");
                                else if(ID == 11 || ID == 63 || ID == 115 || ID == 167 || ID == 219 || ID == 271)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"3S.jpg");
                                else if(ID == 12 || ID == 64 || ID == 116 || ID == 168 || ID == 220 || ID == 272)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"4D.jpg");
                                else if(ID == 13 || ID == 65 || ID == 117 || ID == 169 || ID == 221 || ID == 273)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"4H.jpg");
                                else if(ID == 14 || ID == 66 || ID == 118 || ID == 170 || ID == 222 || ID == 274)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"4C.jpg");
                                else if(ID == 15 || ID == 67 || ID == 119 || ID == 171 || ID == 223 || ID == 275)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"4S.jpg");
                                else if(ID == 16 || ID == 68 || ID == 120 || ID == 172 || ID == 224 || ID == 276)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"5D.jpg");
                                else if(ID == 17 || ID == 69 || ID == 121 || ID == 173 || ID == 225 || ID == 277)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"5H.jpg");
                                else if(ID == 18 || ID == 70 || ID == 122 || ID == 174 || ID == 226 || ID == 278)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"5C.jpg");
                                else if(ID == 19 || ID == 71 || ID == 123 || ID == 175 || ID == 227 || ID == 279)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"5S.jpg");
                                else if(ID == 20 || ID == 72 || ID == 124 || ID == 176 || ID == 228 || ID == 280)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"6D.jpg");
                                else if(ID == 21 || ID == 73 || ID == 125 || ID == 177 || ID == 229 || ID == 281)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"6H.jpg");
                                else if(ID == 22 || ID == 74 || ID == 126 || ID == 178 || ID == 230 || ID == 282)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"6C.jpg");
                                else if(ID == 23 || ID == 75 || ID == 127 || ID == 179 || ID == 231 || ID == 283)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"6S.jpg");
                                else if(ID == 24 || ID == 76 || ID == 128 || ID == 180 || ID == 232 || ID == 284)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"7D.jpg");
                                else if(ID == 25 || ID == 77 || ID == 129 || ID == 181 || ID == 233 || ID == 285)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"7H.jpg");
                                else if(ID == 26 || ID == 78 || ID == 130 || ID == 182 || ID == 234 || ID == 286)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"7C.jpg");
                                else if(ID == 27 || ID == 79 || ID == 131 || ID == 183 || ID == 235 || ID == 287)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"7S.jpg");
                                else if(ID == 28 || ID == 80 || ID == 132 || ID == 184 || ID == 236 || ID == 288)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"8D.jpg");
                                else if(ID == 29 || ID == 81 || ID == 133 || ID == 185 || ID == 237 || ID == 289)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"8H.jpg");
                                else if(ID == 30 || ID == 82 || ID == 134 || ID == 186 || ID == 238 || ID == 290)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"8C.jpg");
                                else if(ID == 31 || ID == 83 || ID == 135 || ID == 187 || ID == 239 || ID == 291)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"8S.jpg");
                                else if(ID == 32 || ID == 84 || ID == 136 || ID == 188 || ID == 240 || ID == 292)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"9D.jpg");
                                else if(ID == 33 || ID == 85 || ID == 137 || ID == 189 || ID == 241 || ID == 293)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"9H.jpg");
                                else if(ID == 34 || ID == 86 || ID == 138 || ID == 190 || ID == 242 || ID == 294)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"9C.jpg");
                                else if(ID == 35 || ID == 87 || ID == 139 || ID == 191 || ID == 243 || ID == 295)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"9S.jpg");
                                else if(ID == 36 || ID == 88 || ID == 140 || ID == 192 || ID == 244 || ID == 296)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"10D.jpg");
                                else if(ID == 37 || ID == 89 || ID == 141 || ID == 193 || ID == 245 || ID == 297)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"10H.jpg");
                                else if(ID == 38 || ID == 90 || ID == 142 || ID == 194 || ID == 246 || ID == 298)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"10C.jpg");
                                else if(ID == 39 || ID == 91 || ID == 143 || ID == 195 || ID == 247 || ID == 299)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"10S.jpg");
                                else if(ID == 40 || ID == 92 || ID == 144 || ID == 196 || ID == 248 || ID == 300)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"JD.jpg");
                                else if(ID == 41 || ID == 93 || ID == 145 || ID == 197 || ID == 249 || ID == 301)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"JH.jpg");
                                else if(ID == 42 || ID == 94 || ID == 146 || ID == 198 || ID == 250 || ID == 302)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"JC.jpg");
                                else if(ID == 43 || ID == 95 || ID == 147 || ID == 199 || ID == 251 || ID == 303)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"JS.jpg");
                                else if(ID == 44 || ID == 96 || ID == 148 || ID == 200 || ID == 252 || ID == 304)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"QD.jpg");
                                else if(ID == 45 || ID == 97 || ID == 149 || ID == 201 || ID == 253 || ID == 305)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"QH.jpg");
                                else if(ID == 46 || ID == 98 || ID == 150 || ID == 202 || ID == 254 || ID == 306)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"QC.jpg");
                                else if(ID == 47 || ID == 99 || ID == 151 || ID == 203 || ID == 255 || ID == 307)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"QS.jpg");
                                else if(ID == 48 || ID == 100 || ID == 152 || ID == 204 || ID == 256 || ID == 308)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"KD.jpg");
                                else if(ID == 49 || ID == 101 || ID == 153 || ID == 205 || ID == 257 || ID == 309)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"KH.jpg");
                                else if(ID == 50 || ID == 102 || ID == 154 || ID == 206 || ID == 258 || ID == 310)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"KC.jpg");
                                else if(ID == 51 || ID == 103 || ID == 155 || ID == 207 || ID == 259 || ID == 311)
                                    gtk_image_set_from_file(GTK_IMAGE(D5),"KS.jpg");
                            }
                            else{
                                if(ID == 320)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"No_Card.png");
                                else if(ID == 0 || ID == 52 || ID == 104 || ID == 156 || ID == 208 || ID == 260)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"AD.jpg");
                                else if(ID == 1 || ID == 53 || ID == 105 || ID == 157 || ID == 209 || ID == 261)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"AH.jpg");
                                else if(ID == 2 || ID == 54 || ID == 106 || ID == 158 || ID == 210 || ID == 262)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"AC.jpg");
                                else if(ID == 3 || ID == 55 || ID == 107 || ID == 159 || ID == 211 || ID == 263)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"AS.jpg");
                                else if(ID == 4 || ID == 56 || ID == 108 || ID == 160 || ID == 212 || ID == 264)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"2D.jpg");
                                else if(ID == 5 || ID == 57 || ID == 109 || ID == 161 || ID == 213 || ID == 265)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"2H.jpg");
                                else if(ID == 6 || ID == 58 || ID == 110 || ID == 162 || ID == 214 || ID == 266)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"2C.jpg");
                                else if(ID == 7 || ID == 59 || ID == 111 || ID == 163 || ID == 215 || ID == 267)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"2S.jpg");
                                else if(ID == 8 || ID == 60 || ID == 112 || ID == 164 || ID == 216 || ID == 268)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"3D.jpg");
                                else if(ID == 9 || ID == 61 || ID == 113 || ID == 165 || ID == 217 || ID == 269)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"3H.jpg");
                                else if(ID == 10 || ID == 62 || ID == 114 || ID == 166 || ID == 218 || ID == 270)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"3C.jpg");
                                else if(ID == 11 || ID == 63 || ID == 115 || ID == 167 || ID == 219 || ID == 271)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"3S.jpg");
                                else if(ID == 12 || ID == 64 || ID == 116 || ID == 168 || ID == 220 || ID == 272)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"4D.jpg");
                                else if(ID == 13 || ID == 65 || ID == 117 || ID == 169 || ID == 221 || ID == 273)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"4H.jpg");
                                else if(ID == 14 || ID == 66 || ID == 118 || ID == 170 || ID == 222 || ID == 274)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"4C.jpg");
                                else if(ID == 15 || ID == 67 || ID == 119 || ID == 171 || ID == 223 || ID == 275)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"4S.jpg");
                                else if(ID == 16 || ID == 68 || ID == 120 || ID == 172 || ID == 224 || ID == 276)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"5D.jpg");
                                else if(ID == 17 || ID == 69 || ID == 121 || ID == 173 || ID == 225 || ID == 277)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"5H.jpg");
                                else if(ID == 18 || ID == 70 || ID == 122 || ID == 174 || ID == 226 || ID == 278)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"5C.jpg");
                                else if(ID == 19 || ID == 71 || ID == 123 || ID == 175 || ID == 227 || ID == 279)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"5S.jpg");
                                else if(ID == 20 || ID == 72 || ID == 124 || ID == 176 || ID == 228 || ID == 280)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"6D.jpg");
                                else if(ID == 21 || ID == 73 || ID == 125 || ID == 177 || ID == 229 || ID == 281)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"6H.jpg");
                                else if(ID == 22 || ID == 74 || ID == 126 || ID == 178 || ID == 230 || ID == 282)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"6C.jpg");
                                else if(ID == 23 || ID == 75 || ID == 127 || ID == 179 || ID == 231 || ID == 283)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"6S.jpg");
                                else if(ID == 24 || ID == 76 || ID == 128 || ID == 180 || ID == 232 || ID == 284)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"7D.jpg");
                                else if(ID == 25 || ID == 77 || ID == 129 || ID == 181 || ID == 233 || ID == 285)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"7H.jpg");
                                else if(ID == 26 || ID == 78 || ID == 130 || ID == 182 || ID == 234 || ID == 286)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"7C.jpg");
                                else if(ID == 27 || ID == 79 || ID == 131 || ID == 183 || ID == 235 || ID == 287)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"7S.jpg");
                                else if(ID == 28 || ID == 80 || ID == 132 || ID == 184 || ID == 236 || ID == 288)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"8D.jpg");
                                else if(ID == 29 || ID == 81 || ID == 133 || ID == 185 || ID == 237 || ID == 289)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"8H.jpg");
                                else if(ID == 30 || ID == 82 || ID == 134 || ID == 186 || ID == 238 || ID == 290)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"8C.jpg");
                                else if(ID == 31 || ID == 83 || ID == 135 || ID == 187 || ID == 239 || ID == 291)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"8S.jpg");
                                else if(ID == 32 || ID == 84 || ID == 136 || ID == 188 || ID == 240 || ID == 292)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"9D.jpg");
                                else if(ID == 33 || ID == 85 || ID == 137 || ID == 189 || ID == 241 || ID == 293)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"9H.jpg");
                                else if(ID == 34 || ID == 86 || ID == 138 || ID == 190 || ID == 242 || ID == 294)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"9C.jpg");
                                else if(ID == 35 || ID == 87 || ID == 139 || ID == 191 || ID == 243 || ID == 295)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"9S.jpg");
                                else if(ID == 36 || ID == 88 || ID == 140 || ID == 192 || ID == 244 || ID == 296)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"10D.jpg");
                                else if(ID == 37 || ID == 89 || ID == 141 || ID == 193 || ID == 245 || ID == 297)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"10H.jpg");
                                else if(ID == 38 || ID == 90 || ID == 142 || ID == 194 || ID == 246 || ID == 298)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"10C.jpg");
                                else if(ID == 39 || ID == 91 || ID == 143 || ID == 195 || ID == 247 || ID == 299)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"10S.jpg");
                                else if(ID == 40 || ID == 92 || ID == 144 || ID == 196 || ID == 248 || ID == 300)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"JD.jpg");
                                else if(ID == 41 || ID == 93 || ID == 145 || ID == 197 || ID == 249 || ID == 301)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"JH.jpg");
                                else if(ID == 42 || ID == 94 || ID == 146 || ID == 198 || ID == 250 || ID == 302)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"JC.jpg");
                                else if(ID == 43 || ID == 95 || ID == 147 || ID == 199 || ID == 251 || ID == 303)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"JS.jpg");
                                else if(ID == 44 || ID == 96 || ID == 148 || ID == 200 || ID == 252 || ID == 304)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"QD.jpg");
                                else if(ID == 45 || ID == 97 || ID == 149 || ID == 201 || ID == 253 || ID == 305)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"QH.jpg");
                                else if(ID == 46 || ID == 98 || ID == 150 || ID == 202 || ID == 254 || ID == 306)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"QC.jpg");
                                else if(ID == 47 || ID == 99 || ID == 151 || ID == 203 || ID == 255 || ID == 307)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"QS.jpg");
                                else if(ID == 48 || ID == 100 || ID == 152 || ID == 204 || ID == 256 || ID == 308)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"KD.jpg");
                                else if(ID == 49 || ID == 101 || ID == 153 || ID == 205 || ID == 257 || ID == 309)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"KH.jpg");
                                else if(ID == 50 || ID == 102 || ID == 154 || ID == 206 || ID == 258 || ID == 310)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"KC.jpg");
                                else if(ID == 51 || ID == 103 || ID == 155 || ID == 207 || ID == 259 || ID == 311)
                                    gtk_image_set_from_file(GTK_IMAGE(D6),"KS.jpg");
                            }
                        }
                    }
                    else
                        gtk_label_set_text (GTK_LABEL(dealerCards),"waiting");
                    }
                gdk_threads_leave();
                cout.write(read_msg_.body(), read_msg_.body_length());
                cout << "\n";
                do_read_header();
                if(read_msg_.gs.player_quit == true){
                    sleep(10);
                    exit(0);
                }
            }
            else{
                socket_.close();
            }
            if(read_msg_.gs.current_number == 8){
                current = 7;
            }
        });
    }
    void do_write(){
        asio::async_write(socket_, asio::buffer(write_msgs_.front().data(), write_msgs_.front().length()), [this](error_code ec, size_t){
            if (!ec){
                write_msgs_.pop_front();
                if (!write_msgs_.empty()){
                    do_write();
                }
            }
            else{
                socket_.close();
            }
        });
    }
private:
    asio::io_context& io_context_;
    tcp::socket socket_;
    chat_message read_msg_;
    chat_message_queue write_msgs_;
};
chat_client *c;
static void destroy_event(GtkWidget *widget, gpointer data){
    gtk_main_quit();
}
static gboolean delete_event(GtkWidget *widget, GdkEvent *event, gpointer data){
    return FALSE; // must return false to trigger destroy event for window
}
//Quit button event.
static void quitCallback(GtkWidget *widget, GdkEventButton *event, gpointer callback_data){
    cerr << "quit button pressed: " << event->button << endl;
    chat_message msg;
    msg.body_length (0);
    msg.ca.quit = true;
    msg.ca.hit = false;
    msg.ca.stand = false;
    msg.ca.join = false;
    msg.ca.wager_valid = false;
    msg.ca.name_valid = false;
    msg.encode_header();
    assert ( c );
    c->write(msg);
}
//Hit button event.
static void hitCallback(GtkWidget *widget, GdkEventButton *event, gpointer callback_data){
    cerr << "hit button pressed: " << event->button << endl;
    chat_message msg;
    msg.body_length (0);
    msg.ca.quit = false;
    msg.ca.hit = true;
    msg.ca.stand = false;
    msg.ca.join = false;
    msg.ca.wager_valid = false;
    msg.ca.name_valid = false;
    msg.encode_header();
    assert ( c );
    c->write(msg);
}
//Stand button event.
static void standCallback(GtkWidget *widget, GdkEventButton *event, gpointer callback_data){
    cerr << "stand button pressed: " << event->button << endl;
    chat_message msg;
    msg.body_length (0);
    msg.ca.quit = false;
    msg.ca.hit = false;
    msg.ca.stand = true;
    msg.ca.join = false;
    msg.ca.wager_valid = false;
    msg.ca.name_valid = false;
    msg.encode_header();
    assert ( c );
    c->write(msg);
}
//Join button event.
static void joinCallback(GtkWidget *widget, GdkEventButton *event, gpointer callback_data){
    cerr << "join button pressed: " << event->button << endl;
    GtkWidget *callbackWidget = (GtkWidget*) callback_data;
    assert ( callbackWidget );
    GtkTextBuffer *toBuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (callbackWidget));
    GtkTextIter start;
    GtkTextIter end;
    gtk_text_buffer_get_start_iter ( toBuffer, &start );
    gtk_text_buffer_get_end_iter ( toBuffer, &end );
    gchar *text = gtk_text_buffer_get_text ( toBuffer, &start, &end, FALSE );
    if (text){
        chat_message msg;
        msg.body_length (0);
        msg.ca.quit = false;
        msg.ca.hit = false;
        msg.ca.stand = false;
        msg.ca.join = true;
        msg.ca.wager_valid = false;
        msg.ca.name_valid = true;
        if (strlen(text) < sizeof(msg.ca.name)){
            strcpy(msg.ca.name,text);
        }
        msg.encode_header();
        assert ( c );
        c->write(msg);
    }
}
//Wager button event.
static void wagerCallback(GtkWidget *widget, GdkEventButton *event, gpointer callback_data){
    cerr << "wager button pressed: " << event->button << endl;
    GtkWidget *callbackWidget = (GtkWidget*) callback_data;
    assert ( callbackWidget );
    GtkTextBuffer *toBuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (callbackWidget));
    GtkTextIter start;
    GtkTextIter end;
    gtk_text_buffer_get_start_iter ( toBuffer, &start );
    gtk_text_buffer_get_end_iter ( toBuffer, &end );
    gchar *text = gtk_text_buffer_get_text ( toBuffer, &start, &end, FALSE );
    if (text){
        chat_message msg;
        msg.body_length (0);
        msg.ca.quit = false;
        msg.ca.hit = false;
        msg.ca.stand = false;
        msg.ca.join = false;
        msg.ca.wager_valid = true;
        msg.ca.name_valid = false;
        msg.ca.wager = text[0];
        msg.encode_header();
        assert ( c );
        c->write(msg);
    }
}
//Main code, sets everything up.
int main(int argc, char *argv[]){
    gdk_threads_init();
    gdk_threads_enter();
    gtk_init( &argc, &argv);
    GtkWidget *window = NULL;
    GtkWidget *hitButton,*joinButton,*quitButton,*standButton,*wagerButton = NULL;
    GtkWidget *toView,*toWView  = NULL;  // text to the chat server
    GtkTextBuffer *toBuffer,*toWBuffer = NULL;
    toView = gtk_text_view_new ();
    toWView = gtk_text_view_new();
    fromView = gtk_label_new (NULL);
    dealerCards = gtk_label_new (NULL);
    playerCards = gtk_label_new (NULL);
    toBuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (toView));
    toWBuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (toWView));
    gtk_text_buffer_set_text (toBuffer, "Enter your name here", -1);
    gtk_label_set_text (GTK_LABEL(fromView), "");
    gtk_text_buffer_set_text (toWBuffer, "Enter your wager here", -1);
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Casino Royale");
    gtk_window_set_default_size(GTK_WINDOW(window), 2000, 2000);
    hitButton = gtk_button_new_with_label("Hit");
    standButton = gtk_button_new_with_label("Stand");
    quitButton = gtk_button_new_with_label("Quit");
    wagerButton = gtk_button_new_with_label("Wager");
    joinButton = gtk_button_new_with_label("Join");
    //Dealer card grid.
    dcardrow = gtk_grid_new();
    D1 = gtk_image_new_from_file("No_Card.png");
    gtk_grid_attach(GTK_GRID(dcardrow),D1,0,0,1,1);
    D2 = gtk_image_new_from_file("No_Card.png");
    gtk_grid_attach_next_to(GTK_GRID(dcardrow),D2,D1,GTK_POS_RIGHT,1,1);
    D3 = gtk_image_new_from_file("No_Card.png");
    gtk_grid_attach_next_to(GTK_GRID(dcardrow),D3,D2,GTK_POS_RIGHT,1,1);
    D4 = gtk_image_new_from_file("No_Card.png");
    gtk_grid_attach_next_to(GTK_GRID(dcardrow),D4,D3,GTK_POS_RIGHT,1,1);
    D5 = gtk_image_new_from_file("No_Card.png");
    gtk_grid_attach_next_to(GTK_GRID(dcardrow),D5,D4,GTK_POS_RIGHT,1,1);
    D6 = gtk_image_new_from_file("No_Card.png");
    gtk_grid_attach_next_to(GTK_GRID(dcardrow),D6,D5,GTK_POS_RIGHT,1,1);
    //Player card grid.
    pcardrow = gtk_grid_new();
    P1 = gtk_image_new_from_file("No_Card.png");
    gtk_grid_attach(GTK_GRID(pcardrow),P1,1,0,1,1);
    P2 = gtk_image_new_from_file("No_Card.png");
    gtk_grid_attach_next_to(GTK_GRID(pcardrow),P2,P1,GTK_POS_RIGHT,1,1);
    P3 = gtk_image_new_from_file("No_Card.png");
    gtk_grid_attach_next_to(GTK_GRID(pcardrow),P3,P2,GTK_POS_RIGHT,1,1);
    P4 = gtk_image_new_from_file("No_Card.png");
    gtk_grid_attach_next_to(GTK_GRID(pcardrow),P4,P3,GTK_POS_RIGHT,1,1);
    P5 = gtk_image_new_from_file("No_Card.png");
    gtk_grid_attach_next_to(GTK_GRID(pcardrow),P5,P4,GTK_POS_RIGHT,1,1);
    P6 = gtk_image_new_from_file("No_Card.png");
    gtk_grid_attach_next_to(GTK_GRID(pcardrow),P6,P5,GTK_POS_RIGHT,1,1);
    //Builds the GUI.
    GtkWidget *  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 1);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_box_pack_start(GTK_BOX(vbox),joinButton, FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(vbox),toView, FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(vbox),wagerButton, FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(vbox),toWView, FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(vbox),hitButton, FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(vbox),standButton, FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(vbox),quitButton, FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(vbox),dealerCards, FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(vbox),dcardrow, FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(vbox),playerCards, FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(vbox),pcardrow, FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(vbox),fromView, TRUE,TRUE,0);
    //BUilds the events.
    g_signal_connect(G_OBJECT(hitButton), "button_press_event", G_CALLBACK(hitCallback), fromView);
    g_signal_connect(G_OBJECT(joinButton), "button_press_event", G_CALLBACK(joinCallback), toView);
    g_signal_connect(G_OBJECT(wagerButton), "button_press_event", G_CALLBACK(wagerCallback), toWView);
    g_signal_connect(G_OBJECT(standButton), "button_press_event", G_CALLBACK(standCallback), fromView);
    g_signal_connect(G_OBJECT(quitButton), "button_press_event", G_CALLBACK(quitCallback), fromView);
    gtk_widget_show_all(window);
    g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(delete_event), NULL);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy_event), NULL);
    if (argc != 3){
        cerr << "Usage: chat_client <host> <port>\n";
        return 1;
    }
    asio::io_context io_context;
    tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve(argv[1], argv[2]);
    c = new chat_client(io_context, endpoints);
    assert(c);
    thread t([&io_context](){ io_context.run(); });
    gtk_main();
    gdk_threads_leave();
    c->close();
    t.join();
    return 0;
}
