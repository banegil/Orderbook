
Build in folder /main with make
Run in folder /main with /a.out



-Folders:
    /files contains input_file.csv and output_file.cvs
    /main contains the exe file, make, main.cpp(more compact code), oldMain.cpp(old implementation, repetitive code), mainTradeOn.cpp(doesn't work, in "Future implementations" I explain why and what I would have done if I had more time)
    /utils contains the file threadSafeQueue.h, which makes sure that both the Output thread and the Input thread work correctly.
    /src contains the files output.h and input.h, which contain the Output and Input classes, respectively.


-Threads:
    This program follows a scheme similar to producer/consumer, but with an intermediate thread that supervises the element, modifies it, to later be consumed.
    This program has 3 threads. The Input thread, the Output thread, and the main thread. 
    The main thread is the one with the most workload, there could be another version where the main thread offloads some work to the input thread and the output thread,
    or create more threads inside the main thread.


-Informal code explanation:
    We create 2 different queues, one for the input and one for the output, in the files src/input.h and src/output.h two constants MAX_LINES_READ and MAX_WRITES_READ have been defined, this is simply to give a size to the queues,
    in case in the event that one reads faster than the other writes, or vice versa.

    The Input and Output class contain one function each (readInput and writeOutput), which are going to be two threads that are in charge of reading and writing the data. The Input class has another function called
    readLine(converts the line into a vector of strings), in the Ouput class, the function that writes the data does it in a customized way, that is, it reads the file output_file.csv to check if it matches.

    Once the Input and Output threads have been created, the main thread is in charge of processing each new line that arrives in the InputQueue(linesIn) and after processing it, writes in the OutputQueue(linesOut).

    On line processing. At first I thought of using an unordered_map<tSymbol, orderbook> to store an orderbook in each symbol, I discarded that idea because there really are no "scenarios" where symbols are mixed, they are always the same. For a more advanced implementation it would be necessary.
    I use two template functions newOrderProcessing and orderCancelProcessing to avoid repeating code.   
  
    
-Future implementations: 
    It would be convenient to use at least one file for error handling, this program has not been specially designed to handle possible errors or use try catch exceptions. the most important treatment is that of the syntax,
    making sure that the input is correct, but also possible cases in which an orderId or userId is not recognized when canceling an order for example or simply possible code errors.    

    Market orders: the easiest option would have been to just keep the main.cpp code and do a search on unordered_map<string, tOrder> itOrders;(line 32 main.cpp) for value, but that's catastrophic, O(n) on an n that can be very long.
    In mainTradeOn.cpp I have created the structs tUserInfo and tQty, in this way the aks/bids of the main.cpp that had a <tPrice, tQty> type structure in mainTradeOn.cpp would be <int, tQty>. tQty is a class that contains
    the information of each price of each symbol, for example, price 11 in the IBM symbol contains the information (tQty) of 3 users who have limit orders at that price.
    The data structure created is prepared to support what was mentioned above, but the implementation that I have done in newOrderProcessing(line 59 mainTradeOn.cpp) is only assuming that each price has only one user,
    which is not real, it would have to be replaced by an implementation which takes into account that many users can have orders at the same price.
    The rest of the code should also be updated (cancel orders etc)
