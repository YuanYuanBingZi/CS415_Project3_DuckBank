#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "account.h"
#include "string_parser.h"
#include <pthread.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#define  SHARED_MEMORY_NAME "happy"


void* process_transaction(void* arg);
void* update_balance(void* arg);
void puddle_bank();
void update_acct();
void readLine1();
void readLine2();
void initAccount();
int checkBalance();
int deposit();
int withdraw();
int transferFunds();



int count = 0;
int pid;
int shared_memory_size = 0;
int end = 0;
int last_call = 0;
int pack = 0;
command_line* tokens;
int acct_num = 0;
account *duck_accounts;
account *shared_accounts;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_barrier_t barrier;


int total_requests = 0;
int processed_requests = 0;
int thread_count = 0;
int update_count = 0;


int main(int argc, char*argv[]){
    if(argc != 2){
        printf("Error! Please type the program and input file\n");
        return 1;
    }


    FILE *file = fopen(argv[1], "r");
    if(file == NULL){
        printf("Error opening file");
        return 1;
    }
    
    size_t len = 128;
    char *line = NULL;
    size_t read;
    getline(&line, &len, file);
    acct_num = atoi(line);
    
    
    char* output_path = "output";
    DIR* output_dir = opendir(output_path);
    if(!output_dir){
        int makeDir = mkdir(output_path, 0777);
    }

    char* savings_path = "savings" ;
    DIR* savings_dir = opendir(savings_path);
    if(!savings_dir){
        int make_SavingsDir = mkdir(savings_path, 0777);
    }

    pthread_t worker_threads[10];
    pthread_t bank_thread;
    pthread_barrier_init(&barrier, NULL, 11);

    int fd = shm_open(SHARED_MEMORY_NAME, O_CREAT | O_RDWR, 0777);
    if (fd == -1){
        perror("Error! shm_open");
    }

    shared_memory_size= sizeof(account) * acct_num;
    if(ftruncate(fd, shared_memory_size) == -1){
        perror("Error at ftruncate!\n");
        close(fd);
    }

    shared_accounts = mmap(NULL, shared_memory_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared_accounts == MAP_FAILED){
        printf("Error at mmp in parent process.\n");
        close(fd);
    }    

    for(int i = 0; i < acct_num; i++){
        initAccount(i, file, &shared_accounts[i], len);
    }

    duck_accounts = malloc(shared_memory_size);

    memcpy(duck_accounts, shared_accounts, shared_memory_size);

    
    tokens = malloc(sizeof(command_line) * 200000);
    if (tokens == NULL){
        printf("Error creating tokens");
    }
    
    count = 0;
    while((getline(&line, &len, file)) != -1 && count < 200000){
        tokens[count] = str_filler(line, " ");
        count++;
    }
    //printf("count %d\n", count);

    pack = count / 10;
    //printf("pack %d\n", pack);


    pid_t child_pid;
    child_pid = fork();
    if(child_pid == -1){
        printf("Error at creating child process\n");
    }else if(child_pid == 0){
        pid = getpid();
        printf("Child process %d is running. \n", getpid());
        puddle_bank();
    }



    for(int i = 0; i < 10; i++){
        pthread_create(&worker_threads[i], NULL, process_transaction, (void*)(intptr_t)i);
    }

    pthread_create(&bank_thread, NULL, &update_balance, (void*)(intptr_t)pid);
    pthread_barrier_wait(&barrier);
    
   
    for (int i = 0; i < 10; i++) {
        pthread_join(worker_threads[i], NULL);
    }

    pthread_join(bank_thread, NULL);


    FILE *fp = fopen("output/output.txt", "w");
    for(int i = 0; i < acct_num; i++){
		fprintf(fp, "%d balance:  %0.2f\n", i, shared_accounts[i].balance);
        fprintf(fp, "\n");
	}
    fclose(fp);

    if(munmap(shared_accounts , shared_memory_size) == -1){
        printf("Error at munmap\n");
    }
    close(fd);

    fd = shm_unlink(SHARED_MEMORY_NAME);
    if(fd == -1){
        printf("unlink error!\n");
    }
    pthread_barrier_destroy(&barrier);
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);

    free_command_line(tokens);
    free(duck_accounts);

    fclose(file);

    return 0;

}

