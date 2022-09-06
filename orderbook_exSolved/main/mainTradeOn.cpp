#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <future>
#include <unordered_map>
#include <map>
#include <queue>
#include <mutex>
#include <cstring>
using namespace std;

#include "../utils/threadSafeQueue.h"
#include "../src/input.h"
#include "../src/output.h"

struct tOrder {
    int price;
    int qty;
    char side;
};

struct tUserInfo {
    int userId;
    int userOrderId;
    int qty;
};

struct tQty {
    int total = 0;
    vector<tUserInfo> userInfo;
    
    public:
    void addQty(int quantity, int userId, int userOrderId){
        total += quantity;
        // TO DO, insert in userInfo all the relevant info
    }
    
    void subtractQty(int quantity, int userId, int userOrderId){
        total -= quantity;
        // TO DO, erase in userInfo all the relevant info
    }
};

struct orderbook {
    map<int, int> asks; // TO DO  map<int, tQty> asks;
    map<int, int, greater<int>> bids; // TO DO  map<int, tQty> bids;
    
    // this data structure is for search orders(to cancel them) in constant time or O(1)
    // TO DO, check string(user) + string(userOrderId) are unique_keys
    unordered_map<string, tOrder> itOrders; // <string(user) + string(userOrderId), tOrder(price, qty, side)>
};


int cont = 0; // This is just for my custom output scenario, to have a counter that counts the scenarios. It should not be a global variable, I have put it here simply for convenience

// this is to pass the parameter T& o, two different data types, asks(map<int, int>)/bids(map<int, int, greater<int>>)
template<typename T, typename Q>
void newOrderProcessing(const vector<string>& order, T& o, Q& o2, bool rejectOrder, ThreadsafeQueue<string>& linesOut, const pair<int, int>& bestAnt, pair<int, int>& priceQty, const string& side){

    if(priceQty.first == 0){
        // Market order
        if(o2.size() == 0)
            linesOut.push("R, " + order[1] + ", " + order[6]); // Market order cannot be executed because there is no bid/ask on the opposite side of the order book
        else{
        
            auto&& it = o2.begin();
            while (priceQty.second > 0 && it != o2.end()) {
                // another while is needed here, because each price can have the qty of more than one user
                int minumum = max(it->second, priceQty.second);
            
                it->second -= minumum;
                priceQty.second -= minumum;
            
                if (it->second == 0){ 
                    pair<int, int> userBuy, userSell;
                    
                    if(side == "B"){
                        userBuy = {stoi(order[1]), stoi(order[6])}; // user, userOrderId 
                        // TO DO userSell = {};
                    }
                    else{ // side == 'S'
                        // TO DO userBuy = {};
                        userSell = {stoi(order[1]), stoi(order[6])}; // user, userOrderId 
                    }
                    
                    // The las element is minumum because order could be partially filled
                    linesOut.push("T, " + to_string(userBuy.first) + ", " + to_string(userBuy.second) + ", " + to_string(userSell.first) + ", " + to_string(userSell.second) + ", " + order[3] + ", " + to_string(minumum));
                    it = o2.erase(it);
                }
                else 
                    ++it;
            }   
        }
        
        if(bestAnt.first != o.begin()->first || bestAnt.second != o.begin()->second)  // if bbo change 
            linesOut.push("B, " + side + ", " + to_string(o.begin()->first) + ", " + to_string(o.begin()->second)); 
        
    }    
    else if(rejectOrder){
        // Reject        
        linesOut.push("R, " + order[1] + ", " + order[6]); // user, userOrderId        
    }
    else{
        // Acknowledge
        linesOut.push("A, " + order[1] + ", " + order[6]); // user, userOrderId
                  
        if(o.find(priceQty.first) == o.end())
            o[priceQty.first] = priceQty.second; 
        else
            o[priceQty.first] += priceQty.second; // we add the qty of the order 
            
        if(bestAnt.first != o.begin()->first || bestAnt.second != o.begin()->second)  // if bbo change 
            linesOut.push("B, " + side + ", " + to_string(o.begin()->first) + ", " + to_string(o.begin()->second)); 
    }   
     
}



