/*
 *
 * shell - to present the process and resource manager.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "manager.h"

#define BUFSIZE 16

static FILE *in;                // input file stream: from stdin or from file.
static int buf_size;            // the size of p_id_to_name map 
static int base_id;             // the available process id for creating process in system
static char **p_id_to_name;     // map from process id to process name
// functions for running shell
static void run_loop();
static char *read_line();
static char **split_line(char *);
static void execute_cmd(char **);
// functions for executing command
static void it(char **);
static void cr(char **);
static void de(char **);
static void req(char **);
static void rel(char **);
static void to(char **);
static void list(char **);
static void pr(char **);
static void help();

/*
 * it - init
 *      - initilize the basic process and resource manager
 */
static void it(char **args) {
    if (args[1] == NULL) {
        init();         // run
        buf_size = BUFSIZE; 
        p_id_to_name = malloc (BUFSIZE * sizeof(char *));   // initalize pid_to_pname map
        p_id_to_name[process_init->id] = strdup("init"); 
        base_id = process_init->id + 1;     // update base_id
        printf("the process and resource manager is initilized\n");
        printf("init process is running\n");
    } else {
        fprintf(stderr, "Usage: %s\n", args[0]);
    }
}

/*
 * cr - cr <process name> <priority>
 *      - create a new process with corresponding priority
 */
static void cr(char **args) {
    if (args[1] == NULL || args[2] == NULL || args[3] != NULL) {
        fprintf(stderr, "Usage: %s <process name> <priority>\n", args[0]);
    } else {
        for (int i = 2; i < base_id; i++) { // check if unique
            if (p_id_to_name[i] == NULL) continue;
            if (strcmp(args[1], p_id_to_name[i]) == 0) {
                fprintf(stderr, "process name is existed\n");
                return;
            }
        }
        int priority = atoi(args[2]);
        if (base_id >= buf_size) {      // expand capacity of map
            buf_size *= 2;
            p_id_to_name = realloc(p_id_to_name, buf_size);
        }
        create(base_id, priority);  // run
        p_id_to_name[base_id++] = strdup(args[1]);  
        printf("process %s is created\n", args[1]);
        printf("process %s is running\n", p_id_to_name[self->id]);
    }
}

/* 
 * de - de <process name>  
 *      - delete the target process
 */
static void de(char **args) {
    if (args[1] == NULL || args[2] != NULL) {
        fprintf(stderr, "Usage: %s <process name>\n", args[0]);
    } else {
        if (base_id >= buf_size) {      // expand capacity
            buf_size += buf_size;
            p_id_to_name = realloc(p_id_to_name, buf_size);
        }
        for (int i = 2; i < base_id; i++) {     // search for target process
            if (p_id_to_name[i] == NULL) continue;
            if (strcmp(args[1], p_id_to_name[i]) == 0) {
                destroy(i);     // run
                p_id_to_name[i] = NULL; 
                printf("process %s is deleted\n", args[1]);
                printf("process %s is running\n", p_id_to_name[self->id]);
                return;
            }
        }
        fprintf(stderr, "process %s is not exist\n", args[1]);
    }
}

/*
 * req - req <resource name> <request number>
 *      - currently running process request a certain number of resources
 */
static void req(char **args) {
    if (args[1] == NULL || args[3] != NULL) {
        fprintf(stderr, "Usage: %s <resource name> <# of units>\n", args[0]);
    } else {
        int num;
        char *before = p_id_to_name[self->id];
        if (args[2] == NULL) num = 1;
        else num = atoi(args[2]);
        if (strcmp(args[1], "R1") == 0) {
            request(ID_RESOURE_1, num);         // run
        } else if (strcmp(args[1], "R2") == 0) {
            request(ID_RESOURE_2, num);
        } else if (strcmp(args[1], "R3") == 0) {
            request(ID_RESOURE_3, num);
        } else if (strcmp(args[1], "R4") == 0) {
            request(ID_RESOURE_4, num);
        } else {
            fprintf(stderr, "the resource %s is not exist\n", args[1]);
            return;
        }
        printf("process %s request resource %d %s\n", before, num, args[1]);
        if (strcmp(before, p_id_to_name[self->id]) != 0) {  // blocked 
            printf("process %s is running, process %s is blocked\n", p_id_to_name[self->id], before);
        }
    }
}

/*
 * rel - rel <resource name> <release number>
 *      - currently running process release a certain number of resources
 */