void* process_transaction(void* arg){
    pthread_barrier_wait(&barrier);
    int index = (int)(intptr_t)arg;
    int start_index = index * pack;
    int end_index = start_index + pack;
    if (index == 9){
        end_index = count;
    }
    int i = start_index;
    int flag = 0;
    while (i < end_index){
        command_line token = tokens[i];
        char* command = token.command_list[0];
        double amount = 0.0;
        if(strcmp(command, "C") == 0){
            checkBalance(token.command_list[1], token.command_list[2]);
        }else if(strcmp(command, "D") == 0){
            amount = atof(token.command_list[3]);
            deposit(token.command_list[1], token.command_list[2], amount);
            flag = 1;
        }else if(strcmp(command, "T") == 0){
            amount = atof(token.command_list[4]);
            transferFunds(token.command_list[1], token.command_list[2], token.command_list[3], amount);
            flag = 1;
        }else if(strcmp(command, "W") == 0){
            amount = atof(token.command_list[3]);
            withdraw(token.command_list[1], token.command_list[2], amount);
            flag = 1;
        }

        pthread_mutex_lock(&lock);
        total_requests += flag;
        if(total_requests == 4995){
            pthread_cond_broadcast(&cond);
        }
        while(total_requests >= 4995){
            pthread_cond_wait(&cond, &lock);
        }
        pthread_mutex_unlock(&lock);

        i += 1;
    }

    pthread_mutex_lock(&lock);
    thread_count++;
    pthread_mutex_unlock(&lock);
    //printf("%d\n", thread_count);
    if(thread_count == 10){
        last_call = 2;
        duck_accounts[0].request = 1;
        printf("the last call\n");
        pthread_cond_broadcast(&cond);
    }
        
}


void readLine1(FILE *fp, char *field, size_t size) {
    char *line = NULL;
    getline(&line, &size, fp);
    line[strcspn(line, "\n")] = 0;
    strcpy(field, line);
}

void readLine2(FILE *fp, double *field, size_t size) {
    char *line = NULL;
    getline(&line, &size, fp);
    line[strcspn(line, "\n")] = 0;
    *field = atof(line);  
}

void initAccount(int i, FILE *fp, account *acc, size_t size) {
    readLine1(fp, acc->index, size);
    readLine1(fp, acc->account_number, size);
    readLine1(fp, acc->password, size);

    readLine2(fp, &acc->balance, size);

    acc->p_balance = acc->balance * 0.2;
    readLine2(fp, &acc->reward_rate, size);
    acc->transaction_tracter = 0;
    acc->p_transaction_tracter = 0;
    acc->request = 0;
    
    pthread_mutex_init(&acc->ac_lock, NULL);
    sprintf(acc->out_file, "output/account%d.txt", i);
    sprintf(acc->saving_file,"savings/account%dsaving.txt", i);
    FILE *o_fp = fopen(acc->out_file, "w");
    if(o_fp == NULL){
        printf("Error opening output file\n");
        return;
    }
    fprintf(o_fp, "account %d: \n", i);
    fclose(o_fp);
    FILE *s_fp = fopen(acc->saving_file, "w");
    if(s_fp == NULL){
        printf("Error opening saving file\n");
        return;
    }
    fprintf(s_fp, "account %d: \n", i);
    fclose(s_fp);
}


int checkBalance(char* acc, char* pass){
    for(int i = 0; i < acct_num; i++){
        if(strcmp(acc, duck_accounts[i].account_number) == 0 && strcmp(pass, duck_accounts[i].password) == 0){
            return 1;}
    }
    return 0;
}

int deposit(char*acc, char*pass, double amount){
    for(int i = 0; i < acct_num; i++){
        if(strcmp(acc, duck_accounts[i].account_number) == 0 && strcmp(pass, duck_accounts[i].password) == 0){
            pthread_mutex_lock(&duck_accounts[i].ac_lock);
            duck_accounts[i].transaction_tracter += amount;
            duck_accounts[i].balance += amount;
            duck_accounts[i].p_transaction_tracter += amount;
            pthread_mutex_unlock(&duck_accounts[i].ac_lock);
            return 1;}
    }
    return 0;

}