template<typename T>
void orderCancelProcessing(T& o, ThreadsafeQueue<string>& linesOut, const tOrder& itOrder, const pair<int, int>& bestAnt){

    o[itOrder.price] -= itOrder.qty; // we subtract the qty of the order            
    if(o[itOrder.price] == 0) // if amount is 0, we erase the price
        o.erase(itOrder.price);
    
    if(o.size() == 0)
       linesOut.push("B, " + string(1, itOrder.side) + ", -, -"); 
    else if(bestAnt.first != o.begin()->first || bestAnt.second != o.begin()->second)  // if bbo change 
        linesOut.push("B, " + string(1, itOrder.side) + ", " + to_string(o.begin()->first) + ", " + to_string(o.begin()->second)); 
            
}



void orderInProcessing(const vector<string>& order, orderbook& o, ThreadsafeQueue<string>& linesOut){
    pair<int, int> bestAskAnt = {0, 0}, bestBidAnt = {0, 0};
    
    // This for best/top of book change
    if(o.asks.size() > 0)
        bestAskAnt = {o.asks.begin()->first, o.asks.begin()->second};
    if(o.bids.size() > 0)
        bestBidAnt = {o.bids.begin()->first, o.bids.begin()->second};


    if(order[0] == "N"){ // new order
        pair<int, int> priceQty = {stoi(order[3]), stoi(order[4])}; // {price, qty} 
        o.itOrders[ order[1] + "-" + order[6] ] = {priceQty.first, priceQty.second, order[5][0]}; // (string(user) + string(userOrderId)) = {price, qty, side}
       
        if(order[5] == "S") // willing to sell
            newOrderProcessing(order, o.asks, o.bids, o.bids.size() > 0 && priceQty.first <= o.bids.begin()->first, linesOut, bestAskAnt, priceQty, order[5]);
        else if(order[5] == "B") // willing to buy
            newOrderProcessing(order, o.bids, o.asks, o.asks.size() > 0 && priceQty.first >= o.asks.begin()->first, linesOut, bestBidAnt, priceQty, order[5]);  
        else
            linesOut.push("ERROR: <orderProcessing> N order " + order[5] + " side, impossible case!");
            
    }
    else if(order[0] == "C"){ // cancel order
        linesOut.push("A, " + order[1] + ", " + order[2]);
        // TO DO, checkRejectOrder(), here it would verify that the order to cancel really exists
        
        string user = order[1] + "-" + order[2]; // <user, userOrderId>
        auto&& itOrder = o.itOrders[user];
        
        if(itOrder.side == 'S')        
            orderCancelProcessing(o.asks, linesOut, itOrder, bestAskAnt);
        else if(itOrder.side == 'B')
            orderCancelProcessing(o.bids, linesOut, itOrder, bestBidAnt);
        else
            linesOut.push("ERROR: <orderProcessing> C order, the order " + string(1, itOrder.side) + " must be in bids or asks, impossible case!");
        
    }
    else{ // flush orderbook from last processed symbol    
        o.asks.clear();
        o.bids.clear();
        o.itOrders.clear();
        
        cont++;
        linesOut.push(order[0] + " (flush scenario: " + to_string(cont) + ")\n");       
    }   
    
}




int main(){
    orderbook oBook; 
    ThreadsafeQueue<string> linesIn, linesOut; // better 2 different queues than 1 for I/O, too much overhead mutex performance
    Input i; Output o;
    
    // I prefer to use std::ref than global variables. async() have the option of launch::async
    future<void> inputThread = async(&Input::readInput, i, ref(linesIn)); 
    future<void> ouputThread = async(&Output::writeOutput, o, ref(linesOut));
    
    while(1){
        if(linesIn.size() > 0){ 
            orderInProcessing( i.readLine( linesIn.front() ), oBook, linesOut );
            linesIn.pop();
        }
    }
}



