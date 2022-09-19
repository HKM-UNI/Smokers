#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdbool.h>

typedef enum cigarette_enum { PAPER, PHOSPHORS, TOBACCO } CigaretteMaterial;

typedef struct smoker_type {
    char name[10];
    CigaretteMaterial material;
} Smoker;

CigaretteMaterial offer1;
CigaretteMaterial offer2;
bool is_smooking = true;

pthread_t tid[3];
pthread_mutex_t lock;

char* offer_name(CigaretteMaterial offer) {
    switch (offer)
    {
        case PAPER: return "Paper"; break;
        case PHOSPHORS: return "Phospors"; break;
        case TOBACCO: return "Tobacco"; break;
        default: return "unknown"; break;
    }
}

void release_offers()
{
    offer1 = (CigaretteMaterial) (rand() % 3);
    offer2 = (CigaretteMaterial) (rand() % 3);
    while (offer1 == offer2) {
        offer2 = (CigaretteMaterial) (rand() % 3);
    }

    printf("New Materials: %s & %s\n", offer_name(offer1), offer_name(offer2));
    fflush(stdout);
    
    sleep(1);
}

void trade(Smoker *smoker)
{
    // Retorna si su material es igual al de uno de los ofertados o ya hay alguien fumando
    if (smoker->material == offer1 || smoker->material == offer2) {
        return;
    }

    printf("[%s] is now smoking...\n",smoker->name);
	fflush(stdout);
	sleep(3);
}

void* agent_loop(void* arg) {
    while (1) {
        if (is_smooking) {
            continue;
        }
        release_offers();
    }
    return NULL;
}

void* trade_loop(void* arg) {
    Smoker* smoker = (Smoker*) arg;
    while (1) {
        pthread_mutex_lock(&lock);
        is_smooking = true;
        trade(smoker);
        is_smooking = false;
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

int main(void)
{
    srand(time(NULL));
    int thread_result;

    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }

    Smoker smokers[3] = {
        { "Smoker 1", PAPER },
        { "Smoker 2", PHOSPHORS },
        { "Smoker 3", TOBACCO }
    };

    pthread_t agent_thread;

    thread_result = pthread_create(&agent_thread, NULL, &agent_loop, NULL);
    if (thread_result != 0)
        printf("\nCan't create thread :[%s]", strerror(thread_result));

    for (int i=0; i<=2; i++) {
        thread_result = pthread_create(&(tid[i]), NULL, &trade_loop, &smokers[i]);
        if (thread_result != 0)
            printf("\nCan't create thread :[%s]", strerror(thread_result));
    }

    pthread_join(agent_thread, NULL);
    pthread_mutex_destroy(&lock);
    return 0;
}