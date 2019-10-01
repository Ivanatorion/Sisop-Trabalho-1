
/*
 *	Programa de exemplo de uso da biblioteca cthread
 *
 *	Vers�o 1.0 - 14/04/2016
 *
 *	Sistemas Operacionais I - www.inf.ufrgs.br
 *
 */

#include "../include/support.h"
#include "../include/cthread.h"
#include <stdio.h>
#include <stdlib.h>

int teste = 10;

csem_t* sem1;


void func2() {
    cwait(sem1);
    
    teste++;
    
    printf("entrou no func2 = %d\n", teste);
    
    cyield();
    
    csignal(sem1);

}

void* func0(void *arg) {
	printf("Eu sou a thread ID0 imprimindo %d\n", *((int *)arg));
	func2();
	return;
}

void* func1(void *arg) {
	printf("Eu sou a thread ID1 imprimindo %d\n", *((int *)arg));
	func2();
    return;
}


int main(int argc, char *argv[]) {
    sem1 = (csem_t*) malloc(sizeof(csem_t));

    csem_init(sem1, argc);

	int	id0, id1;
	int i;

	id0 = ccreate(func0, (void *)&i, 0);
	id1 = ccreate(func1, (void *)&i, 0);

	printf("Eu sou a main ap�s a cria��o de ID0 e ID1\n");

	cjoin(id0);
	cjoin(id1);

	printf("Eu sou a main voltando para terminar o programa\n");
	
	char nomes[2000];
	cidentify(nomes, 2000);
	
	printf("Identify: \n%s\n", nomes);
}

