#include <iostream>
#include <pthread.h>
#include <fstream>
#include <bits/stdc++.h> 
#include <unistd.h>
#include <semaphore.h>

using namespace std;
using namespace std::chrono;

/**
 * Runners
 */
void *teller(void* param);
void *client(void* param);

/**
 * Mutex
 */
pthread_mutex_t start;   //lock to keep client threads without progressing before all threads are created
pthread_mutex_t mut;  //a general mutex for clients
pthread_mutex_t wrt;  //mutex to avoid conflicts while tellers are writing to the file
pthread_mutex_t emptyTellerMutex;   //mutex to avoid conflicts when empty teller list is updated 
pthread_mutex_t l1;  
pthread_mutex_t l2;  //lock to keep teller b from writing to file before teller a (at the beginning)
pthread_mutex_t l3;  //lock to keep teller c from writing to file before teller b (at the beginning)
pthread_mutex_t seatLock;  //mutex to avoid conflicts in seat reserving 


vector<bool>emptyTeller={true,true,true};   //vector that keeps the situation of tellers (available or busy)
vector<bool>seats;  //vector that keeps the situation of tellers (available or reserved)

/**
 * Clients pass their information to tellers via buffers 
 */
string buffer1[3];  //buffer for teller1, will keep client name, service time, seat num
string buffer2[3];  //buffer for teller2, will keep client name, service time, seat num
string buffer3[3];  //buffer for teller3, will keep client name, service time, seat num

/**
 * Semaphores
 */
sem_t empty;  //counting semaphore for keep count of empty tellers  (value=0 when all tellers are busy)
sem_t empty1;  //binary semaphore for teller a, resolves producer consumer problem with the client
sem_t empty2;  //binary semaphore for teller b, resolves producer consumer problem with the client
sem_t empty3;  //binary semaphore for teller c, resolves producer consumer problem with the client
sem_t full1;   //binary semaphore for teller a, resolves producer consumer problem with the client
sem_t full2;   //binary semaphore for teller b, resolves producer consumer problem with the client
sem_t full3;   //binary semaphore for teller c, resolves producer consumer problem with the client


vector<vector<string>> tokens; //keeps tokens in the input
vector<int>indices;  //keeps indices for client threads, given as parameter in the thread creation

//Enumeration for tellers, given as parameters in the thread creation
int id1=1;
int id2=2;
int id3=3;

int numClients;  //number of clients 
int counter=0; //counter for number of threads created



ofstream outFile;




