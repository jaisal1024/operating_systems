# a fully operational restaurant - THE WEST END DINER
Jaisal Friedman
Operating Systems Spring 2019
NYU Abu Dhabi

## overview
### Coordinator
1. create shared memory through a structure that stores segment of memory
2. output the shmid (shared segment ID)
3. declare 2 structures: 1 for semaphores and 1 for the shared memory segment. Both of which are denoted in the header file func.h
4. Read in at least 10 menu items from menu.txt file
5. Put manager to sleep with sem_wait on a manager lock that is woken up by the final client
6. calculate daily statistics
7. write to daily_stats.txt and database.txt file
8. detach shared memory
9. unlink semaphores
10. Say goodbye :)

### Cashiers
1. attach to shared memory
2. initialize semaphore struct with named semaphores needed
3. acquire cashier lock semaphore and check if there are cashiers left. if there are increment the client count and store the client number.  
4. loop while there are still clients left to be serviced
5. acquire the cashier lock
6. perform a trywait on the client queue. If it fails (client queue is empty), release lock, and go on a break.
7. if it succeeds, signal the cashier queue, releasing one client to populate the shared_mem at the current counter.
8. wait for the client to signal back on client-cashier semaphore
9. Decrement clients left, release lock, and perform the client service by sleeping for random service time.
10. add order to shared_mem menu struct and print to database the client information.
11. update the clients_left by accessing the shared_mem_ client counter.
12. continue loop
13. after loop - acquire the lock and increment the cashier counter if its not already full
14. release the shared_mem and semaphores and exit.

### Client
1. attach to shared memory
2. initialize semaphore struct with named semaphores needed
3. acquire client lock and check if there is space in the client queue.
if there are increment the client queue count and store the client number.
4. wait for the cashier to signal on cashier queue semaphore
5. once signaled, populate the shared_mem with client information and then signal back to cashier when done on the client-cashier semaphore
6. acquire the service time and wait to be served (go to sleep)
7. wait for food to be ready, go to sleep
8. join the server queue by signaling to the server_queue semaphore
9. wait for server-client semaphore to signal while the server processes the client
10. set the server time, then sleep for eating time
11. check if its the last client; if it is signal the manager lock to close the restaurant.
12. release the shared_mem and semaphores and exit.


### Server
1. attach to shared memory
2. initialize semaphore struct with named semaphores needed
3. acquire server lock semaphore and check if there are any servers in action. if there isn't increment the server count and continue
4. loop while the clients left is greater than 0. clients left is initially MAX_CLIENTS size.
5. wait on server_queue semaphore for clients to signal they need to be serviced.
6. determine a service time and sleep
7. signal back to client through client-server semaphore that the service is complete
8. decrement the clients_left
9. once the loop finishes: acquire the lock, set the server count to 1, release the lock
10. release the shared_mem and semaphores and exit.

### MACROS (defined in func.h)
MAX_INPUT_SIZE 200
DESCR_SIZE 100
MAX_CLIENTS 5 - max number of clients per day
MAX_QUEUE 2 - max length of client queue
TIME_SERVER 10 - max time the server will wait
SUCCESS 1
FAILURE 0
ITEM_ID 0
DESCR 1
PRICE 2
MIN_TIME 3
MAX_TIME 4
MENU_SIZE 10 - update if you add items to menu.txt file
RW_ACCESS 0666

### Database & Daily statistics
- the database.txt file stores the database file which the program writes to each day. This is appended to each day.
- the daily_stats.txt file stores the daily statistics at the end of each day of the restaurant. This is overwritten each time the restaurant runs.

### Makefile && scripts
- make del_sems will produce the ./del_sems script
- sh kill_ipcs.sh and ./del_sems invoked on make clean to ensure that shared memory and named semaphores are removed from kernel
- sh kill_ipcs.sh will remove all System V references to shared memory and semaphores
- if executed till termination, the manager will remove the shared memory and unlink the named semaphores
