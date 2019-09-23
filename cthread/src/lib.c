
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"

//1 = Printa informacoes de criacao/troca de threads no terminal
//0 = Nao printa nada
#define DEBUG_MODE 1

int initied = 0;
int nextTid = 0;

FILA2 fApto, fBloq, fExec, fTerm;

ucontext_t econtext; //Contexto do escalonador

void give_cpu_to_next(){
    int done;

    FirstFila2(&fExec);
    TCB_t* executando = ((TCB_t*) GetAtIteratorFila2(&fExec));
    TCB_t* nextThread;
    TCB_t* auxThread;

    //Acha a thread mais prioritaria (menor "prio") e armazena em "nextThread"
    FirstFila2(&fApto);
    nextThread = GetAtIteratorFila2(&fApto);
    while(NextFila2(&fApto) == 0){
        auxThread = GetAtIteratorFila2(&fApto);
        if(auxThread->prio < nextThread->prio)
            nextThread = auxThread;
    }

    //Remove a thread antiga da fila de executando e coloca a nova
    DeleteAtIteratorFila2(&fExec);
    AppendFila2(&fExec, nextThread);

    //Remove a thread escalonada da fila de aptos.
    FirstFila2(&fApto);
    auxThread = GetAtIteratorFila2(&fApto);
    done = 0;
    while(done == 0){
        if(auxThread->tid == nextThread->tid){
            done = 1;
            DeleteAtIteratorFila2(&fApto);
        }
        else{
            NextFila2(&fApto);
            auxThread = GetAtIteratorFila2(&fApto);
        }
    }

    if(DEBUG_MODE)
        printf("\033[0;31mTrocando %d por %d\n\033[0m", executando->tid, nextThread->tid);

    startTimer();
    swapcontext(&(executando->context), &(nextThread->context));
}

void escalonador(){
    getcontext(&econtext);

    if(!initied)
        return;

    //Quando chegar aqui, alguma thread acabou a sua execucao
    int done;

    FirstFila2(&fExec);
    TCB_t* executando = ((TCB_t*) GetAtIteratorFila2(&fExec));

    FirstFila2(&fBloq);
    TCB_t* auxBloq = ((TCB_t*) GetAtIteratorFila2(&fBloq));

    //Verifica se alguem estava esperando essa thread
    if(executando->waiting_tid != -1){
        done = 0;
        while(done == 0){
            if(auxBloq->tid == executando->waiting_tid){
                done = 1;
                DeleteAtIteratorFila2(&fBloq);
            }
            else{
                NextFila2(&fBloq);
                auxBloq = GetAtIteratorFila2(&fBloq);
            }
        }
        AppendFila2(&fApto, auxBloq);
    }

    //Coloca a thread terminada na fila de termino
    executando->prio = stopTimer();
    executando->state = PROCST_TERMINO;
    AppendFila2(&fTerm, executando);

    //Escalona a proxima thread
    give_cpu_to_next();
}

void init_cThread(){
    //Cria as filas
    if(CreateFila2(&fApto) || CreateFila2(&fBloq) || CreateFila2(&fExec) || CreateFila2(&fTerm)){
        printf("Erro ao criar fila.");
        exit(0);
    }

    //Inicializa o contexto do escalonador
    escalonador();

    //Inicializa o TCB da "main"
    ucontext_t mainContext;
    getcontext(&mainContext);

    TCB_t* mainTCB = (TCB_t*) malloc(sizeof(TCB_t));
    mainTCB->tid = nextTid; //Sempre 0
    nextTid++;
    mainTCB->state = PROCST_EXEC;
    mainTCB->prio = 0;
    mainTCB->context = mainContext;
    mainTCB->waiting_tid = -1;

    AppendFila2(&fExec, mainTCB);

    startTimer();
    initied = 1;
}

int ccreate (void* (*start)(void*), void *arg, int prio) {
    if(initied == 0)
        init_cThread();

    ucontext_t threadContext;
    getcontext(&threadContext);
    threadContext.uc_link = &econtext;
    threadContext.uc_stack.ss_sp = malloc(sizeof(char)*16384);
    threadContext.uc_stack.ss_size = 16384;
    makecontext(&threadContext, start, 1, arg);

    TCB_t* threadTCB;
    threadTCB = (TCB_t*) malloc(sizeof(TCB_t));

    threadTCB->tid = nextTid;
    nextTid++;
    threadTCB->state = PROCST_APTO;
    threadTCB->prio = prio;
    threadTCB->context = threadContext;
    threadTCB->waiting_tid = -1;

    AppendFila2(&fApto, threadTCB);

    if(DEBUG_MODE)
        printf("\033[0;32mCriada a Thread %d\n\033[0m", threadTCB->tid);

	return threadTCB->tid;
}

int cyield(void) {
	FirstFila2(&fExec);
    TCB_t* executando = ((TCB_t*) GetAtIteratorFila2(&fExec));

	executando->prio = stopTimer();
	executando->state = PROCST_APTO;
	AppendFila2(&fApto, executando);
	give_cpu_to_next();

	return 0;
}

int cjoin(int tid) {
    int done;

    TCB_t* tW = NULL;

    FirstFila2(&fExec);
    TCB_t* executando = ((TCB_t*) GetAtIteratorFila2(&fExec));

    //Procura em aptos
    FirstFila2(&fApto);
    done = 0;
    while(!done){
        tW = (TCB_t*) GetAtIteratorFila2(&fApto);
        if(tW == NULL)
            done = 1;
        else{
            if(tW->tid == tid){
                if(tW->waiting_tid != -1)
                    return -1;

                tW->waiting_tid = executando->tid;
                done = 1;
            }
            else{
                tW = NULL;
                if(NextFila2(&fApto))
                    done = 1;
            }
        }
    }
    if(tW != NULL){
        AppendFila2(&fBloq, executando);
        executando->state = PROCST_BLOQ;
        executando->prio = stopTimer();
        give_cpu_to_next();
        return 0;
    }

    //Procura em Bloq
    FirstFila2(&fBloq);
    done = 0;
    while(!done){
        tW = (TCB_t*) GetAtIteratorFila2(&fBloq);
        if(tW == NULL)
            done = 1;
        else{
            if(tW->tid == tid){
                if(tW->waiting_tid != -1)
                    return -1;

                tW->waiting_tid = executando->tid;
                done = 1;
            }
            else{
                tW = NULL;
                if(NextFila2(&fBloq))
                    done = 1;
            }
        }
    }
    if(tW != NULL){
        AppendFila2(&fBloq, executando);
        executando->state = PROCST_BLOQ;
        executando->prio = stopTimer();
        give_cpu_to_next();
        return 0;
    }

    return -1;
}

int csem_init(csem_t *sem, int count) {
	sem->fila = (PFILA2) malloc(sizeof(FILA2));
	if(CreateFila2(sem->fila)){
        printf("Erro ao criar fila para semaforo");
        exit(0);
	}
	sem->count = count;

	if(DEBUG_MODE)
        printf("\033[0;33mInicializado semaforo com count: %d\n\033[0m", sem->count);

	return 0;
}

int cwait(csem_t *sem) {
	return -1;
}

int csignal(csem_t *sem) {
	return -1;
}

int cidentify (char *name, int size) {
	strncpy (name, "Bernardo Hummes - \nIvan Peter Lamb - 287692\nMaria Cecilia - ", size);
	return 0;
}


