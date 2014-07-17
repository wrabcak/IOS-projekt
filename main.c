/*
 * Projekt:     Santa Claus Problem, 2. projekt, kurz IOS 
 * @file:       main.c
 * @date:       2013/04/27
 * @author:     Lukas Vrabec xvrabe07@stud.fit.vutbr.cz
 * @class:      1.BIB
 * @brief:      Implementujte v jazyku C modifikovaný synchronizační problém Santa Claus Problem (můžete porovnat např. s jednou z verzí tohoto problému).
 *              Santa a skřítci jsou na severním pólu, Santa odpočívá a skřítci vyrábějí hračky. Pokud skřítek narazí na problém, jde za Santou s žádostí o pomoc. Poté,
 *              co se skřítkovi dostane pomoci od Santy, vrací se skřítek zpět do práce. Jakmile počet návštěv skřítka u Santy dosáhne předem stanoveného počtu, skřítek odjede na dovolenou.
 *              Jakmile Santa pomohl poslednímu skřítkovi (všichni jsou na dovolené), ukončí se procesy skřítků a proces Santy. Skřítci vyrábějící hračky jsou aktivní skřítci,
 *              skřítci na dovolené jsou odpočívající skřítci. 
 */

//* includovanie hlavickovych suborov
#include "main.h"

int main(int argc, char *argv[])
{
    int C; //* pocet navstev na ktore musi ist skriatok k santovi kym odide na dovolenku */
    int E; //* pociatocny pocet skriatkov pracujucich na hrackach */
    int H; //* maximální hodnota doby (v milisekundách) výroby hraček skřítkem */
    int S; //* je maximální hodnota doby (v milisekundách) obsluhy skřítků Santov */
    int return_value=0;  //* pomocna premenna pre navratovu hodnotu funkcie proces_do */
    char *check_error=0; //* pomocna premenna prekontrolovanie spravnosti argumentov programu */
    if (argc == 2 && (strcmp(argv[1],"-h") == 0 || strcmp(argv[1],"--help") == 0 ) )
    {
        print_help();
        return 0;
    }
    if(argc==5)
    {
        C=strtol(argv[1], &check_error, 10); 
        if (*check_error != '\0')
        {
            ERROR_PARAM;
            return 1;
        }
        E=strtol(argv[2], &check_error, 10);
        if (*check_error != '\0')
        {
            ERROR_PARAM;
            return 1;
        }
    }
    if (argc==5 && C>0 && E>0)
    {
        H=strtol(argv[3], &check_error, 10);
        if (*check_error != '\0')
        {
            ERROR_PARAM;
            return 1;
        }
        S=strtol(argv[4], &check_error, 10);
        if (*check_error != '\0')
        {
            ERROR_PARAM;
            return 1;
        }
        if((return_value=proces_do(C,E,H,S))==2)
            return 2;
        else if (return_value==1)
            return 1;
    }
    else
    {
        ERROR_PARAM;
        return 1;
    }
    return 0;
}

