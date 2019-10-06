//
//  main.cpp
//  Scheduling
//
//  Created by Yitz Ben-Ami on 11/20/18.
//  Copyright © 2018 Yitz Ben-Ami. All rights reserved.
//

#include <iostream>
#include <fstream>
using namespace std;

class Scheduling{
public:
    class Node{
    public:
        int jobid;
        int jobtime;
        Node* next;
        Node(int t, int i){
            jobid = i;
            jobtime = t;
            next = NULL;
        }
    };
    Scheduling(int n){
        numNodes = n;
    };
    int numNodes;
    int totalJobTimes;
    int procGiven;
    int **adjacencyMatrix;
    int **scheduleTable;
    int *jobTimeAry;
    Node* open;
    int *processJob;
    int *processTime;
    int *parentCount;
    int *kidCount;
    int *jobDone;
    int *jobMarked;
    Node* n;
    int currentTime;
    
    Node* createNode(int i){
        Node* node = new Node(jobTimeAry[i],i);
        return node;
    }
    void loadMatrix(ifstream& in){
        //allocate
        adjacencyMatrix = new int*[numNodes+1];
        for (int i = 0; i<numNodes+1; i++) {
            adjacencyMatrix[i] = new int[numNodes+1];
        }
        // zero out
        for (int i =0; i<numNodes+1; i++) {
            for (int j =0; j<numNodes+1; j++) {
                adjacencyMatrix[i][j]=0;
            }
        }
        parentCount = new int[numNodes+1];
        kidCount = new int[numNodes+1];
        for(int i = 1; i<numNodes+1; i++){
            parentCount[i] = 0;
            kidCount[i] = 0;
        }
        //load matrix
        int i,j;
        while(in>>i>>j){
            adjacencyMatrix[i][j] = 1;
            parentCount[j]++;
            kidCount[i]++;
        }
    }
    void init(){
        
        scheduleTable = new int*[numNodes+1];
        for (int i=0; i<numNodes+1; i++) {
            scheduleTable[i] = new int[totalJobTimes+1];
        }
        for(int i=0;i<numNodes+1;i++){
            for(int j=0;j<=totalJobTimes;j++){
                scheduleTable[i][j]=0;
            }
        }
        processJob = new int[numNodes+1];
        processTime = new int[numNodes+1];
        jobDone  = new int[numNodes+1];
        jobMarked = new int[numNodes+1];
        for (int i=1; i<=numNodes; i++) {
            jobMarked[i]=0;
            jobDone[i]=0;
            processTime[i]=0;
            processJob[i]=0;
        }
        
        open = new Node(0,0);
    }
    int computeTotalJobTimes(ifstream& in){
        int job,t,numNodes,totalJobTimes = 0;
        in>>numNodes;
        jobTimeAry = new int[numNodes+1]();
        while(in>>job>>t){
            totalJobTimes += t;
            jobTimeAry[job] = t;
        }
        
        return totalJobTimes;
    }
    int getOrphan(){
        for(int i = 1; i<=numNodes; i++){
            if(jobMarked[i] == 0 && parentCount[i]==0)
                return i;
        }
        return -1;
    }
    void Insert2Open(Scheduling::Node* n){
        Node* spot = open;
        while (spot->next != NULL && kidCount[spot->next->jobid] > kidCount[n->jobid]) {
            spot = spot->next;
        }
        n->next = spot->next;
        spot->next = n;
    }
    Node* removeFront(){
        Node* newNode = open->next;
        open->next = open->next->next;
        newNode->next = NULL;
        return newNode;
    }
    string printList(){
        Node *node = open->next;
        string OPEN;
        OPEN += "\nOPEN: ";
        while(node != NULL){
            OPEN +="("+to_string(node->jobid)+","+to_string(node->jobtime)+") -> ";
            node = node->next;
        }
        OPEN += "NULL";
        return OPEN;
    }
    int findProcessor(){
        for (int i=1; i <= procGiven; i++) {
            if(processTime[i]<=0)
                return i;
        }
        return -1;
    }
    void updateTable(int availProc,int currentTime,Node* newJob){
        for (int t = currentTime; t<currentTime+newJob->jobtime; t++) {
            scheduleTable[availProc][t] = newJob->jobid;
        }
    }
    int checkCycle(){
        if(open->next == NULL && !isGraphEmpty() && allProcFinishAllJobs()){
            return 1;
        }
        else
            return 0;
    }
    bool allProcFinishAllJobs(){
        for(int p=1; p<=procGiven; p++){
            if(processJob[p] > 0){
                return false;
            }
        }
        return true;
    }
    bool isGraphEmpty(){
        for(int j=1; j<=numNodes; j++){
            if(jobDone[j] == 0){
                return false;
            }
        }
        return true;
    }
    void printTable(ofstream& out, int currentTime){
        out << "      ";
        for(int i = 0; i<=currentTime; i++){
            if(i==10) out << "-"<< i << "-";
            else
                out << "--" << i << "--";
        }
        out << endl;
        for(int i = 1; i < procGiven+1; i++){
            if(i>9) out << "P(" << i << ")| ";
            else
                out << "P(" << i << ") | ";
            for(int j = 0; j <= currentTime; j++){
                if(scheduleTable[i][j]<10) out << " ";
                if(scheduleTable[i][j] != 0)
                    out << scheduleTable[i][j] << " | ";
                else
                    out << "-" << " | ";
            }
            out <<endl;
        }
        out<<"\n";
    }
    void findDoneJob(){
        int job;
        for (int i = 1;  i<= procGiven; i++){
            job = -1;
            if(processTime[i]==0){
                job=processJob[i];
                processJob[i]=0;
            }
            if(job != -1){
                deleteNode(job);
                deleteEdge(job);
            }
        }
    }
    void deleteNode(int job){
        jobDone[job] = 1;
    }
    void deleteEdge(int job){
        for (int kid=1; kid<=numNodes; kid++) {
            if(adjacencyMatrix[job][kid] > 0)
                parentCount[kid]--;
        }
    }
    void debugPrint(int currentTime,ofstream& debug){
        debug<<"Current Time: "<<currentTime<<endl;
        debug<<"Marked Jobs: ";
        for(int i=1;i<=numNodes;i++)
            if(jobMarked[i]==1)
                debug<<i<< ", ";
        debug<<endl;
        debug<<"Processing Time: ";
        for (int i = 1; i <= procGiven; i++)
            debug<<"P(" << i << ") = " << processTime[i] << ", ";
        debug<<endl;
        debug<<"Processing Jobs: ";
        for (int i = 1; i <= procGiven; i++)
            if(processJob[i] != 0)
                debug<<"P(" << i << ")->" << processJob[i] << ", ";
        debug<<endl;
        debug<<"Done Jobs: ";
        for(int i = 1; i <= numNodes; i++)
            if(jobDone[i] == 1)
                debug<<i<< ", ";
        debug<<endl;
    }
    int getUsedProc(){
        int p = 0;
        for(int i = 1; i <= procGiven; i++)
            if(processTime[i] == 1)
                p++;
        return p;
    }
    void fillScheduleTable(ifstream& in1,ifstream& in2,ofstream& o1,ofstream& debug){
        
        init();
        int orphanNode=0,availProc,procUsed = 0;
        currentTime = 0;        
        while (!isGraphEmpty()) {
            while(true){
                orphanNode = getOrphan();
                if(orphanNode == -1)break;
                jobMarked[orphanNode] = 1;
                n = createNode(orphanNode);
                Insert2Open(n);      
            }

            cout<<printList()<<endl;
            
            Node* newJob;
            while (open->next != NULL && procUsed<procGiven) {
                availProc = findProcessor();
                if(availProc > 0){
                    procUsed++;
                    newJob = removeFront();
                    processJob[availProc] = newJob->jobid;
                    processTime[availProc] = newJob->jobtime;
                    updateTable(availProc, currentTime, newJob);
                }
                else if(availProc == -1)
                    break;
            }
            if(checkCycle() >= 1){
                cout<<"We found a cycle. Bye....\n";
                in1.close();
                in2.close();
                o1.close();
                debug.close();
                exit(0);
            }
            printTable(o1,currentTime);
            currentTime++;
            
            procUsed = getUsedProc();

            for(int i=1;i<=numNodes;i++){
                if(processTime[i] != 0)
                    processTime[i]--;
            }

            findDoneJob();
            
            debugPrint(currentTime,debug);
        }
    }
};

int main(int argc, const char * argv[]) {
    
    ifstream in1,in2;
    ofstream o1,debug;
    in1.open(argv[1]);
    in2.open(argv[2]);
    o1.open(argv[3]);
    debug.open(argv[4]);

    int numNodes,procGiven;
    in1>>numNodes;
    Scheduling s(numNodes);

    s.loadMatrix(in1);
    
    s.totalJobTimes = s.computeTotalJobTimes(in2);

    cout<<"how many processors are needed?: ";
    cin>>procGiven;
    while(procGiven<=0){
        cout<<endl<<"how many processors are needed?: ";
        cin>>procGiven;
    }
    if(procGiven>numNodes){
        procGiven = numNodes;
    }
    s.procGiven = procGiven;

    s.fillScheduleTable(in1,in2,o1,debug);

    s.printTable(o1,s.currentTime);

    in1.close();
    in2.close();
    o1.close();
    debug.close();
    
    return 0;
    
}
