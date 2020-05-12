#include <stdio.h>
#include <stdlib.h>

#include "manager.h"


static void search_creation_tree(int id, pcb_t *cur, pcb_t **target);
static void kill_tree(pcb_t *p);
static pcb_t *preempt(pcb_t *new_pcb, pcb_t *old_pcb);
static void insert_into_head_of_ready_list(pcb_t **rl, pcb_t *p);

static void release_all(pcb_t *p);

pcb_t *get_new_pcb() {
    return (pcb_t *) malloc(sizeof(pcb_t));
}

void delete_pcb(pcb_t *p) {
    free(p->status);
    free(p->creation_tree);
    free(p);
}

re_l_node_t *get_new_re_l_node() {
    return (re_l_node_t *) malloc(sizeof(re_l_node_t));
}

void delete_re_l_node(re_l_node_t *re_l_node) {
    free(re_l_node);
}

wa_l_node_t *get_new_wa_l_node() {
    return (wa_l_node_t *) malloc(sizeof(wa_l_node_t));
}

void delete_wa_l_node(wa_l_node_t *wa_l_node) {
    free(wa_l_node);
}

/* 
 * get the process with target id
 *              - search creation tree to get the target process
 */
pcb_t *get_pcb(int id) {
    pcb_t *ret = NULL;
   
    // search the creation tree from the init process
    search_creation_tree(id, process_init,  &ret);

    return ret;
}

/* 
 * search_creation_tree - the helper function of get_pcb(id) 
 */
static void search_creation_tree(int id, pcb_t *cur, pcb_t **ret) {
    if (cur == NULL) return;
    if (*ret != NULL) return;   // cut branch of subtree

    if (cur->id == id) {
        *ret = cur;         // search succeed
        return;
    }

    pcb_t *tmp = *(cur->creation_tree->child);
    while (tmp != NULL) {
        search_creation_tree(id, tmp, ret);
        tmp = tmp->cl_next;
    } 
}


static void init_resouces() {
    // init resource 1
    resource_1 = (rcb_t *) malloc(sizeof(rcb_t));
    resource_1->id = ID_RESOURE_1 ;
    resource_1->status = (resource_status_t *) malloc(sizeof(resource_status_t));
    resource_1->status->k = NUM_RESOURE_1;
    resource_1->status->u = NUM_RESOURE_1;
    resource_1->waiting_list = calloc(1, sizeof(wa_l_node_t *));

    // init resource 2
    resource_2 = (rcb_t *) malloc(sizeof(rcb_t));
    resource_2->id = ID_RESOURE_2;
    resource_2->status = (resource_status_t *) malloc(sizeof(resource_status_t));
    resource_2->status->k = NUM_RESOURE_2;
    resource_2->status->u = NUM_RESOURE_2;
    resource_2->waiting_list = calloc(1, sizeof(wa_l_node_t *));

    // init resource 3
    resource_3 = (rcb_t *) malloc(sizeof(rcb_t));
    resource_3->id = ID_RESOURE_3;
    resource_3->status = (resource_status_t *) malloc(sizeof(resource_status_t));
    resource_3->status->k = NUM_RESOURE_3;
    resource_3->status->u = NUM_RESOURE_3;
    resource_3->waiting_list = calloc(1, sizeof(wa_l_node_t *));

    // init resource 4
    resource_4 = (rcb_t *) malloc(sizeof(rcb_t));
    resource_4->id = ID_RESOURE_4;
    resource_4->status = (resource_status_t *) malloc(sizeof(resource_status_t));
    resource_4->status->k = NUM_RESOURE_4;
    resource_4->status->u = NUM_RESOURE_4;
    resource_4->waiting_list = calloc(1, sizeof(wa_l_node_t *));

}

rcb_t *get_rcb(int id) {
    if (id == ID_RESOURE_1) {
        return resource_1;
    } else if (id == ID_RESOURE_2) {
        return resource_2;
    } else if (id == ID_RESOURE_3) {
        return resource_3;
    } else {
        return resource_4;
    }
}


/*
 * request - currently running process request a certain numebr of resources
 */
