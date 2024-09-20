
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include "account.h"
#include "string_parser.h"

void* process_transaction(void* arg);
void* update_balance(void* arg);
void update_acct();
void readLine1();
void readLine2();
void initAccount();
int checkBalance();
int deposit();
int withdraw();
int transferFunds();

command_line* tokens;
account* accounts;
int acct_num = 0;
int count = 0;
int pack = 0;
pthread_mutex_t request_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t request_condition = PTHREAD_COND_INITIALIZER;
pthread_barrier_t barrier;







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
    accounts =  malloc(sizeof(account) * acct_num);
    char* dir = "Output";
	int makeDir = mkdir(dir, 0777);
    pthread_t worker_threads[10];
    pthread_t bank_thread;


    for(int i = 0; i < acct_num; i++){
        initAccount(i, file, &accounts[i], len);
        //printf("Account %d Balance: %.2lf\n", i, accounts[i].balance);
        }
    
    tokens = malloc(sizeof(command_line) * 20000);
    
    count = 0;
    while((getline(&line, &len, file)) != -1){
        tokens[count] = str_filler(line, " ");
        count++;
    }

    pack = count / 10;

    for(int i = 0; i < 10; i++){
        pthread_create(&worker_threads[i], NULL, process_transaction, (void*)(intptr_t)i);
    }
    
    for (int i = 0; i < 10; i++) {
        pthread_join(worker_threads[i], NULL);
    }

    pthread_create(&bank_thread, NULL, &update_balance, NULL);
    pthread_join(bank_thread, NULL);



    FILE *fp = fopen("output/output.txt", "w");
    for(int i = 0; i < acct_num; i++){
		fprintf(fp, "%d balance:  %0.2f\n", i, accounts[i].balance);
        fprintf(fp, "\n");
	}
    fclose(fp);

    free(line);
    free(accounts);

    fclose(file);

    return 0;

}

void* process_transaction(void* arg){
    int index = (int)(intptr_t)arg;
    int start_index = index * pack;
    int end_index = start_index + pack;
    if (index == 9){
        end_index = count;
    }
    for(int i = start_index; i < end_index; i++){
        command_line token = tokens[i];
        char* command = token.command_list[0];
        double amount = 0.0;
        if(strcmp(command, "C") == 0){
            checkBalance(token.command_list[1], token.command_list[2]);
        }else if(strcmp(command, "D") == 0){
            amount = atof(token.command_list[3]);
            deposit(token.command_list[1], token.command_list[2], amount);
        }else if(strcmp(command, "T") == 0){
            amount = atof(token.command_list[4]);
            transferFunds(token.command_list[1], token.command_list[2], token.command_list[3], amount);
        }else if(strcmp(command, "W") == 0){
            amount = atof(token.command_list[3]);
            withdraw(token.command_list[1], token.command_list[2], amount);
        }

    }

    return 0;

}

void readLine1(FILE *fp, char *field, size_t size) {
    char *line = (char*)malloc(size);
    getline(&line, &size, fp);
    line[strcspn(line, "\n")] = 0;
    strcpy(field, line);
    free(line); 
}

void readLine2(FILE *fp, double *field, size_t size) {
    char *line = (char*)malloc(size);
    getline(&line, &size, fp);
    line[strcspn(line, "\n")] = 0;
    *field = atof(line);
    free(line);  
}

void initAccount(int i, FILE *fp, account *acc, size_t size) {
    readLine1(fp, acc->index, size);
    readLine1(fp, acc->account_number, size);
    readLine1(fp, acc->password, size);

    readLine2(fp, &acc->balance, size);
    readLine2(fp, &acc->reward_rate, size);
    acc->transaction_tracter = 0;
    
    pthread_mutex_init(&acc->ac_lock, NULL);
    sprintf(acc->out_file, "output/account%d.txt", i);
    FILE *o_fp = fopen(acc->out_file, "w");
    fprintf(o_fp, "account %d: \n", i);
    fclose(o_fp);
}


int checkBalance(char* acc, char* pass){
    for(int i = 0; i < acct_num; i++){
        if(strcmp(acc, accounts[i].account_number) == 0 && strcmp(pass, accounts[i].password) == 0){
            return 1;}
    }
    return 0;
}

int deposit(char*acc, char*pass, double amount){
    for(int i = 0; i < acct_num; i++){
        if(strcmp(acc, accounts[i].account_number) == 0 && strcmp(pass, accounts[i].password) == 0){
            pthread_mutex_lock(&accounts[i].ac_lock);
            accounts[i].transaction_tracter += amount;
            accounts[i].balance += amount;
            update_acct(accounts[i].out_file, accounts[i].balance);
            pthread_mutex_unlock(&accounts[i].ac_lock);
            return 1;}
    }
    return 0;

}

int withdraw(char*acc, char*pass, double amount){
    for(int i = 0; i < acct_num; i++){
        if(strcmp(acc, accounts[i].account_number) == 0 && strcmp(pass, accounts[i].password) == 0){
            pthread_mutex_lock(&accounts[i].ac_lock);
            accounts[i].transaction_tracter += amount;
            accounts[i].balance -= amount;
            update_acct(accounts[i].out_file, accounts[i].balance);
            pthread_mutex_unlock(&accounts[i].ac_lock);
            return 1;}
    }
    return 0;

}

int transferFunds(char*src_acc, char*pass, char* dst_acc, double amount){
    int src_index = -1;
    int dst_index = -1;

    for (int i = 0; i < acct_num; i++) {
        if (strcmp(src_acc, accounts[i].account_number) == 0 && strcmp(pass, accounts[i].password) == 0) {
            src_index = i;
            break;}
    }

    for (int j = 0; j < acct_num; j++) {
        if (strcmp(dst_acc, accounts[j].account_number) == 0) {
            dst_index = j;
            break;}
    }
    if (src_index != -1 && dst_index != -1) {
        pthread_mutex_lock(&accounts[src_index].ac_lock);
        accounts[src_index].transaction_tracter += amount;
        accounts[src_index].balance -= amount;
        update_acct(accounts[src_index].out_file, accounts[src_index].balance);
        pthread_mutex_unlock(&accounts[src_index].ac_lock);

        pthread_mutex_lock(&accounts[dst_index].ac_lock);
        accounts[dst_index].balance += amount;
        update_acct(accounts[dst_index].out_file, accounts[dst_index].balance);
        pthread_mutex_unlock(&accounts[dst_index].ac_lock);
        return 1;}

    return 0;
}

void update_acct(char* path, double amount){
    FILE *fp = fopen(path, "a");
    fprintf(fp, "Current Balance: %.2lf\n", amount);
    fclose(fp);
}
void* update_balance(void*arg){
    int update_count = 0;
	for(int i = 0; i < acct_num; i++){
        pthread_mutex_lock(&accounts[i].ac_lock);
		accounts[i].balance += accounts[i].reward_rate * accounts[i].transaction_tracter;
        update_count++;
        pthread_mutex_unlock(&accounts[i].ac_lock);
	}
    
    return (void*)(intptr_t)update_count;
	
}