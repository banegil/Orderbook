

// In the event that the system reads faster than it writes, or vice versa, it is ok to have a maximum size of their respective queues(e.g. MAX_LINES_READ)
#define MAX_LINES_READ 1000

class Input {
    bool checkInputSyntax(const string& line){
        // TO DO, line[0] == 'N' || 'F' || 'C', symbol exist?, char == 'B' || 'S' etc ...
        return 1;
    }
    
    public:    
    void readInput(ThreadsafeQueue<string>& linesIn){
        string line;
        fstream file ("../files/input_file.csv", ios::in);
        
        if(file.is_open()){
            // to give it more realism, this would have to be a while(1), simulating a websocket or some dataStreamer in real time. But in this example, it's just a file
            while(linesIn.size() < MAX_LINES_READ && getline(file, line)){ 
                                 
                if(line[0] != '#' && line.size() > 0){ // remove comment and empty lines
                
                    if(checkInputSyntax(line))                        
                        linesIn.push(line);
                    else
                        cout << "ERROR <InputSyntax>: " << line << endl;
                }
            }
        }
    }
    
    vector<string> readLine(const string& str){
        stringstream ss(str);
        string elem;  
        vector<string> splittedStrings;  
        
        while (getline(ss, elem, ' ')){
        
            if(elem.back() == ',')
                elem.pop_back();
                
            splittedStrings.push_back(elem);
        }
        
        // TO DO, check error splittedStrings.size() (e.g. if splittedStrings[0] == 'N', size need to be 7)
        
        return splittedStrings;
    }
};