void request(int id, int n) {
    rcb_t *r = get_rcb(id);
    if (r->status->u >= n) {     // allocate
        r->status->u -= n;
        insert_into_resource_list(self->resource_list, r, n);
    } else {                    // block
        if (n > r->status->k) {
            fprintf(stderr, "request resource error\n");
            exit(EXIT_FAILURE);
        } 
        self->status->type = STATUS_BLOCKED;
        self->status->list = &bl;
        remove_from_ready_list(&rl, self);            // schedule
        self->rl_next = NULL;
        insert_into_blocked_list(&bl, self);
        insert_into_waiting_list(r->waiting_list, self, n);

        scheduler();           // schedule
    }
}


/*
 * release - currently running process release a certain numebr of resources
 */
void release(int id, int n) {
    rcb_t *r = get_rcb(id);
    
    r->status->u += n;          // deallocate
    remove_from_resource_list(self->resource_list, r, n);
    
    // try to awake blocked processes in its waiting list.
    wa_l_node_t *tmp = *r->waiting_list; 
    while (tmp != NULL) {
        int req_num = tmp->n;

        if (r->status->u >= req_num) {      // unblock
            pcb_t *p = tmp->p;

            // turn the process from blocked to ready.
            remove_from_blocked_list(&bl, p);
            p->bl_next = NULL;
            p->status->type = STATUS_READY;
            p->status->list = &rl;
            insert_into_ready_list(&rl, p);
 
            // allocate the numebr of resource to the unblocked process
            r->status->u -= req_num;           
            insert_into_resource_list(p->resource_list, r, req_num);
            remove_from_waiting_list(r->waiting_list, p);
        } 
        tmp = tmp->next;
    }

    scheduler();
}


/*
 * init - create the process and resource manager
 */
void init() {

    process_init = NULL;
    self = NULL;
    rl = NULL;
    bl = NULL;

    // init four kinds of resources
    init_resouces();

    // create the init process 
    process_init = create(1, 0);
}


/*
 * create - create a new process 
 */
pcb_t *create(int id, int priority) {

    // create a new pcb and initialize the pcb
    pcb_t *p = get_new_pcb();

    p->id = id;

    p->resource_list = calloc(1, sizeof(re_l_node_t *));

    p->status = (process_status_t *) malloc(sizeof(process_status_t));
    p->status->type = STATUS_READY;
    p->status->list = &rl;

    p->creation_tree = (creation_tree_t *) malloc(sizeof(creation_tree_t));
    p->creation_tree->parent = self;
    p->creation_tree->child = calloc(1, sizeof(pcb_t *));

    p->priority = priority;

    p->rl_next = NULL;
    p->bl_next = NULL;
    p->cl_next = NULL;
   
    // insert the process into the child list
    if (self != NULL) insert_into_child_list(self->creation_tree->child, p);    

    // insert the process into the ready list
    insert_into_ready_list(&rl, p);

    // schedule if needed
    scheduler();

    return p;
}


/*
 * destroy - destroy the target process with its all descendants.
 */
void destroy(int id) {
    // get the target process
    pcb_t *p = get_pcb(id);
    if (p == NULL) return;    
    
    // remove itself from the child list of its parent process
    remove_from_child_list(p->creation_tree->parent->creation_tree->child, p);

    // kill itself and all its descendants
    kill_tree(p);
        
    // schedule if needed
    scheduler();
}

static void kill_tree(pcb_t *p) {
    if (p == NULL) return;

    // kill its descendants recursivly
    pcb_t *tmp = *(p->creation_tree->child); 
    while (tmp != NULL) {
        kill_tree(tmp);
        tmp = tmp->cl_next;
    }

    // remove itself from all possible list.
    if (p->status->type == STATUS_RUNNING) {
        interrupt();
    } else if (p->status->type == STATUS_READY) {
        remove_from_ready_list(&rl, p); 
    } else if (p->status->type == STATUS_BLOCKED) {
        remove_from_blocked_list(&bl, p);
    }

    // release all its resource
    release_all(p);

    // delete the pcb in memory
    delete_pcb(p); 
}

/*
 * release_all - release all resources the process hold on
 */