int main(int argc, char *argv[]){

    /**
     * Initializing synchronization tools
     */ 
    sem_init(&empty, 0, 0);  
    sem_init(&empty1,0,1); //initilize buffer for teller a as empty
    sem_init(&empty2,0,1); //initilize buffer for teller b as empty
    sem_init(&empty3,0,1); //initilize buffer for teller c as empty
    sem_init(&full1,0,0); 
    sem_init(&full2,0,0); 
    sem_init(&full3,0,0);     

    pthread_mutex_init(&start, NULL);
    pthread_mutex_init(&mut, NULL);
    pthread_mutex_init(&wrt, NULL);
    pthread_mutex_init(&emptyTellerMutex, NULL);
    pthread_mutex_init(&seatLock, NULL);
    pthread_mutex_init(&l1, NULL);
    pthread_mutex_init(&l2, NULL);
    pthread_mutex_init(&l3, NULL);


    pthread_mutex_lock(&l1); //Teller B is going to wait A to unlock and then start
    pthread_mutex_lock(&l2); //Teller B is going to wait A to unlock and then start
    pthread_mutex_lock(&l3); //Teller B is going to wait A to unlock and then start

    string config=argv[1];  //input file 
    string output=argv[2];  //output file 

    ifstream configFile(config);
    outFile.open (output);
    outFile << "Welcome to the Sync-Ticket!\n";

    
    /**
     * Teller threads are created
     * With given id references, tellers will identify themselves
     */

    pthread_t tid[3];  //list of teller threads
    pthread_create(&tid[0], NULL, teller,&id1); //Teller A
    pthread_create(&tid[1], NULL, teller,&id2); //Teller B
    pthread_create(&tid[2], NULL, teller,&id3);  //Teller C


    string theater;
    configFile>>theater;

    //Seat vector is resized according to theater
    if(theater=="OdaTiyatrosu")
        seats.resize(60);
    else if(theater=="UskudarTiyatroSahne")
        seats.resize(80);
    else
        seats.resize(200);
    
    

    string line;
    configFile>>line;  //reading number of clients
    numClients=stoi(line);  

    pthread_t cid[numClients];   //list of client threads
    for(int i=0;i<numClients;i++){
        indices.push_back(i);   //global index references will be passed to client threads
    }

    
    
    /**
     * Client information are read and stored in tokens
     * Since tokens is global, each client thread will be able to access its own data
     */
    for(int i=0;i<numClients;i++){
        configFile>>line;
        vector<string>v;
        stringstream check(line); 
        string token; 
        while(getline(check, token, ',')) { 
            v.push_back(token);
        } 
        tokens.push_back(v); 
    }

    

    /**
     * Client threads are created
     * With given indices references, clients will identify themselves
     */
    for(int i=0;i<numClients;i++){
        pthread_create(&cid[i], NULL, client,&indices[i]);  
    }
    
    /**
     * Wait for all clients to get service
     */
    for(int i = 0; i < numClients; i++) {
	   pthread_join(cid[i], NULL);
	}

    sleep(2);  //Wait for tellers to complete everthing (just a guarantee)
   
    /**
     * Destroying synchronization tools
     */ 
	sem_destroy(&empty);
    sem_destroy(&empty1);
    sem_destroy(&empty2);
    sem_destroy(&empty3);
    sem_destroy(&full1);
    sem_destroy(&full2);
    sem_destroy(&full3);
    pthread_mutex_destroy(&start);
    pthread_mutex_destroy(&mut);
    pthread_mutex_destroy(&wrt);
    pthread_mutex_destroy(&emptyTellerMutex);
    pthread_mutex_destroy(&seatLock);
    pthread_mutex_destroy(&l2);
    pthread_mutex_destroy(&l3);

    outFile<<"All clients received service.\n";

    outFile.close();
    return 0;
}



/**
 * Runner for teller threads
 */
void *teller(void* param){

    int *id = (int *)param;  //id=1->A id=2->B id=3->C 
    char teller=64+*id;

    if(teller=='A'){
        pthread_mutex_lock(&wrt);
        outFile << "Teller "<<teller<<" has arrived.\n";
        pthread_mutex_unlock(&wrt);
        pthread_mutex_unlock(&l2);  //give chance to teller B now
    }
    else if(teller=='B'){
        pthread_mutex_lock(&l2);  //wait for teller a to unlock l2
        pthread_mutex_lock(&wrt);
        outFile << "Teller "<<teller<<" has arrived.\n";
        pthread_mutex_unlock(&wrt);
        pthread_mutex_unlock(&l2);
        pthread_mutex_unlock(&l3);  //give chance to teller c now
    }
    else{
        pthread_mutex_lock(&l3);  //wait for teller B to unlock l3
        pthread_mutex_lock(&wrt);
        outFile << "Teller "<<teller<<" has arrived.\n";
        pthread_mutex_unlock(&wrt);
        pthread_mutex_unlock(&l3);
    }


    string name;
    int service;
    int seatWanted;

    sem_post(&empty);

    /**
     * General loop for tellers
     */
    while(true){
        
        
        /**
         * Each teller has different buffers, different producer consumer problems
         */
        if(*id==1){  //Teller A
            sem_wait(&full1);     //wait for buffer to be filled by client
            pthread_mutex_lock(&seatLock);   //lock the arranging seat procedure
            name=buffer1[0];  //receive data from client
            service=stoi(buffer1[1]);
            seatWanted=stoi(buffer1[2]);
            sem_post(&empty1);  //signal that buffer is consumed
        }
        else if(*id==2){  //Teller B
            sem_wait(&full2);
            pthread_mutex_lock(&seatLock);
            name=buffer2[0];
            service=stoi(buffer2[1]);
            seatWanted=stoi(buffer2[2]);
            sem_post(&empty2);
        }

        else{   //Teller C
            sem_wait(&full3);
            pthread_mutex_lock(&seatLock);
            name=buffer3[0];
            service=stoi(buffer3[1]);
            seatWanted=stoi(buffer3[2]);
            sem_post(&empty3);

        }


        int seatGiven=0;
        if(seatWanted<=seats.size()){   //wanted seat is valid
            if(!seats[seatWanted-1]){  //wanted seat is empty
                seatGiven=seatWanted;
            }
            else{
                for(int i=0;i<seats.size();i++){  //look for the first seat available
                    if(!seats[i]){
                        seatGiven=i+1;
                        break;
                    }
                }
            }
        }
        else{
            for(int i=0;i<seats.size();i++){
                if(!seats[i]){
                    seatGiven=i+1;
                    break;
                }
            }
        }
        if(seatGiven!=0){
            seats[seatGiven-1]=true;
        }
        pthread_mutex_unlock(&seatLock);

        /**
         * Sleep during service time
         * Sleep and nanosleep together to avoid overflow
         */
        struct timespec delta = {0 /*secs*/, (service%1000)*1000000 /*nanosecs*/};
        sleep(service/1000);
        nanosleep(&delta,&delta);

        pthread_mutex_lock(&emptyTellerMutex);
        emptyTeller[*id-1]=true;  //tellers update themselves as available 
        pthread_mutex_unlock(&emptyTellerMutex);
        sem_post(&empty);   //Signaling empty for receiving clients  

        /**
         * Output the procedure
         */
        pthread_mutex_lock(&wrt);
        if(seatGiven==0){
            outFile<<name<<" requests seat "<<seatWanted<<", reserves None"<<". Signed by Teller "<<teller<<".\n";  
        }
        else{
            outFile<<name<<" requests seat "<<seatWanted<<", reserves seat "<<seatGiven<<". Signed by Teller "<<teller<<".\n";
        }
        pthread_mutex_unlock(&wrt);

    }


    pthread_exit(0);
}


