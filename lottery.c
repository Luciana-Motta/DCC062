/*
*  lottery.c - Implementacao do algoritmo Lottery Scheduling e sua API
*
*  Autores: SUPER_PROGRAMADORES_C
*  Projeto: Trabalho Pratico I - Sistemas Operacionais
*  Organizacao: Universidade Federal de Juiz de Fora
*  Departamento: Dep. Ciencia da Computacao
*
*/

#include "lottery.h"
#include <stdio.h>
#include <string.h>


//Nome unico do algoritmo. Deve ter 4 caracteres.
const char lottName[]="LOTT";
int total_tickets = 0;
int slot = -1; //slot para este caso será sempre zero
//=====Funcoes Auxiliares=====





//=====Funcoes da API=====

//Funcao chamada pela inicializacao do S.O. para a incializacao do escalonador
//conforme o algoritmo Lottery Scheduling
//Deve envolver a inicializacao de possiveis parametros gerais
//Deve envolver o registro do algoritmo junto ao escalonador
void lottInitSchedInfo() {
    //Aloca a estrutura SchedInfo
    SchedInfo *sched_info = malloc(sizeof(SchedInfo));

    //Nomeia o escalonador do tipo lottery
	memcpy(sched_info->name,lottName,sizeof lottName);

    //Acoplamento das funções necessarias para o escalonador funcionar no modo lottery
	sched_info->initParamsFn = &lottInitSchedParams;
	sched_info->notifyProcStatusChangeFn = &lottNotifyProcStatusChange;
	sched_info->scheduleFn = &lottSchedule;
	sched_info->releaseParamsFn = &lottReleaseParams;

    //Define o número do tipo lottery entre os escalonadores possiveis
    //Adiciona a posição do escalonador as caracteristicas do mesmo
    slot = schedRegisterScheduler(sched_info);
}

//Inicializa os parametros de escalonamento de um processo p, chamada
//normalmente quando o processo e' associado ao slot de Lottery
void lottInitSchedParams(Process *p, void *params) {

    //Associa um processo a um algoritmo de escalonamento especifico
    schedSetScheduler(p, params,slot);

    LotterySchedParams *lotter;
	lotter = processGetSchedParams(p);
}

//Recebe a notificação de que um processo sob gerência de Lottery mudou de estado
//Deve realizar qualquer atualização de dados da Loteria necessária quando um processo muda de estado
void lottNotifyProcStatusChange(Process* p) {
	// A schedNotifyProcStatusChange(p) recebe uma notificação de que o estatus do processo p foi modificado e ela chama a função
	// lottNotifyProcStatusChange para modificar os dados da dados necessarios quando o estado muda

    //pega o status do processo p
    int status = processGetStatus(p);

    //pega os parametros do lottery_params
    LotterySchedParams *lotter;
    //Atualiza o estatus do processo dentro do lottery
    lotter = processGetSchedParams(p);
    //STAT:(status)	0: processo inicializado
	//			2: processo bloqueado
	//			4: processo pronto
	//			8: processo em execução
	//			16:processo finalizado

	//se o processo sai de bloqueado para pronto ou de em execução para pronto, é necessário adicionar o seu número de tikets ao total
    if (status == 0 || status == 4){
        total_tickets += lotter->num_tickets;
    }

    // se o processo é bloqueado, em execução ou finalizado, é necessario retirar o seu número de tikets do total
    if (status == 2 || status == 8 || status == 16) //PROBLEMAAAAAAA(acho que tem problema no plist)
        total_tickets = total_tickets - lotter->num_tickets;

}

//Retorna o proximo processo a obter a CPU, conforme o algortimo Lottery
Process* lottSchedule(Process *plist) {
    //Criando um novo processo para receber o processo sorteado
	Process *proximo = NULL;
	//criando uma variavel auxiliar para sortear um número entre 1 e 1000
	unsigned int ticket_premiado;
	ticket_premiado = (rand()%total_tickets);


	LotterySchedParams *lotter;
	int maximo = 0;
	int minimo = 0;
	Process *aux;
    for (aux = plist; aux != NULL; aux = processGetNext(aux))
	{
		if (processGetStatus(aux) == 4)
		{
			lotter = processGetSchedParams(aux);
			maximo = lotter->num_tickets + minimo;
			if (minimo <= ticket_premiado && ticket_premiado < maximo)
			{
				proximo = aux;
			}
			minimo += maximo;
		}
	}

    return proximo;

}

//Libera os parametros de escalonamento de um processo p, chamada
//normalmente quando o processo e' desassociado do slot de Lottery
//Retorna o numero do slot ao qual o processo estava associado
int lottReleaseParams(Process *p) {

    //Salva o numero do slot está associado ao processo p
	int slot = processGetSchedSlot(p);
	//criamos um parametro do tipo void nulo
    void* sched_null = NULL;
    //seta os parametros relacionados ao escalonador para nulo
    processSetSchedParams(p, sched_null);
    //retorna o valor salvo do slot anteriormente
	return slot;
}

//Transfere certo numero de tickets do processo src para o processo dst.
//Retorna o numero de tickets efetivamente transfeirdos (pode ser menos)
int lottTransferTickets(Process *src, Process *dst, int tickets) {

	LotterySchedParams *lotter_a;
	lotter_a = processGetSchedParams(src);
	if (lotter_a->num_tickets <= tickets)
        tickets = lotter_a->num_tickets;
    lotter_a->num_tickets -= tickets;

    LotterySchedParams *lotter_b;
	lotter_b = processGetSchedParams(dst);
	lotter_b->num_tickets += tickets;

	return tickets;
}