int proces_do(int C,int E,int H,int S)
{
     semaphores *semafory = malloc(sizeof(semaphores));
     if (semafory == NULL)
     {
         fprintf(stderr,"Chyba pri alokacii pamate pre semafory!!!\n");
         return 1;
     }
     int sh_mem = shm_open("/xvrabe07_sharedMemory", O_CREAT | O_EXCL | O_RDWR, 0666); //* vytvorenie zdielanej pamate */
     if ( sh_mem == -1 )
     {
         fprintf(stderr, "Chyba pri vytvarani zdielanej pamate!!!\n");
         return 1;
     }
     if (( ftruncate(sh_mem, sizeof(struct_pamat))) == -1 ) //* alokovanie 2 int do zdielanej pamate */
     {
         fprintf(stderr,"Chyba pri alokacii zdielanej pamate!!!\n");
         return 2;
     }
     struct_pamat *zdielana_pamat = mmap(NULL, sizeof(struct_pamat), PROT_READ | PROT_WRITE, MAP_SHARED, sh_mem, 0); // Namapovanie zdielanej pamate do adresneho priestoru procesu
     if ( zdielana_pamat == MAP_FAILED )
     {
         fprintf(stderr,"Chyba pri namapovani zdielanej pamate!!!\n");
         return 2;
     }
     semafory->sem_queue = sem_open("/xvrabe07_semafor5", O_CREAT | O_EXCL, 0666, 0);
     if ( semafory->sem_queue == SEM_FAILED )
     {
         fprintf(stderr,"Chyba pri otvarani semaforu!!!\n");
         return 1;
     }         
     semafory->sem_counter = sem_open("/xvrabe07_semafor1", O_CREAT | O_EXCL, 0666, 1);  //* mutex */
     if ( semafory->sem_counter == SEM_FAILED )
     {
         fprintf(stderr,"Chyba pri otvarani semaforu!!!\n");
         return 1;
     }          
     semafory->santa_help = sem_open("/xvrabe07_semafor2", O_CREAT | O_EXCL, 0666, 0);
     if ( semafory->santa_help == SEM_FAILED )
     {
         fprintf(stderr,"Chyba pri otvarani semaforu!!!\n");
         return 1;
     } 
     semafory->wait_skriatok = sem_open("/xvrabe07_semafor3", O_CREAT | O_EXCL, 0666, 0);
     if ( semafory->wait_skriatok == SEM_FAILED )
     {
         fprintf(stderr,"Chyba pri otvarani semaforu!!!\n");
         return 1;
     }     
     semafory->sem_elf_got_help = sem_open("/xvrabe07_semafor4", O_CREAT | O_EXCL, 0666, 0);
     if ( semafory->sem_elf_got_help == SEM_FAILED )
     {
         fprintf(stderr,"Chyba pri otvarani semaforu!!!\n");
         return 1;
     }
     semafory->elf_finished = sem_open("/xvrabe07_semafor6", O_CREAT | O_EXCL, 0666, 0);
     if ( semafory->elf_finished == SEM_FAILED )
     {
         fprintf(stderr,"Chyba pri otvarani semaforu!!!\n");
         return 1;
     }
     zdielana_pamat->counter=1;  
     zdielana_pamat->pocet_skriatkov=E;
     FILE *output_file;
     if((output_file=fopen("santa.out","w"))== NULL)
     {
         fprintf(stderr,"Chyba pri otvarani suboru santa.out\n");
         return 1;
     }
     setlinebuf(output_file);
     pid_t pid_santa = fork();
     if(pid_santa<0)
     {
         ERROR_FORK_SANTA;
         cleanup(zdielana_pamat,sh_mem,semafory);
         if(fclose(output_file) == EOF )
            ERROR_FCLOSE;
         return 2;
     }
     else if(pid_santa==0) // proces santa
     {
        
        santa(output_file,zdielana_pamat,E,S,semafory);   
        exit(0);
     }
     else if (pid_santa>0)
     {   
        for (int i=1;i<=E;i++) //E je pocet skriatkov pracujucich na hrackach
        {
            pid_t pid_skriatok = fork();
            if(pid_skriatok<0)
            {
                ERROR_FORK_ELF;
                kill(pid_santa,SIGSTOP);
                cleanup(zdielana_pamat,sh_mem,semafory);
                if(fclose(output_file) == EOF )
                    ERROR_FCLOSE;
                return 2;
            }
            else if(pid_skriatok==0) // proces skriatok
            {
                
                skriatok(output_file,zdielana_pamat,i,H,C,semafory);      
                exit(0);
            }
        } 
     }
     for(int i=1;i<=E+1;i++)
     {
        int status;
        wait(&status);
     }
     cleanup(zdielana_pamat,sh_mem,semafory);
     if(fclose(output_file) == EOF )
        ERROR_FCLOSE;
     exit(0);
     return 0;
}