void *client(void* param){
    
    /**
     * To begin synchronous, all clients wait until all client threads are created
     */
    pthread_mutex_lock(&start);
    counter++;
    pthread_mutex_unlock(&start);
    while(counter<numClients);   //BELKİ BUSY WAITING SORUNDUR
    

    int *id = (int *)param;
    vector<string>arr=tokens[*id];

    int arrive=stoi(arr[1]);  //retrieve arrive and service times from tokens
    int service=stoi(arr[2]);
    
    /**
     * Clients sleep until their arrival
     */
    struct timespec delta1;
    struct timespec delta2;
    delta1 = {0 /*secs*/, (arrive%1000)*1000000/*nanosecs*/};
    sleep(arrive/1000);
    nanosleep(&delta1,&delta1);
  

    sem_wait(&empty);  //wait for an available teller    
    pthread_mutex_lock(&emptyTellerMutex);
    int tell=0;

    for(int i=0;i<3;i++){   //visit the first teller among available ones
        if(emptyTeller[i]){
            tell=i+1;   //pick the min available
            emptyTeller[i]=false;
            break;
        }
    }

    pthread_mutex_unlock(&emptyTellerMutex);


    
    /**
     * Producer part of producer consumer problem between teller and client
     * Client fills the buffer with his/her own data to reserve seat
     */
    if(tell==1){  //if Teller A is visited
        sem_wait(&empty1);
        pthread_mutex_lock(&mut);
        buffer1[0]=arr[0];    //name
        buffer1[1]=arr[2];  //service time
        buffer1[2]=arr[3];  //seat
        pthread_mutex_unlock(&mut);
        sem_post(&full1);   //teller is signaled that buffer is ready
    }
    else if(tell==2){   //if Teller B is visited
        sem_wait(&empty2);
        pthread_mutex_lock(&mut);
        buffer2[0]=arr[0];    //name
        buffer2[1]=arr[2];  //service time
        buffer2[2]=arr[3];  //seat
        pthread_mutex_unlock(&mut);
        sem_post(&full2);
    }
    else{  //if Teller C is visited
        sem_wait(&empty3);
        pthread_mutex_lock(&mut);
        buffer3[0]=arr[0];    //name
        buffer3[1]=arr[2];  //service time
        buffer3[2]=arr[3];  //seat
        pthread_mutex_unlock(&mut);
        sem_post(&full3);
    }
    
    /**
     * Sleep until service time over
     */
    delta2 = {0 /*secs*/, (service%1000)*1000000 /*nanosecs*/}; 
    sleep(service/1000);
    nanosleep(&delta2,&delta2);
    
    pthread_exit(0);
}












