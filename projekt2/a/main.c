#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <semaphore.h>

//Zagłodzeni czytelnicy

int p = 0, c = 0;
int liczbaP,liczbaC;
int pisarzeKolejka = 0,first = 1;
pthread_mutex_t mutexP = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexC = PTHREAD_MUTEX_INITIALIZER;

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
void *Pisarz()
{
    while(1)
    {
        pisarzeKolejka++;
        pthread_mutex_lock(&mutexP);
        pisarzeKolejka--;

        if(first)
        {
            pthread_mutex_lock(&mutexC);
            first = 0;
        }
            

        p++;
        printf("Kczytelnikow: %d Kpisarzy: %d [C:%d P:%d]\n",liczbaC-c,liczbaP-p,c,p);
        usleep((5+rand()%15)*100000);
        p--;
        printf("Kczytelnikow: %d Kpisarzy: %d [C:%d P:%d]\n",liczbaC-c,liczbaP-p,c,p);

        if(pisarzeKolejka == 0)
        {
            pthread_mutex_unlock(&mutexC);
            first = 1;
        }
            
        pthread_mutex_unlock(&mutexP);

        usleep((5+rand()%15)*100000);
    }
    
}
void *Czytelnik()
{
    while(1)
    {
        if(pisarzeKolejka == 0)
        {
            pthread_mutex_lock(&mutexC);
            if(c == 0)
                pthread_mutex_lock(&mutexP);
            
            c++;
            pthread_mutex_unlock(&mutexC);

            printf("Kczytelnikow: %d Kpisarzy: %d [C:%d P:%d]\n",liczbaC-c,liczbaP-p,c,p);
            usleep((5+rand()%15)*100000);
            c--;
            printf("Kczytelnikow: %d Kpisarzy: %d [C:%d P:%d]\n",liczbaC-c,liczbaP-p,c,p);

            if(c == 0)
                pthread_mutex_unlock(&mutexP);
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

    pthread_t threads[liczbaP + liczbaC];

    int control;
    for(int i=0;i<liczbaP + liczbaC;i++)
    {
        if(i < liczbaP)
            control = pthread_create(&threads[i], NULL, Pisarz, NULL);
        else
            control = pthread_create(&threads[i], NULL, Czytelnik, NULL);
        if(control) 
        {
            fprintf(stderr,"Error - pthread_create() return code: %d\n",control);
            return 2;
        }
    }
    for(int i=0;i<liczbaP + liczbaC;i++)
    {
        if(pthread_join(threads[i],NULL) != 0)
            return 3;
    }
    pthread_mutex_destroy(&mutexC);
    pthread_mutex_destroy(&mutexP);

    return 0;
}