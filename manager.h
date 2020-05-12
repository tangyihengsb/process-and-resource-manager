#ifndef manager_h
#define manager_h

/* constant for process status */
#define STATUS_READY 0
#define STATUS_RUNNING 1
#define STATUS_BLOCKED 2

/* constant for process priority */ 
#define PRIORITY_INIT 0
#define PRIORITY_USER 1
#define PRIORITY_SYSTEM 2

/* constant for resource id and number */
#define ID_RESOURE_1 1
#define ID_RESOURE_2 2
#define ID_RESOURE_3 3
#define ID_RESOURE_4 4
#define NUM_RESOURE_1 1
#define NUM_RESOURE_2 2
#define NUM_RESOURE_3 3
#define NUM_RESOURE_4 4

struct process_status;
struct resource_status;
struct creation_tree;
struct pcb;
struct rcb;
struct re_l_node;
struct wa_l_node;

typedef struct process_status process_status_t;
typedef struct resource_status resource_status_t;
typedef struct creation_tree creation_tree_t;
typedef struct pcb pcb_t;
typedef struct rcb rcb_t;
typedef struct re_l_node re_l_node_t;
typedef struct wa_l_node wa_l_node_t;

rcb_t *resource_1;
rcb_t *resource_2;
rcb_t *resource_3;
rcb_t *resource_4;

pcb_t *process_init;         // the init process always be running or ready
pcb_t *self;          // the currently running process

pcb_t *rl;            // the ready list of system
pcb_t *bl;            // the blocked list of system


/*
 * structure of the process control block (PCB)
 */
struct pcb {
    int id;                                 // the unique process identifier
    re_l_node_t  **resource_list;           // the resource list  
    process_status_t *status;               // the process status 
    creation_tree_t *creation_tree;         // the pointers to parent process and child process list
    int priority;
    pcb_t *rl_next;                         // the next pointer for process in the ready list
    pcb_t *bl_next;                         // the next pointer for process in the blocked list
    pcb_t *cl_next;                         // the next pointer for process in the child list
};


/*
 * structure of the resoure control block (RCB)
 */
struct rcb {
    int id;                                 // the unique identifier of resoure
    resource_status_t *status;              // the status of the resource
    wa_l_node_t **waiting_list;             // the list of blocked process
    //rcb_t *rl_next;                         // the next pointer for resource in resource list
};


/* 
 * structure of the status for pcb
 */
struct process_status {
    int type;                               // the process status: ready, run and block
    pcb_t **list;                           // the list process belong to: BL or RL
};


/*
 * structure of the status for rcb
 */
struct resource_status {
    int k;                                  // the initial number of resoure
    int u;                                  // the available number of resoure
};


/*
 * the structure of the creation_tree for pcb.
 */
struct creation_tree {
    pcb_t *parent;                          // the parent process
    pcb_t **child;                          // the child process list
};


/*
 * the structure of the node for resoure list in pcb.
 */
struct re_l_node {
    rcb_t *r;                               // the rcb
    int n;                                  // the numebr of the resoure
    re_l_node_t *next;
};


/*
 * the structure of the node for waiting list in rcb.
 */
struct wa_l_node {
    pcb_t *p;                               // the pcb
    int n;                                  // the number of the request resoure
    wa_l_node_t *next;
};


/******************************************************
 * manipluation for allocate and free memory for data structure
 *****************************************************/

/* get a new pcb for process */
pcb_t *get_new_pcb();

/* get target process by id */
pcb_t *get_pcb(int id);

/* delete the pcb */
void delete_pcb(pcb_t *p);

/* get the resoure descripter of target resoure */
rcb_t *get_rcb(int id);

/* for the node of resoure list in pcb */
re_l_node_t *get_new_re_l_node();
void delete_re_l_node(re_l_node_t *re_l_node);

/* for the node of waiting list in rcb */
wa_l_node_t *get_new_wa_l_node();
void delete_wa_l_node(wa_l_node_t *wa_l_node);


/******************************************************
 * manipluation for process control  
 *****************************************************/

/* establish a new process by id and priority */
pcb_t *create(const int id, int priority);

/* init the manager */
void init();

/* remove one or more process */
void destroy(int id);


/******************************************************
 * manipluation for resoure control  
 *****************************************************/

/* process request a certain number of resoures */
void request(int id, int num);

/* process release a certain number of resoures */
void release(int id, int num);


/******************************************************
 * manipluation for schedule and interrupt
 *****************************************************/

/* schedule process */
void scheduler();

/* clock interrupt */
void interrupt();


/******************************************************
 * manipluation for all kinds of linked list
 *****************************************************/

/* for the ready list in system */
void insert_into_ready_list(pcb_t **rl, pcb_t *p);
void remove_from_ready_list(pcb_t **rl, pcb_t *p);

/* for the blocked list in system */
void insert_into_blocked_list(pcb_t **bl, pcb_t *p);
void remove_from_blocked_list(pcb_t **bl, pcb_t *p);

/* for the child list in pcb */
void insert_into_child_list(pcb_t **cl, pcb_t *p);
void remove_from_child_list(pcb_t **cl, pcb_t *p);

/* for the resoure list in pcb */
void insert_into_resource_list(re_l_node_t **re_l, rcb_t *r, int n);
void remove_from_resource_list(re_l_node_t **re_l, rcb_t *r, int n);

/* for the waiting list in rcb */
void insert_into_waiting_list(wa_l_node_t **wa_l, pcb_t *p, int n);
void remove_from_waiting_list(wa_l_node_t **wa_l, pcb_t *p);

#endif
