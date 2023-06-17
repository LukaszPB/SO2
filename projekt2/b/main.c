#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

//Brak zagłodzenia

struct fifo
{
    pthread_cond_t cond;
    pthread_t thread;
    struct fifo *next;
    int kat;
    int id;
};


int p = 0, c = 0;
int liczbaP = 0, liczbaC = 0;
int info = 0;
pthread_mutex_t mutexCzytelnia = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexWypisz = PTHREAD_MUTEX_INITIALIZER;
struct fifo *first = NULL;

void wypisz()
{
    struct fifo *p = first;
    printf("Zawartość kolejki \n");
    while(p!=NULL)
    {
        if(p->kat == 0)
            printf("Pisarz ");
        else
            printf("Czytelnik ");
        printf("%d: %p\n",p->id,(void*)&p->thread);

        p = p->next;
    }
}
int toInt(char *arg)
{
    int i = 0, liczba = 0, x;

    while(arg[i] != '\0')
    {
        x = arg[i++] - 48;

        if(x < 0 || x > 9)
            return -1;

        liczba = liczba*10 + x;    
    }
    return liczba;
}
pthread_cond_t* pop()
{
    struct fifo *tmp = first->next;
    struct fifo *p = first;
    struct fifo *a = first;

    while(p->next != NULL)
        p = p->next;

    first->next = NULL;
    p->next = first;
    first = tmp;

    return &a->cond;
}
void *Pisarz(void* arg)
{
    pthread_cond_t* my_cond = (pthread_cond_t*)arg;

    while(1)
    {
        pthread_cond_wait(my_cond,&mutexCzytelnia);

        p++;
        printf("Kczytelnikow: %d Kpisarzy: %d [C:%d P:%d]\n",liczbaC-c,liczbaP-p,c,p);
        usleep((5+rand()%15)*100000);
        p--;
        printf("Kczytelnikow: %d Kpisarzy: %d [C:%d P:%d]\n",liczbaC-c,liczbaP-p,c,p);

        if(info)
        {
            pthread_mutex_lock(&mutexWypisz);
            wypisz();
            pthread_mutex_unlock(&mutexWypisz);
        }

        pthread_mutex_unlock(&mutexCzytelnia);
        pthread_cond_signal(pop());

        usleep((5+rand()%15)*100000);
    }
}
void *Czytelnik(void* arg)
{
    pthread_cond_t* my_cond = (pthread_cond_t*)arg;
    int a;

    while(1)
    {
        pthread_cond_wait(my_cond,&mutexCzytelnia);
        a = first->next->kat;
        if(a == 1)
        {
            pthread_mutex_unlock(&mutexCzytelnia);
            pthread_cond_signal(pop());
        }

        c++;
        printf("Kczytelnikow: %d Kpisarzy: %d [C:%d P:%d]\n",liczbaC-c,liczbaP-p,c,p);
        usleep((5+rand()%15)*100000);
        c--;
        printf("Kczytelnikow: %d Kpisarzy: %d [C:%d P:%d]\n",liczbaC-c,liczbaP-p,c,p);

        pthread_mutex_unlock(&mutexCzytelnia);

        if(info)
        {
            pthread_mutex_lock(&mutexWypisz);
            wypisz();
            pthread_mutex_unlock(&mutexWypisz);
        }

        if(a == 0)
        {
            pthread_mutex_unlock(&mutexCzytelnia);
            pthread_cond_signal(pop());
        }

        usleep((5+rand()%15)*100000);
    }
}

int main(int argc, char *argv[]) 
{
    time_t tt;
    int zarodek = time(&tt);
    srand(zarodek);

    if(argc < 3)
    {
        printf("Zbyt mało argumentów, podaj dwie liczby dodatnie (liczbę pisarzy i czytelników)\n");
        return 1;
    }
    liczbaP = toInt(argv[1]);
    liczbaC = toInt(argv[2]);
    if(liczbaP == -1 || liczbaC == -1)
    {
        printf("Argumenty muszą być liczbami dodatnimi\n");
        return 1;
    }
    if(argc > 3)
        if(argv[3][0] == 'i')
            info = 1;

    int control;
    struct fifo *p;
    struct fifo *nowy;
    for(int i=0;i<liczbaP + liczbaC;i++)
    {
        if(i < liczbaP)
        {
            nowy = malloc(sizeof(struct fifo));
            pthread_cond_init(&nowy->cond,NULL);
            control = pthread_create(&nowy->thread, NULL, Pisarz, (void*)&nowy->cond);
            nowy->next = NULL;
            nowy->kat = 0;
            nowy->id = i+1;
            if(first == NULL)
                first = nowy;
            else
            {
                p = first;
                while(p->next != NULL)
                    p = p->next;
                p->next = nowy;
            }
        }
        else
        {
            nowy = malloc(sizeof(struct fifo));
            pthread_cond_init(&nowy->cond,NULL);
            control = pthread_create(&nowy->thread, NULL, Czytelnik, (void*)&nowy->cond);
            nowy->next = NULL;
            nowy->kat = 1;
            nowy->id = i+1-liczbaP;
            if(first == NULL)
                first = nowy;
            else
            {
                p = first;
                while(p->next != NULL)
                    p = p->next;
                p->next = nowy;
            }
        }
        if(control) 
        {
            fprintf(stderr,"Error - pthread_create() return code: %d\n",control);
            return 2;
        }
    }
    
    pthread_cond_signal(pop());
    
    struct  fifo *tmp; 
    while(first != NULL)
    {
        if(pthread_join(first->thread,NULL) != 0)
            return 3;
        pthread_cond_destroy(&first->cond);
        tmp = first;
        first = first->next;
        free(tmp);
    }

    pthread_mutex_destroy(&mutexCzytelnia);

    return 0;
}