static void release_all(pcb_t *p) {
    re_l_node_t *tmp = *(p->resource_list);
    while (tmp != NULL) {
        tmp->r->status->u += tmp->n;
        wa_l_node_t *tmp2 = *(tmp->r->waiting_list);
        while (tmp2 != NULL) {
            int req_num = tmp2->n;
            if (tmp->r->status->u >= req_num) {
                remove_from_blocked_list(&bl, tmp2->p);
                tmp2->p->bl_next = NULL;
                tmp2->p->status->type = STATUS_READY;
                tmp2->p->status->list = &rl;
                insert_into_ready_list(&rl, tmp2->p);
                tmp->r->status->u -= req_num;
                insert_into_resource_list(tmp2->p->resource_list, tmp->r, req_num); 
                remove_from_waiting_list(tmp->r->waiting_list, tmp2->p);
            }
            tmp2 = tmp2->next;
        } 
        scheduler();
        tmp = tmp->next;
    }
}


/*
 * scheduler 
 */
void scheduler() {

    // get highest priority process
    pcb_t *p;
    if (self != NULL && rl->id == self->id) {   // self is in ready list
        if (rl->rl_next == NULL) return; // only one running process
        else p = rl->rl_next;
    } else {    // self is not in ready list
        p = rl;
    }

    // be preempted
    if ( self == NULL ||    // called from init or destroy 
            self->status->type != STATUS_RUNNING ||    // called from request or interrupt
            self->priority < p->priority    // called from create or release
    ) {

        if (p != NULL) self = preempt(p, self);        // self is preempted by p
    } 

}

/*
 * preempt 
 * 
 *      old_pcb: running -> ready
 *
 *      new_pcb: ready -> running
 */
static pcb_t *preempt(pcb_t *new_pcb, pcb_t *old_pcb) {

    if (old_pcb != NULL && old_pcb->status->type != STATUS_BLOCKED) { // old_pcb exist and not block
        old_pcb->status->type = STATUS_READY;
        remove_from_ready_list(&rl, old_pcb);           // remove old_pcb from rl
        old_pcb->rl_next = NULL;
        insert_into_ready_list(&rl, old_pcb);           // insert old_pcb into the end of corrsponding priority level in rl
    }  

    new_pcb->status->type = STATUS_RUNNING;
    remove_from_ready_list(&rl, new_pcb);           // remove new_pcb from rl
    new_pcb->rl_next = NULL;
    insert_into_head_of_ready_list(&rl, new_pcb);   // insert new_pcb at the head of rl, indicate that the new process is running

    return new_pcb;
}

/*
 * interrupt - emulate the time-out interrupt
 *       currently running process:   running -> ready
 */
void interrupt() {

    /*remove_from_ready_list(&rl, self);*/
    self->status->type = STATUS_READY;
    /*insert_into_ready_list(&rl, self);*/

    scheduler();
}

void insert_into_ready_list(pcb_t **rl, pcb_t *p) {
    if (p == NULL) return;

    if (*rl == NULL) {
        *rl = p;
    } else {
        pcb_t *tmp = *rl;
        while (tmp != NULL) {
            // insert at the end of ready list.
            if (tmp->rl_next == NULL) {
                tmp->rl_next = p;
                return;
            }
            // insert at the end of corrsponding priority level
            if (tmp->rl_next->priority < p->priority) {
                p->rl_next = tmp->rl_next;
                tmp->rl_next = p;
                return;
            }
            tmp = tmp->rl_next;
        }
    }
}

static void insert_into_head_of_ready_list(pcb_t **rl, pcb_t *p) {
    if (p == NULL) return;
    if (rl == NULL) {
        *rl = p;            // update rl
    } else {
        p->rl_next = *rl;   // insert 
        *rl = p;            // update rl
    }
}

void remove_from_ready_list(pcb_t **rl, pcb_t *p) {
    if (p == NULL) return;
    if (*rl == NULL) return;

    // remove from the head of ready list.
    if (*rl == p) {
        *rl = (*rl)->rl_next;
        return;
    }
    // remove from the middle or the end of ready list.
    pcb_t *tmp = *rl;
    while (tmp != NULL) {
        if (tmp->rl_next == p) {
            tmp->rl_next = tmp->rl_next->rl_next;
        } 
        tmp = tmp->rl_next;
    }
}

void insert_into_child_list(pcb_t **cl, pcb_t *p) {
    if (p == NULL) return;

    if (*cl == NULL) {
        *cl = p;
    } else {
        // insert at the end of child list
        pcb_t *tmp = *cl;
        while (tmp->cl_next != NULL) {
            tmp = tmp->cl_next; 
        }
        tmp->cl_next = p;
    }
}

