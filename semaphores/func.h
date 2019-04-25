#ifndef FUNC_H_
#define FUNC_H_

#include <semaphore.h>

#define MAX_INPUT_SIZE 200
#define DESCR_SIZE 100
#define MAX_CLIENTS 2
#define MAX_QUEUE 1
#define TIME_SERVER 10
#define SUCCESS 1
#define FAILURE 0
#define ITEM_ID 0
#define DESCR 1
#define PRICE 2
#define MIN_TIME 3
#define MAX_TIME 4
#define MENU_SIZE 10
#define RW_ACCESS 0666

extern const char *SEM_MANAGER_LOCK;
extern const char *SEM_CASHIER_QUEUE;
extern const char *SEM_CASHIER_LOCK;
extern const char *SEM_CLIENT_CASHIER;
extern const char *SEM_CLIENT_QUEUE;
extern const char *SEM_CLIENT_LOCK;
extern const char *SEM_CLIENT_SERVER;
extern const char *SEM_SERVER_QUEUE;
extern const char *SEM_SERVER_LOCK;

extern const char *database_dir;

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
  int item_id;
  int cashier_time;
  int food_time;
  int server_time;
  int eat_time;
  double arrival_time;
  double depart_time;
  double bill;
} client;

typedef struct {
  sem_t *manager_lock;
  sem_t *cashier_queue;
  sem_t *cashier_lock;
  sem_t *client_cashier;
  sem_t *client_queue;
  sem_t *client_server;
  sem_t *client_lock;
  sem_t *server_queue;
  sem_t *server_lock;
} semaphores;

typedef struct {
  int client;
  int cashier;
  int max_cashier;
  int server;
  int client_queue;
} counters;

typedef struct {
  menu_item menu[MENU_SIZE];
  client clients[MAX_CLIENTS];
  counters counters_;
  int server_time;
} shared_mem;

extern shared_mem *attach_shared_mem(int);
extern void detach_shared_mem(shared_mem *, int);
extern void detach_shared_mem_and_close_all_sem(shared_mem *, int, semaphores);
extern int randomize_n(int);
extern int randomize_bt(int, int);

#endif // FUNC_H_
