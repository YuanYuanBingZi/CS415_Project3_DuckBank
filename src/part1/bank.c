#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "account.h"
#include "string_parser.h"
#include <pthread.h>
#include <sys/stat.h>

void process_transaction();
void update_balance();
void update_acct();
void readLine1();
void readLine2();
void initAccount();
int checkBalance();
int deposit();
int withdraw();
int transferFunds();


command_line tokens;
account* accounts;
int acct_num = 0;

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
    char *line = (char*)malloc(len);
    getline(&line, &len, file);
    acct_num = atoi(line);
    accounts =  malloc(sizeof(account) * acct_num);
    char* dir = "Output";
	int makeDir = mkdir(dir, 0777);


    for(int i = 0; i < acct_num; i++){
        initAccount(i, file, &accounts[i], len);
        //printf("Account %d Balance: %.2lf\n", i, accounts[i].balance);
        }
    
    process_transaction(argv);
    update_balance();

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

void process_transaction(char** argv){
    size_t size = 128;
    char* line = (char*) malloc(size);
    FILE *fp = fopen(argv[1], "r");
    while(getline(&line, &size, fp) != -1){
        tokens = str_filler(line, " ");
        char* command = tokens.command_list[0];
        double amount = 0.0;
        if(strcmp(command, "C") == 0){
            checkBalance(tokens.command_list[1], tokens.command_list[2]);
        }else if(strcmp(command, "D") == 0){
            amount = atof(tokens.command_list[3]);
            deposit(tokens.command_list[1], tokens.command_list[2], amount);
        }else if(strcmp(command, "T") == 0){
            amount = atof(tokens.command_list[4]);
            transferFunds(tokens.command_list[1], tokens.command_list[2], tokens.command_list[3], amount);
        }else if(strcmp(command, "W") == 0){
            amount = atof(tokens.command_list[3]);
            withdraw(tokens.command_list[1], tokens.command_list[2], amount);
        }

    }

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
            accounts[i].transaction_tracter += amount;
            accounts[i].balance += amount;
            update_acct(accounts[i].out_file, accounts[i].balance);
            return 1;}
    }
    return 0;

}

int withdraw(char*acc, char*pass, double amount){
    for(int i = 0; i < acct_num; i++){
        if(strcmp(acc, accounts[i].account_number) == 0 && strcmp(pass, accounts[i].password) == 0){
            accounts[i].transaction_tracter += amount;
            accounts[i].balance -= amount;
            update_acct(accounts[i].out_file, accounts[i].balance);
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
        accounts[src_index].transaction_tracter += amount;
        accounts[src_index].balance -= amount;
        accounts[dst_index].balance += amount;
        update_acct(accounts[src_index].out_file, accounts[src_index].balance);
        update_acct(accounts[dst_index].out_file, accounts[dst_index].balance);
        return 1;}

    return 0;
}

void update_acct(char* path, double amount){
    FILE *fp = fopen(path, "a");
    fprintf(fp, "Current Balance: %.2lf\n", amount);
    fclose(fp);
}
void update_balance(){
	for(int i = 0; i < acct_num; i++){
		accounts[i].balance += accounts[i].reward_rate * accounts[i].transaction_tracter;
	}
	
}