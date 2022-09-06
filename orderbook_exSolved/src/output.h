


// In the event that the system reads faster than it writes, or vice versa, it is ok to have a maximum size of their respective queues(e.g. MAX_LINES_WRITE)
#define MAX_LINES_WRITE 1000

class Output {

    public:
    void writeOutput(ThreadsafeQueue<string>& linesOut){
        string line = "#";
        fstream file ("../files/output_file.csv", ios::in);
        bool errorScenario = 0;
        
        while(1){
        
            if(linesOut.size() > 0){           
            
                if(linesOut.front()[0] == 'F' || linesOut.front()[0] == 'E'){ // E for error messages 
                
                    if(errorScenario){
                        cout << "ERROR <writeOutput> in current scenario output with expected output, please check again!" << endl;
                        cout << "NOTE(if there are more scenarios in the same execution, all scenarios going to throw error, even if they are OK, this is just a fast error checker)" << endl;
                    }
                      
                    errorScenario = 0;  
                        
                    linesOut.print();
                    linesOut.pop();
                }
                else{
                
                    while(getline(file, line) && (line[0] == '#' || line.size() == 0)){
                        // empty body
                    }
                    
                    linesOut.print();    
                    
                    if(line != linesOut.front())
                        errorScenario = 1;
                                 
                    linesOut.pop(); 
                }
            }
        }
    } 
};