void remove_from_child_list(pcb_t **cl, pcb_t *p) {
    if (p == NULL) return;
    if (cl == NULL) return;
    
    pcb_t *tmp = *cl;
    // remove from the head of child list.
    if (tmp == p) {
        *cl = tmp->cl_next;
        return;
    }
    // remove from the middle or the end of child list.
    while (tmp->cl_next != NULL) {
        if (tmp->cl_next == p) {
            tmp->cl_next = tmp->cl_next->cl_next;
        }
        tmp = tmp->cl_next;
    }
}


void insert_into_blocked_list(pcb_t **bl, pcb_t *p) {
    if (p == NULL) return;

    if (*bl == NULL) {
        *bl = p;
    } else {
        // insert at the end of blocked list
        pcb_t *tmp = *bl;
        while (tmp->bl_next != NULL) {
            tmp = tmp->bl_next;
        }
        tmp->bl_next = p;
    }
}

void remove_from_blocked_list(pcb_t **bl, pcb_t *p) {
    if (p == NULL) return;
    if (*bl == NULL) return;

    // remove from the head of blocked list.
    if (*bl == p) {
        *bl = (*bl)->bl_next;
        return;
    }
    // remove from the middle or the end of blocked list.
    pcb_t *tmp = *bl;
    while (tmp != NULL) {
        if (tmp->bl_next == p) {
            tmp->bl_next = tmp->bl_next->bl_next;
        } 
        tmp = tmp->bl_next;
    }
    
}

void insert_into_waiting_list(wa_l_node_t **wa_l, pcb_t *p, int n) {
    if (p == NULL) return;
    
    wa_l_node_t *node_to_insert = get_new_wa_l_node();
    node_to_insert->p = p;
    node_to_insert->n = n;
    node_to_insert->next = NULL;

    if (*wa_l == NULL) {
        *wa_l = node_to_insert; 
    } else {
        wa_l_node_t *tmp = *wa_l;
        while (tmp->next != NULL) 
            tmp = tmp->next;
        tmp->next = node_to_insert;           // insert at the end of waiting list
    }
}

void remove_from_waiting_list(wa_l_node_t **wa_l, pcb_t *p) {
    if (p == NULL) return;
    if (*wa_l == NULL) return;

    wa_l_node_t *tmp = *wa_l;
    if (tmp->p == p) {          // remove from the head
        *wa_l = tmp->next;
        delete_wa_l_node(tmp);
        return;
    }
    while (tmp->next != NULL) { // remove from the middle or end
        if (tmp->next->p == p) {
            wa_l_node_t *node_to_remove = tmp->next;
            tmp->next = node_to_remove->next;   
            delete_wa_l_node(node_to_remove);
            return;
        }
        tmp = tmp->next;
    }
}

/*
 * insert_into_resource_list - FIFO
 */
void insert_into_resource_list(re_l_node_t **re_l, rcb_t *r, int n) {
    if (r == NULL) return;
    
    // create and initialize new node in resource list
    re_l_node_t *node_to_insert = get_new_re_l_node();
    node_to_insert->r = r;
    node_to_insert->n = n;
    node_to_insert->next = NULL;

    if (*re_l == NULL) {
        *re_l = node_to_insert ;
    } else {
        re_l_node_t *tmp = *re_l;
        while (tmp != NULL) {
            if (tmp->r == r) {
                tmp->n += n; 
                return;
            }
            if (tmp->next == NULL) {
                tmp->next = node_to_insert;
                return;
            } 
            tmp = tmp->next;
        }
    }
}

void remove_from_resource_list(re_l_node_t **re_l, rcb_t *r, int n) {
    if (r == NULL) return;
    if (*re_l == NULL) return;

    re_l_node_t *tmp = *re_l;
    if (tmp->r == r) {          // remove from the head
        *re_l = tmp->next;
        delete_re_l_node(tmp);
        return;
    }
    while (tmp->next != NULL) {     // remove from the middle or end
        if (tmp->next->r == r) {
            if (n < tmp->next->n) {
                tmp->next->n -= n;      // resume amount
            } else if (n == tmp->next->n) {
                re_l_node_t *node_to_remove = tmp->next;
                tmp->next = node_to_remove->next;    
                delete_re_l_node(node_to_remove);
            } 
            return;
        }
        tmp = tmp->next;
    }
}