int santa(FILE *output_file,struct_pamat *zdielana_pamat,int E,int S,semaphores *semafory)
{
    srand(getpid() * time(NULL));
    sem_wait(semafory->sem_counter);
    if(zdielana_pamat->pocet_skriatkov>3)
        {
            sem_post(semafory->sem_counter);
            for(int j=0;j<3;j++)
                sem_post(semafory->sem_queue);
        }
    else
        sem_post(semafory->sem_counter);
    sem_wait(semafory->sem_counter);
    if(zdielana_pamat->pocet_skriatkov<=3)
    {
        sem_post(semafory->sem_counter);
        sem_post(semafory->sem_queue);
    }
    else
        sem_post(semafory->sem_counter);
    sem_wait(semafory->sem_counter);
    fprintf(output_file," %d: santa: started\n",zdielana_pamat->counter++);
    sem_post(semafory->sem_counter);
    sem_wait(semafory->sem_counter);
    while(zdielana_pamat->pocet_skriatkov>0)
    {
        if(zdielana_pamat->pocet_skriatkov>3)
        {
            sem_post(semafory->sem_counter);
            for(int i=0;i<3;i++) 
                sem_wait(semafory->santa_help);
        }
        else
        {
            sem_post(semafory->sem_counter);
            sem_wait(semafory->santa_help);
        }
        sem_wait(semafory->sem_counter);
        if(zdielana_pamat->pocet_skriatkov>0)
            fprintf(output_file," %d: santa: checked state: %d: %d\n", zdielana_pamat->counter++,zdielana_pamat->pocet_skriatkov,zdielana_pamat->obsluha); // poriesit NE a WE!!!!!
        sem_post(semafory->sem_counter);
        sem_wait(semafory->sem_counter);
        if (zdielana_pamat->pocet_skriatkov>0)
        {
            sem_post(semafory->sem_counter);
            sem_wait(semafory->sem_counter);
            fprintf(output_file," %d: santa: can help\n", zdielana_pamat->counter++);
            sem_post(semafory->sem_counter);
            usleep(random_time(S));
        }
        else 
            sem_post(semafory->sem_counter);
        sem_wait(semafory->sem_counter);
        int obs=zdielana_pamat->obsluha;
        sem_post(semafory->sem_counter);
        for(int i=0;i<obs;i++)
        {
            sem_post(semafory->sem_elf_got_help);   
            sem_wait(semafory->wait_skriatok);
        }
        sem_wait(semafory->sem_counter);
        if(zdielana_pamat->pocet_skriatkov>3)
        {
            sem_post(semafory->sem_counter);
            for(int i=0;i<3;i++)
                sem_post(semafory->sem_queue); 
        }
        else
            sem_post(semafory->sem_counter);
        sem_wait(semafory->sem_counter);
        if(zdielana_pamat->pocet_skriatkov<=3)
        {      
            sem_post(semafory->sem_counter);
            sem_post(semafory->sem_queue);
        }
        else
            sem_post(semafory->sem_counter);
        sem_wait(semafory->sem_counter);
        if(zdielana_pamat->pocet_skriatkov==0)
        {
            sem_post(semafory->sem_counter);
            sem_wait(semafory->sem_counter);
            fprintf(output_file," %d: santa: checked state: 0: 0\n", zdielana_pamat->counter++);
            sem_post(semafory->sem_counter);
            for(int i=1;i<=E;i++)
                sem_post(semafory->elf_finished);
            sem_wait(semafory->sem_counter);
            fprintf(output_file," %d: santa: finished\n",zdielana_pamat->counter++);
            sem_post(semafory->sem_counter);
        }
        else
            sem_post(semafory->sem_counter);
        sem_wait(semafory->sem_counter);
    }
    sem_post(semafory->sem_counter);
    exit(0);
    return 0;
}

