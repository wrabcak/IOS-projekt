/*
 * Projekt:     Santa Claus Problem, 2. projekt, kurz IOS 
 * @file:       main.h
 * @date:       2013/04/27
 * @author:     Lukas Vrabec xvrabe07@stud.fit.vutbr.cz
 * @class:      1.BIB
 * @brief:      hlavickovy subor k suboru main.c
 * 
 */
#ifndef SANTA_H
#define SANTA_H


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

//* definicia makier na vypisovanie chybovych hlaseni na stderr */

#define ERROR_FORK_ELF  \
{                       \
    fprintf(stderr,"Chyba pri forkovani procesu skriatok!\n");\
}                       \

#define ERROR_FORK_SANTA \
{                       \
    fprintf(stderr,"Chyba pri forkovani procesu santa!\n"); \
}                       \

#define ERROR_PARAM     \
{                       \
    fprintf(stderr,"Zle zadane parametre!!!\n"); \
    return 1;           \
}                       \

#define ERROR_SEM_CLOSE     \
{                       \
    fprintf(stderr,"Chyba pri ukoncovani semaforu!!!\n"); \
    return 1;           \
}                       \

#define ERROR_SH_MEM_CLOSE     \
{                       \
    fprintf(stderr,"Chyba pri unmapovani zdielanej pamate!!!\n"); \
    return 1;           \
}                       \

#define ERROR_SH_MEM_UNLINK     \
{                       \
    fprintf(stderr,"Chyba pri unlinkovani zdielanej pamate!!!\n"); \
    return 1;           \
}                       \

#define ERROR_SEM_UNLINK        \
{                       \
    fprintf(stderr,"Chyba pri unlinkovani semaforu!!!\n"); \
    return 1;           \
}                       \

#define ERROR_CLOSE        \
{                       \
    fprintf(stderr,"Chyba pri ukoncovani file deskriptoru!!!\n"); \
    return 1;           \
}

#define ERROR_FCLOSE        \
{                       \
    fprintf(stderr,"Chyba zatvarani suboru!!!\n"); \
    return 2;           \
}

//* 
#define MILISEKUNDY 1000.0

//* struktura ktoru pouzivam pri zdielanej pamati */
typedef struct struct_pamat
{
    int counter;   //* pocitadlo na ocislovanie vystupov */
    int pocet_skriatkov; //* pocet elfov ktory su v danom momente aktivny */
    int obsluha; //* pocet elfov ktory cakaju na pomoc od santu */
}struct_pamat;

//* struktura pre vsetky pouzivane semafory */
typedef struct semaphores
{
    sem_t *sem_counter;         //* mutex na osetrovanie pristupu do kritickych sekcii */
    sem_t *santa_help;          //* semafor ktorym skriatkovia ziadaju pomoc od santy  */ 
    sem_t *wait_skriatok;       //* semafor sluzi na to aby santa pomahal vzdy bud trojici alebo samotnemu skriatkovi (procesu) */
    sem_t *sem_elf_got_help;    //* semafor ktorym santa oznamuje ze skriatok dostal pomoc */ 
    sem_t *sem_queue;           //* semafor sluzi na to aby poziadal o pomoc vzdy len jeden skriatok */
    sem_t *elf_finished;        //* semafor elfovia ktory su na dovolenke cakaju kym skoncia vsetci ostatny a nasledne potom santa odomnkne semafor a vsetci elfia sa ukoncia */
}semaphores;

/*
  * @name: proces_do
  * @param [C]                   pocet navstev na ktore musi ist skriatok k santovi kym odide na dovolenku.
  * @param [E]                   pociatocny pocet skriatkov pracujucich na hrackach.
  * @param [H]                   maximální hodnota doby (v milisekundách) výroby hraček skřítkem.
  * @param [S]                   je maximální hodnota doby (v milisekundách) obsluhy skřítků Santov.
  * @reval  [0]                  vsetko prebehlo v poriadku
  * @retval [1]                  zlyhanie v pripade zleho vstupu etc.
  * @retval [2]                  zlyhalo systemove volanie.
  * @brief                       vo funkcii forkujem procesy a otvaram semafory
  */
int proces_do(int C,int E,int H,int S);

/*
 * @name: santa
 * @param [*output_file]         file deskriptor na subor pre vystup programu
 * @param [*zdielana_pamat]      struktura v ktorej su premenne zdielanej pamate
 * @param [E]                    pociatocny pocet skriatkov pracujucich na hrackach.
 * @param [S]                    je maximální hodnota doby (v milisekundách) obsluhy skřítků Santov.
 * @param [*semafory]            struktura ktora obsahuje semafory 
 * @retaval [0]                  vsetko prebehlo v poriadku
 * @brief                        funkcia obsahuje obsluhu vykonavania santu
 */
int santa(FILE *output_file,struct_pamat *zdielana_pamat,int E,int S,semaphores *semafory);

/*
 * @name: skriatok
 * @param [*output_file]         file deskriptor na subor pre vystup programu
 * @param [*zdielana_pamat]      struktura v ktorej su premenne zdielanej pamate
 * @param [I]                    pocet skriatkov cakajucich na obsluhu
 * @param [H]                    maximální hodnota doby (v milisekundách) výroby hraček skřítkem
 * @param [C]                    pocet navstev na ktore musi ist skriatok k santovi kym odide na dovolenku
 * @param [*semafory]            struktura ktora obsahuje semafory 
 * @retval [0]                   vsetko zbehlo v poriadku
 * @brief                        funkcia obsahuje obsluhu vykonavania skriatka/skriatkov
 */
int skriatok(FILE *output_file,struct_pamat *zdielana_pamat, int I, int H, int C, semaphores *semafory);

/*
 * @name: random_time           
 * @param [number]              maximalna hodnota ktora sa ma nahodne vybrat z 0 az NUMBER cisel
 * @reval                       nahodne cislo v intervale 0 az NUMBER
 * @brief                       funkcia sluzi na nahodne vybratie cisla potrebneho pre simulovanie prace skriatka alebo santu
 */
int random_time(int number);

/*
 * @name: cleanup
 * @param [*zdielana_pamat]     struktura v ktorej su premenne zdielanej pamate
 * @param [sh_mem]              pointer na miesto v pamati kde zacina zdielana pamat
 * @param [*semafory]           struktura ktora obsahuje semafory 
 * @brief                       funkciu po sebe program "upratuje" tj. uzatvara semafory, unmapuje etc.
 */
int cleanup(struct_pamat *zdielana_pamat, int sh_mem, semaphores *semafory);

/*
 * @name: print_help
 * @brief                       funkcia vypisuje napovedu
 */
void print_help(void);

#endif

//* ------ Koniec suboru main.h ----- */