static void rel(char **args) {
    if (args[1] == NULL || args[3] != NULL) {
        fprintf(stderr, "Usage: %s <resource name> <# of units>\n", args[0]);
    } else {
        int num;
        if (args[2] == NULL) num = 1;
        else num = atoi(args[2]);
        if (strcmp(args[1], "R1") == 0) {
            release(ID_RESOURE_1, num);
        } else if (strcmp(args[1], "R2") == 0) {
            release(ID_RESOURE_2, num);
        } else if (strcmp(args[1], "R3") == 0) {
            release(ID_RESOURE_3, num);
        } else if (strcmp(args[1], "R4") == 0) {
            release(ID_RESOURE_4, num);
        } else {
            fprintf(stderr, "the resource %s is not exist\n", args[1]);
            return;
        }
        printf("process %s release resource %d %s\n", p_id_to_name[self->id], num, args[1]);
    }
}

/*
 * to - to
 *      - time-out interrupt
 *
 */
static void to(char **args) {
    if (args[1] != NULL) {
        fprintf(stderr, "Usage: %s\n", args[0]);
    } else {
        printf("process %s is ready\n", p_id_to_name[self->id]);
        interrupt();        // run
        printf("process %s is running\n", p_id_to_name[self->id]);
    }
}

/*
 * list - list (ready, block, res)
 *              - present the process list.
 */
static void list(char **args) {
    if (args[1] == NULL || args[2] != NULL) {
        fprintf(stderr, "Usage: list <ready,block,res>\n");
        return;
    }
    // ready list
    if (strcmp(args[1], "ready") == 0) {
        pcb_t *tmp;
        printf("ready list with priority %d:", PRIORITY_SYSTEM);
        tmp = rl;
        while (tmp != NULL) {
            if (tmp->priority == PRIORITY_SYSTEM && tmp->status->type == STATUS_READY) {
                printf("-> %s ", p_id_to_name[tmp->id]); 
            } 
            tmp = tmp->rl_next;
        }
        printf("\n");
        printf("ready list with priority %d:", PRIORITY_USER);
        tmp = rl;
        while (tmp != NULL) {
            if (tmp->priority == PRIORITY_USER && tmp->status->type == STATUS_READY) {
                printf("-> %s ", p_id_to_name[tmp->id]); 
            } 
            tmp = tmp->rl_next;
        }
        printf("\n");
        printf("ready list with priority %d:", PRIORITY_INIT);
        tmp = rl;
        while (tmp != NULL) {
            if (tmp->priority == PRIORITY_INIT && tmp->status->type == STATUS_READY) {
                printf("-> %s ", p_id_to_name[tmp->id]); 
            } 
            tmp = tmp->rl_next;
        }
        printf("\n");
    } 
    // block list
    else if (strcmp(args[1], "block") == 0) {
        wa_l_node_t *tmp;
        printf("process blocked in resource R1:");
        tmp = *get_rcb(ID_RESOURE_1)->waiting_list;
        while (tmp != NULL) {
            printf("-> %s ", p_id_to_name[tmp->p->id]);
            tmp = tmp->next;
        }
        printf("\n");
        printf("process blocked in resource R2:");
        tmp = *get_rcb(ID_RESOURE_2)->waiting_list;
        while (tmp != NULL) {
            printf("-> %s ", p_id_to_name[tmp->p->id]);
            tmp = tmp->next;
        }
        printf("\n");
        printf("process blocked in resource R3:");
        tmp = *get_rcb(ID_RESOURE_3)->waiting_list;
        while (tmp != NULL) {
            printf("-> %s ", p_id_to_name[tmp->p->id]);
            tmp = tmp->next;
        }
        printf("\n");
        printf("process blocked in resource R4:");
        tmp = *get_rcb(ID_RESOURE_4)->waiting_list;
        while (tmp != NULL) {
            printf("-> %s ", p_id_to_name[tmp->p->id]);
            tmp = tmp->next;
        }
        printf("\n");
    }
    // resource list
    else if (strcmp(args[1], "res") == 0) {
        printf("resource R1 remain: %d\n", get_rcb(ID_RESOURE_1)->status->u);
        printf("resource R2 remain: %d\n", get_rcb(ID_RESOURE_2)->status->u);
        printf("resource R3 remain: %d\n", get_rcb(ID_RESOURE_3)->status->u);
        printf("resource R4 remain: %d\n", get_rcb(ID_RESOURE_4)->status->u);
    } else {
        fprintf(stderr, "Usage: list <ready,block,res>\n");
    }
}


/*
 * pr - pr <process name>
 *      - to present the pcb informantion about the process
 */
