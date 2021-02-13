# SYNC-TICKET

The aim of the project is to simulate a queue for cinema tickets. There are 3 tellers who are responsible for handling the requests of clients. An arrival time and service time interval are given for each client. According to their arrival time, clients are assigned to the available teller sequentially. The implementation of this simulation uses multithreading. Each client and teller are represented as a thread. To avoid conflicts, synchronization techniques are used. After handling client requests, tellers output the seat assignments. When all clients are serviced, program terminates.

## Compilation and Execution

Project requires: 
* Unix based environment 
* gcc/g++ compiler (Version 9.3.0 or more recent)
<!-- end of the list -->
The following commands will be sufficient:
* g++ -pthread -o simulation.o ticket.cpp
* ./simulation.o <input_file> <output_file>
<!-- end of the list -->
For simplicity, make file for compilation and test script are also provided.

## Input/Output

The input format: First row should contain the name of the hall. Second row indicates the number of clients. Then each row belongs to a client. Rows keep client name, arrive time, service time, and requested seat number.

Example input:

```
OdaTiyatrosu
10
Client1,10,50,5
Client2,20,20,1
Client3,30,20,5
Client4,45,10,1
Client5,65,10,5
Client6,70,10,10
Client7,72,10,10
Client8,100,20,10
Client9,95,20,10
Client10,90,20,10

```

The corresponding output:

```
Welcome to the Sync-Ticket!
Teller A has arrived.
Teller B has arrived.
Teller C has arrived.
Client2 requests seat 1, reserves seat 1. Signed by Teller B.
Client3 requests seat 5, reserves seat 2. Signed by Teller C.
Client4 requests seat 1, reserves seat 3. Signed by Teller B.
Client1 requests seat 5, reserves seat 5. Signed by Teller A.
Client5 requests seat 5, reserves seat 4. Signed by Teller A.
Client6 requests seat 10, reserves seat 10. Signed by Teller B.
Client7 requests seat 10, reserves seat 6. Signed by Teller C.
Client10 requests seat 10, reserves seat 7. Signed by Teller A.
Client9 requests seat 10, reserves seat 8. Signed by Teller B.
Client8 requests seat 10, reserves seat 9. Signed by Teller C.
All clients received service.

```

## Implementation

The main thread reads the input file and creates the client and teller threads using `pthread_create()`. It also initates the synchronization tools to avoid conflicts among threads. **Semaphores** and **mutex locks** are used to keep the threads synchronized.  \
Although their arrival times differ, all client threads are created at the beginning. Thus, they need to sleep until their arrival time comes. (in milliseconds) When a client thread wakes up in its arrival time, it needs to wait until a teller is available. This condition is provided with a counting semaphore.
```
sem_wait(&empty);
```
When the block of the semaphore is over, this means there is an available teller. Client detects it and marks that teller as unavailable (Since it is busy with itself now.) Then client needs to pass its information to the teller. (Client name, service time, requested seat) This data transfer is a basic **producer consumer problem**. Which is handled with this piece of code:
``` 
sem_wait(&empty1);
pthread_mutex_lock(&mut);
buffer1[0]=arr[0];    //name
buffer1[1]=arr[2];  //service time
buffer1[2]=arr[3];  //seat
pthread_mutex_unlock(&mut);
sem_post(&full1);   //teller is signaled that buffer is ready
```
That is all about the client. When data is passed, teller starts to work. It first consumes (obtains) the data with the consumer part of the problem:
``` 
sem_wait(&full1);     //wait for buffer to be filled by client
pthread_mutex_lock(&seatLock);   //lock the arranging seat procedure
name=buffer1[0];  //receive data from client
service=stoi(buffer1[1]);
seatWanted=stoi(buffer1[2]);
sem_post(&empty1);  //signal that buffer is consumed
```
With the necessary data, teller reserves the requested seat if available. Else, it reserves the min available seat. Of course, more than one teller can make reservation simulatenously, hence, mutex locks are again used there. After job is done, teller sleeps as long as the service time (part of the simulation). After that, teller signals that it became available with semaphore update:
```
sem_post(&empty);   //Signaling empty for receiving clients 
```
This loop goes on until no clients remain for service.