int withdraw(char*acc, char*pass, double amount){
    for(int i = 0; i < acct_num; i++){
        if(strcmp(acc, duck_accounts[i].account_number) == 0 && strcmp(pass, duck_accounts[i].password) == 0){
            pthread_mutex_lock(&duck_accounts[i].ac_lock);
            duck_accounts[i].transaction_tracter += amount;
            duck_accounts[i].p_transaction_tracter += amount;
            duck_accounts[i].balance -= amount;
            pthread_mutex_unlock(&duck_accounts[i].ac_lock);
            return 1;}
    }
    return 0;

}

int transferFunds(char*src_acc, char*pass, char* dst_acc, double amount){
    int src_index = -1;
    int dst_index = -1;

    for (int i = 0; i < acct_num; i++) {
        if (strcmp(src_acc, duck_accounts[i].account_number) == 0 && strcmp(pass, duck_accounts[i].password) == 0) {
            src_index = i;
            break;}
    }

    for (int j = 0; j < acct_num; j++) {
        if (strcmp(dst_acc, duck_accounts[j].account_number) == 0) {
            dst_index = j;
            break;}
    }
    if (src_index != -1 && dst_index != -1) {
        pthread_mutex_lock(&duck_accounts[src_index].ac_lock);
        duck_accounts[src_index].transaction_tracter += amount;
        duck_accounts[src_index].p_transaction_tracter += amount;
        duck_accounts[src_index].balance -= amount;
        pthread_mutex_unlock(&duck_accounts[src_index].ac_lock);

        pthread_mutex_lock(&duck_accounts[dst_index].ac_lock);
        duck_accounts[dst_index].balance += amount;
        pthread_mutex_unlock(&duck_accounts[dst_index].ac_lock);
        return 1;}

    return 0;
}

void update_acct(char* path, double amount){
    FILE *fp = fopen(path, "a");
    fprintf(fp, "Current Balance: %.2lf\n", amount);
    fclose(fp);
}
void* update_balance(void*arg){
    while(processed_requests < count){
        pthread_mutex_lock(&lock);
        while(total_requests < 4995){
            if(last_call == 2) break;
            pthread_cond_wait(&cond, &lock);
        }

        printf("Updateing Balance......\n");
    
        for(int i = 0; i < acct_num; i++){
            pthread_mutex_lock(&duck_accounts[i].ac_lock);
            duck_accounts[i].balance += duck_accounts[i].reward_rate * duck_accounts[i].transaction_tracter;
            update_acct(duck_accounts[i].out_file, duck_accounts[i].balance);
            duck_accounts[i].transaction_tracter = 0;
            update_count++;
            pthread_mutex_unlock(&duck_accounts[i].ac_lock);
        }
        
       
        memcpy(shared_accounts, duck_accounts, shared_memory_size);
        
        kill(pid, SIGCONT);
        
        processed_requests += total_requests;
        total_requests = 0;
        
        printf("processed_request is %d!\n", processed_requests);
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&lock);
    }
    return (void*)(intptr_t)update_count;
}

void puddle_bank(){
    int fd = shm_open(SHARED_MEMORY_NAME, O_RDWR, 06666);
    if(fd == -1){
        perror("can't open the memory in puddle process.\n");}
    
    account* shared_accounts = mmap(NULL, shared_memory_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    
    if(shared_accounts == MAP_FAILED){
        perror("Error at mmap in puddle process.\n");
        close(fd);
    }
    
    while(1){
        printf("puddle bank is updating\n");
        for(int i = 0; i < acct_num; i++){
            shared_accounts[i].p_balance += 0.02 * shared_accounts[i].p_transaction_tracter;
            update_acct(shared_accounts[i].saving_file, shared_accounts[i].p_balance);
            shared_accounts[i].p_transaction_tracter = 0;
        }

        if(shared_accounts[0].request){
            printf("last call from puddle!\n");
            if(munmap(shared_accounts , shared_memory_size) == -1){
                perror("Error at munmap\n");}
            free(shared_accounts);
            close(fd);
            exit(0);

        }
        kill(getpid(), SIGSTOP);

        }

    }
        







