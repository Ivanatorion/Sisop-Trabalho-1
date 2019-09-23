
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"

int initied = 0;
int nextTid = 0;

FILA2 fApto, fBloq, fExec;

ucontext_t econtext;

void give_cpu_to_next(){
    int done;

    FirstFila2(&fExec);
    TCB_t* executando = ((TCB_t*) GetAtIteratorFila2(&fExec));
    TCB_t* nextThread;
    TCB_t* auxThread;
    FirstFila2(&fApto);
    nextThread = GetAtIteratorFila2(&fApto);
    while(NextFila2(&fApto) == 0){
        auxThread = GetAtIteratorFila2(&fApto);
        if(auxThread->prio < nextThread->prio)
            nextThread = auxThread;
    }
    DeleteAtIteratorFila2(&fExec);
    AppendFila2(&fExec, nextThread);

    //Remove a Thread da fila de aptos.
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

    printf("\033[0;31mTrocando %d por %d\n\033[0m", executando->tid, nextThread->tid);

    swapcontext(&(executando->context), &(nextThread->context));

}

void escalonador(){
    getcontext(&econtext);

    if(!initied)
        return;

    //Quando chega aqui, alguma thread acabou a sua execucao
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

    give_cpu_to_next();
}

void init_cThread(){
    if(CreateFila2(&fApto) || CreateFila2(&fBloq) || CreateFila2(&fExec)){
        printf("Erro ao criar fila.");
    }

    escalonador();

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

    printf("\033[0;32mCreated Thread %d\n\033[0m", threadTCB->tid);

	return threadTCB->tid;
}

int cyield(void) {
	return -1;
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
        give_cpu_to_next();
        return 0;
    }

    return -1;
}

int csem_init(csem_t *sem, int count) {
	return -1;
}

int cwait(csem_t *sem) {
	return -1;
}

int csignal(csem_t *sem) {
	return -1;
}

int cidentify (char *name, int size) {
	strncpy (name, "Sergio Cechin - 2019/2 - Teste de compilacao.", size);
	return 0;
}


