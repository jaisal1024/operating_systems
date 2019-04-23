#ifndef FUNC_H_
#define FUNC_H_

#include <semaphore.h>

#define SEGMENT_SIZE 2048
#define MAX_INPUT_SIZE 200
#define DESCR_SIZE 100
#define INIT_CLIENTS_SERVED 0
#define MAX_CLIENTS 20
#define MAX_QUEUE 10
#define SUCCESS 1
#define FAILURE 0
#define ITEM_ID 0
#define DESCR 1
#define PRICE 2
#define MIN_TIME 3
#define MAX_TIME 4
#define MENU_SIZE 10
#define RW_ACCESS 0666

extern const char *SEM_CASHIER;
extern const char *SEM_CLIENT;
extern const char *SEM_LOCK;

typedef struct {
  int item_id;
  char description[DESCR_SIZE];
  double price;
  int min_time;
  int max_time;
  int quantity;
} menu_item;

typedef struct {
  int client_id;
  double time_waiting;
  double time_in_shop;
  double bill;
} client;

typedef struct {
  menu_item menu[MENU_SIZE];
  sem_t *sem_cashiers;
  sem_t *sem_binary_locking;
  sem_t *sem_clients;
  client clients[MAX_CLIENTS];
} shared_mem;

extern shared_mem *attach_shared_mem_and_open_sem(int shmid);
extern void detach_shared_mem_and_close_sem(shared_mem *, int shmid);

#endif // FUNC_H_
