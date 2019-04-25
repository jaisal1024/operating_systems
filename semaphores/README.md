# a fully operational restaurant - THE WEST END DINER
Jaisal Friedman
Operating Systems Spring 2019
NYU Abu Dhabi

## to-do
1. fix server close & client update issue
2. check once more

## overview
### Coordinator
1. create shared memory through a structure that stores this segment of memory
2. output the shmid (shared segment ID)
3. use a structure, then you just deference to access the semaphore. The structure contains
  - array of menu item structs
  - semaphores: no_of_cashiers, binary_locking, and no_of_clients (set to max people)
  Menu struct
  - int item_id;
  - char* description;
  - double price
  - int min_time
  - int max_time
  - int quantity
At least 10 menu items read in by file
4. choose number of cashiers
5. close after it reaches 20 clients serviced
6. detach shared memory
7. Ensure no processes running

### Cashiers
1. Critical section problem on cashier selecting the next free client in the queue. Cannot use a pthread lib mutex lock (only for pthread).
2. binary semaphore (sleep using semaphore.h)
3. writes to shared memory the order for statistics, get the info through shared memory from the client

### Client

### Server
