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

struct orderbook {
    map<int, int> asks; // <price, quantity>
    map<int, int, greater<int>> bids; // <price, quantity>
    
    // this data structure is for search orders(to cancel them) in constant time or O(1)
    // TO DO, check string(user) + string(userOrderId) are unique_keys
    unordered_map<string, tOrder> itOrders; // <string(user) + string(userOrderId), tOrder(price, qty, side)>
};

int cont = 0; // This is just for my custom output scenario, to have a counter that counts the scenarios. It should not be a global variable, I have put it here simply for convenience

void orderInProcessing(const vector<string>& order, orderbook& o, ThreadsafeQueue<string>& linesOut){
    pair<int, int> bestAskAnt = {0, 0}, bestBidAnt = {0, 0};
    string side = "";
    int price, qty;
    
    // This for best/top of book change
    if(o.asks.size() > 0)
        bestAskAnt = {o.asks.begin()->first, o.asks.begin()->second};
    if(o.bids.size() > 0)
        bestBidAnt = {o.bids.begin()->first, o.bids.begin()->second};


    if(order[0] == "N"){ // new order
        pair<int, int> priceQty = {stoi(order[3]), stoi(order[4])}; // {price, qty} 
        o.itOrders[ order[1] + "-" + order[6] ] = {priceQty.first, priceQty.second, order[5][0]}; // (string(user) + string(userOrderId)) = {price, qty, side}
       
        if(order[5] == "S"){ // willing to sell
            if(o.bids.size() > 0 && priceQty.first <= o.bids.begin()->first) // check Reject order          
                linesOut.push("R, " + order[1] + ", " + order[6]); // Reject
            else{

                linesOut.push("A, " + order[1] + ", " + order[6]);  // Acknowledge
                          
                if(o.asks.find(priceQty.first) == o.asks.end())
                    o.asks[priceQty.first] = priceQty.second; 
                else
                    o.asks[priceQty.first] += priceQty.second; // we add the qty of the order 
                    
                if(bestAskAnt.first != o.asks.begin()->first || bestAskAnt.second != o.asks.begin()->second) { // if bbo change 
                    price = o.asks.begin()->first;
                    qty = o.asks.begin()->second;
                    side = "S";
                }
            }
            
        }
        // same as S
        else if(order[5] == "B"){ // willing to buy
            if(o.asks.size() > 0 && priceQty.first >= o.asks.begin()->first)          
                linesOut.push("R, " + order[1] + ", " + order[6]);                
            else{
            
                linesOut.push("A, " + order[1] + ", " + order[6]); 
            
                if(o.bids.find(priceQty.first) == o.bids.end())
                    o.bids[priceQty.first] = priceQty.second; 
                else
                    o.bids[priceQty.first] += priceQty.second; 
                    
                if(bestBidAnt.first != o.bids.begin()->first || bestBidAnt.second != o.bids.begin()->second) { 
                    price = o.bids.begin()->first;
                    qty = o.bids.begin()->second;
                    side = "B";
                }
            }
     
        }    
        else
            linesOut.push("ERROR: <orderProcessing> N order " + order[5] + " side, impossible case!");
            
    }
    else if(order[0] == "C"){ // cancel order
        linesOut.push("A, " + order[1] + ", " + order[2]);
        // TO DO, checkRejectOrder(), here it would verify that the order to cancel really exists
        
        string user = order[1] + "-" + order[2]; // <user, userOrderId>
        auto&& itOrder = o.itOrders[user];
        
        if(itOrder.side == 'S'){
        
            o.asks[itOrder.price] -= itOrder.qty; // we subtract the qty of the order            
            if(o.asks[itOrder.price] == 0) // if amount is 0, we erase the price
                o.asks.erase(itOrder.price);
            
            if(o.asks.size() == 0)
                side = "S-";
            else if(bestAskAnt.first != o.asks.begin()->first || bestAskAnt.second != o.asks.begin()->second) { // if bbo change 
                price = o.asks.begin()->first;
                qty = o.asks.begin()->second;
                side = "S";
            }
                
        }
        // same as S
        else if(itOrder.side == 'B'){ 
        
            o.bids[itOrder.price] -= itOrder.qty;            
            if(o.bids[itOrder.price] == 0) 
                o.bids.erase(itOrder.price);
             
            if(o.bids.size() == 0)
                side = "B-";                        
            else if(bestBidAnt.first != o.bids.begin()->first || bestBidAnt.second != o.bids.begin()->second) {
                price = o.bids.begin()->first;
                qty = o.bids.begin()->second;
                side = "B";
            }    
        }
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
    
    
    // best/top of book change
    if(side != ""){
        if(side.back() != '-')
            linesOut.push("B, " + side + ", " + to_string(price) + ", " + to_string(qty)); 
        else{
            side.pop_back();
            linesOut.push("B, " + side + ", -, -"); 
        }
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