int skriatok(FILE *output_file,struct_pamat *zdielana_pamat, int I, int H, int C, semaphores *semafory)
{
    srand(getpid() * time(NULL));
    sem_wait(semafory->sem_counter);
    fprintf(output_file," %d: elf: %d: started\n", zdielana_pamat->counter++, I);
    sem_post(semafory->sem_counter);
    while (C>0)
    {
        usleep(random_time(H));
        sem_wait(semafory->sem_counter);
        fprintf(output_file," %d: elf: %d: needed help\n", zdielana_pamat->counter++,I);
        sem_post(semafory->sem_counter);
        sem_wait(semafory->sem_queue);
        sem_wait(semafory->sem_counter);
        fprintf(output_file," %d: elf: %d: asked for help\n", zdielana_pamat->counter++,I);
        zdielana_pamat->obsluha++;
        sem_post(semafory->sem_counter);
        sem_post(semafory->santa_help);
        C--;
        sem_wait(semafory->sem_elf_got_help);
        sem_wait(semafory->sem_counter);
        fprintf(output_file," %d: elf: %d: got help\n", zdielana_pamat->counter++, I);
        zdielana_pamat->obsluha--; 
        sem_post(semafory->sem_counter);
        sem_post(semafory->wait_skriatok);
    }
    sem_wait(semafory->sem_counter);
    fprintf(output_file," %d: elf: %d: got a vacation\n", zdielana_pamat->counter++, I);
    zdielana_pamat->pocet_skriatkov--;
    sem_post(semafory->sem_counter);
    if(zdielana_pamat->pocet_skriatkov==0)
        sem_post(semafory->santa_help);
    sem_wait(semafory->elf_finished);
    sem_wait(semafory->sem_counter);
    fprintf(output_file," %d: elf: %d: finished\n",zdielana_pamat->counter++,I);
    sem_post(semafory->sem_counter);
    return 0;
}

int cleanup(struct_pamat *zdielana_pamat, int sh_mem, semaphores *semafory)
{
    if( sem_close(semafory->sem_counter) == -1 )
        ERROR_SEM_CLOSE;
    if( sem_close(semafory->santa_help) == -1 )
        ERROR_SEM_CLOSE;
    if( sem_close(semafory->wait_skriatok) == -1 )
        ERROR_SEM_CLOSE;
    if( sem_close(semafory->sem_elf_got_help) == -1 )
        ERROR_SEM_CLOSE;
    if( sem_close(semafory->elf_finished) == -1 )
        ERROR_SEM_CLOSE;
    if( sem_close(semafory->sem_queue) == -1 )
        ERROR_SEM_CLOSE;
    if( munmap(zdielana_pamat, sizeof(struct_pamat)) == -1 )
        ERROR_SH_MEM_CLOSE;
    if( shm_unlink("/xvrabe07_sharedMemory") == -1 )
        ERROR_SH_MEM_UNLINK;
    if( sem_unlink("/xvrabe07_semafor1") == -1 )
        ERROR_SEM_UNLINK;
    if( sem_unlink("/xvrabe07_semafor2") == -1 )
        ERROR_SEM_UNLINK;
    if( sem_unlink("/xvrabe07_semafor3") == -1 )
        ERROR_SEM_UNLINK;
    if( sem_unlink("/xvrabe07_semafor4") == -1 )
        ERROR_SEM_UNLINK;
    if( sem_unlink("/xvrabe07_semafor5") == -1 )
        ERROR_SEM_UNLINK;
    if( sem_unlink("/xvrabe07_semafor6") == -1 )
        ERROR_SEM_UNLINK;
    free(semafory);
    if( close(sh_mem) == -1 )
        ERROR_CLOSE;
    return 0;
}
    
int random_time(int number)
{
    return (rand() % (number+1))*MILISEKUNDY;
    
}

void print_help(void )
{
    printf("Usage: santa [C] [E] [H] [S]\n");
    printf("\t C - pocet navstev na ktore musi ist skriatok k santovi kym odide na dovolenku\n");
    printf("\t E - pociatocny pocet skriatkov pracujucich na hrackach\n");
    printf("\t H - maximálna hodnota doby (v milisekundách) výroby hraciek skriatkom\n");
    printf("\t S - je maximálna hodnota doby (v milisekundách) obsluhy skriatkov santom\n");
    printf("\t Vsetky cisla su cele cisla!\n");
}

//* ------ Koniec suboru main.c ----- */
