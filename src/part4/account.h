#include <pthread.h>
#ifndef ACCOUNT_H_
#define ACCOUNT_H_

typedef struct
{
    char index[9];
	char account_number[17];
	char password[9];
    double balance;
    double p_balance;
    double reward_rate;
    int request;
    
    double transaction_tracter;
    double p_transaction_tracter;

    char out_file[64];
    char saving_file[64];

    pthread_mutex_t ac_lock;
}account;

#endif /* ACCOUNT_H_ */