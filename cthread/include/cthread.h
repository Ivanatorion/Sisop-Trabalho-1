/*
 * cthread.h: arquivo de inclusão com os protótipos das funções a serem
 *            implementadas na realização do trabalho.
 *
 * NÃO MODIFIQUE ESTE ARQUIVO.
 *
 * VERSÃO: 29/03/2019
 *
 */
#ifndef __cthread__
#define __cthread__

#include "support.h"

typedef struct s_sem {
	int	count;	/* indica se recurso está ocupado ou não (livre > 0, ocupado = 0) */
	PFILA2	fila; 	/* ponteiro para uma fila de threads bloqueadas no semáforo */
} csem_t;

/******************************************************************************
Parâmetros:
	start:	ponteiro para a função que a thread executará.
	arg:	um parâmetro que pode ser passado para a thread na sua criação.
	prio:	NÃO utilizado neste semestre, deve ser sempre zero.
Retorno:
	Se correto => Valor positivo, que representa o identificador da thread criada
	Se erro	   => Valor negativo.
******************************************************************************/
int ccreate (void* (*start)(void*), void *arg, int prio);

/******************************************************************************
Parâmetros:
	Sem parâmetros
Retorno:
	Quando executada corretamente: retorna 0 (zero)
	Caso contrário, retorna um valor negativo.
******************************************************************************/
int cyield(void);

/******************************************************************************
Parâmetros:
	tid: identificador da thread cuja prioridade será alterada (deixar sempre esse campo como NULL em 2018/02)
	prio: nova prioridade da thread.
Retorno:
	Quando executada corretamente: retorna 0 (zero)
	Caso contrário, retorna um valor negativo.
******************************************************************************/
int csetprio(int tid, int prio);

/******************************************************************************
Parâmetros:
	tid:	identificador da thread cujo término está sendo aguardado.
Retorno:
	Quando executada corretamente: retorna 0 (zero)
	Caso contrário, retorna um valor negativo.
******************************************************************************/
int cjoin(int tid);

/******************************************************************************
Parâmetros:
	sem:	ponteiro para uma variável do tipo csem_t. Aponta para uma estrutura de dados que representa a variável semáforo.
	count: valor a ser usado na inicialização do semáforo. Representa a quantidade de recursos controlados pelo semáforo.
Retorno:
	Quando executada corretamente: retorna 0 (zero)
	Caso contrário, retorna um valor negativo.
******************************************************************************/
int csem_init(csem_t *sem, int count);

/******************************************************************************
Parâmetros:
	sem:	ponteiro para uma variável do tipo semáforo.
Retorno:
	Quando executada corretamente: retorna 0 (zero)
	Caso contrário, retorna um valor negativo.
******************************************************************************/
int cwait(csem_t *sem);

/******************************************************************************
Parâmetros:
	sem:	ponteiro para uma variável do tipo semáforo.
Retorno:
	Quando executada corretamente: retorna 0 (zero)
	Caso contrário, retorna um valor negativo.
******************************************************************************/
int csignal(csem_t *sem);

/******************************************************************************
Parâmetros:
	name:	ponteiro para uma área de memória onde deve ser escrito um string que contém os nomes dos componentes do grupo e seus números de cartão.
		Deve ser uma linha por componente.
	size:	quantidade máxima de caracteres que podem ser copiados para o string de identificação dos componentes do grupo.
Retorno:
	Quando executada corretamente: retorna 0 (zero)
	Caso contrário, retorna um valor negativo.
******************************************************************************/
int cidentify (char *name, int size);


#endif