static void pr(char **args) {
    if (args[1] == NULL || args[2] != NULL) {
        fprintf(stderr, "Usage: %s <process name>\n", args[0]);
    }
    pcb_t *p;
    for (int i = 1; i < base_id; i++) {     // search for the target process 
        if (p_id_to_name[i] == NULL) continue;
        if (strcmp(args[1], p_id_to_name[i]) == 0) {
            p = get_pcb(i);
            break;
        }
    } 
    if (p == NULL) {
        fprintf(stderr, "process %s is not exist\n", args[1]);
        return;
    }
    // print name 
    printf("process name: %s\n", p_id_to_name[p->id]);
    // print id 
    printf("process id: %d\n", p->id);
    // print priority
    printf("process priority: %d\n", p->priority);
    // print resources
    re_l_node_t *tmp = *(p->resource_list);
    while (tmp != NULL) {
        printf("process hold resource R%d\n", tmp->r->id); 
        tmp = tmp->next;
    }
    // print status 
    printf("process status: ");
    if (p->status->type == STATUS_BLOCKED) {
        printf("BLOCKED\n"); 
    } else if (p->status->type == STATUS_READY) {
        printf("READY\n");
    } else if (p->status->type == STATUS_RUNNING){
        printf("RUNNING\n");
    }
    // print parent 
    if (p != process_init) printf("parent process: %s\n", p_id_to_name[p->creation_tree->parent->id]);
    // print child
    pcb_t *child = *(p->creation_tree->child);
    printf("child processes: ");
    while (child != NULL) {
        printf("- %s", p_id_to_name[child->id]); 
    }
    printf("\n");
}



/* 
 * read_line - read a line at once from input stream
 */
static char *read_line() {
    char *line = NULL;
    size_t bufsize = 0;
    ssize_t len = 0; 
    if ( (len = getline(&line, &bufsize, in)) == -1) {
        if (feof(in)) {
            exit(EXIT_SUCCESS);
        } else {
            fprintf(stderr, "read line error\n");
            exit(EXIT_FAILURE);
        }
    }
    if (line == NULL || len == 1) return NULL;
    else if (line[len - 1] == '\n') line[len - 1] = '\0';
    return line;
}

/*
 * split_line - split line to tokens which contains arguments of command.
 */
static char **split_line(char *line) {
    char *stringp = line;
    int position = 0;
    char **tokens = malloc(BUFSIZE * sizeof(char *));
    char *token;
    while ( (token = strsep(&stringp, " ")) != NULL ) {
        tokens[position++] = token;
    } 
    tokens[position] = NULL;
    return tokens;
}

/*
 * execute_cmd - used for invoke manager routines and print some information.
 */
static void execute_cmd(char **args) {
    if (args[0] == NULL) return;
    if (strcmp(args[0], "init") == 0) {
        char **tmp = malloc(BUFSIZE * sizeof(char *));
        it(tmp);
    } else if (strcmp(args[0], "cr") == 0) {
        cr(args); 
    } else if (strcmp(args[0], "de") == 0) {
        de(args);
    } else if (strcmp(args[0], "req") == 0) {
        req(args);
    } else if (strcmp(args[0], "rel") == 0) {
        rel(args);
    } else if (strcmp(args[0], "to") == 0) {
        to(args);
    } else if (strcmp(args[0], "list") == 0) {
        list(args); 
    } else if (strcmp(args[0], "pr") == 0) {
        pr(args);
    } else if (strcmp(args[0], "exit") == 0) {
        exit(EXIT_SUCCESS);
    } else if (strcmp(args[0], "help") == 0) {
        help();      
    } else {
       printf("can't find %s in commands library of the shell, try type in \"help\"\n", args[0]);
    }
}

/*
 * help - print some help information
 */
static void help() {
    // print help information
    printf("commands of presentation shell:\n");
    printf("- init                              // initilize manager\n");
    printf("- cr <process name> <priority>      // create process\n");
    printf("- de <process name>                 // delete process\n");
    printf("- req <resource> <# of units>       // request resource\n");
    printf("- rel <resource name> <# of units>  // release resource\n");
    printf("- to                                // time-out interrupt\n");
    printf("- list ready                        // list all process in ready list\n");
    printf("- list block                        // list all process in blocked list\n");
    printf("- list res                          // list all available resources\n");
    printf("- pr <process name>                 // print pcb information about a given process\n");
    printf("- exit                              // exit the presentation shell\n");
    printf("note: the argument <# of units> is optional, the default is 1\n");
}


/*
 * run_loop - the structure of the working shell.
 */
static void run_loop() {
    char *line;
    char **args; 
    while (1) {
        if (in == stdin)
            printf("shell> ");
        // read line 
        line = read_line();
        if (line == NULL) continue;
        // split line
        args = split_line(line);
        if (args == NULL) continue;
       // execute command
        execute_cmd(args);
        free(line);
        free(args);
    }
}



/*
 *
 * the entrance of program
 *
 */
int main(int argc, char *argv[]) {
    // handle input stream
    if (argc == 1) {
        // from stdin
        in = stdin;
    } else if (argc == 2) {
        // from file
        if ((in = fopen(argv[1], "r")) == NULL) {
            fprintf(stderr, "open file error\n");
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "Usage: ./shell <input file>(optional)\n");
        exit(EXIT_FAILURE);
    }
    // run command loop
    run_loop();
    exit(EXIT_SUCCESS);
}
