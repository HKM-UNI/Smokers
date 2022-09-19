#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdbool.h>

// Retraso entre acciones
#define SMOKING_TIME 5
#define PERMUTATION_TIME 1

// Partes del cigarro
typedef enum cigarette_enum { PAPER, MATCHES, TOBACCO } CigaretteMaterial;

// Datos del fumador
typedef struct smoker_type {
    char name[10];
    CigaretteMaterial material;
} Smoker;

// Ofertas del fumador
CigaretteMaterial offer1;
CigaretteMaterial offer2;

// Evento actual
bool is_smooking = false;

// Lista de threads para los fumadores
pthread_t tid[3];
// Thread blocker
pthread_mutex_t lock;

// Retorna el nombre del material como string
char* material_name(CigaretteMaterial material) {
    switch (material)
    {
        case PAPER: return "Paper"; break;
        case MATCHES: return "Matches"; break;
        case TOBACCO: return "Tobacco"; break;
        default: return "unknown"; break;
    }
}

// Pone las ofertas en la mesa
void release_offers()
{
    offer1 = (CigaretteMaterial) (rand() % 3);
    offer2 = (CigaretteMaterial) (rand() % 3);
    while (offer1 == offer2) {
        offer2 = (CigaretteMaterial) (rand() % 3);
    }

    printf("\nNew Materials: %s & %s\n", material_name(offer1), material_name(offer2));
    fflush(stdout);
    
    sleep(PERMUTATION_TIME);
}

// Comprueba al fumador que completa los materiales
void trade(Smoker *smoker)
{
    // Retorna si su material es igual al de uno de los ofertados
    if (smoker->material == offer1 || smoker->material == offer2) {
        return;
    }
    printf("[%s] has the %s\n", smoker->name, material_name(smoker->material));
    printf("[%s] is now smoking...\n",smoker->name);
	fflush(stdout);
	sleep(SMOKING_TIME);
}

// Ciclo de ejecución del Agente (infinito)
void* agent_loop(void* arg) {
    while (1) {
        // No liberará más ofertas si alguien ya está fumando
        if (is_smooking) {
            continue;
        }
        release_offers();
    }
    return NULL;
}

// Ciclo de ejecución de los fumadores (infinito)
void* trade_loop(void* arg) {
    Smoker* smoker = (Smoker*) arg;
    while (1) {
        // Bloquea la sección para que los demás fumadores deban esperar
        pthread_mutex_lock(&lock);
        is_smooking = true;
        trade(smoker);
        is_smooking = false;
        // Libera la sección, ahora los demás fumadores pueden hacer uso de este bloque
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

int main(void)
{
    // Semilla aleatoria para la generación de items
    srand(time(NULL));
    int thread_result;

    // Instanciación del locker
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }

    // Lista de fumadores, cada uno tiene solo un material
    Smoker smokers[3] = {
        { "Smoker 1", PAPER },
        { "Smoker 2", MATCHES },
        { "Smoker 3", TOBACCO }
    };

    // Thread del agente
    pthread_t agent_thread;

    // Instancia el thread del agente
    thread_result = pthread_create(&agent_thread, NULL, &agent_loop, NULL);
    if (thread_result != 0)
        printf("\nCan't create thread :[%s]", strerror(thread_result));

    // Instancia los threads de los fumadores
    for (int i=0; i<=2; i++) {
        thread_result = pthread_create(&(tid[i]), NULL, &trade_loop, &smokers[i]);
        if (thread_result != 0)
            printf("\nCan't create thread :[%s]", strerror(thread_result));
    }

    // Espera por el thread del agente, el cual nunca terminará, a menos que se
    // envién señales como SIGQUIT, SIQKILL, o SIGINT (teclado)
    pthread_join(agent_thread, NULL);

    // Libera los recursos del locker
    pthread_mutex_destroy(&lock);
    return 0;
}