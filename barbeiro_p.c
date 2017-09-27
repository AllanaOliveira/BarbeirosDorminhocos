#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//definimos 20 como o numero de clientes de cada tipo,
//ou seja, teremos um total de 60 clientes
#define N_CLIENTES 20

sem_t mutex;

sem_t cabelo;
sem_t cabelo_cortado;
sem_t barba;
sem_t barba_feita;
sem_t cor;
sem_t cabelo_pintado;

int fila = 0;

void* f_barbeiro_cabelereiro(void *v){
	while(1){
		sem_wait(&cabelo);		//espera um cliente que queira cortar o cabelo
		sem_wait(&mutex);		//entra na regiao critica
		fila--;				//decrementa a quantidade de pessoas na fila
		sem_post(&mutex);		//sai da regiao critica
		sem_post(&cabelo_cortado);	//sinaliza que o cabelo do cliente foi cortado
	}
	return NULL;
}

void* f_cliente_cabelo(void *v){
	int id = *(int*) v;
	sleep(id %5);							//sleep para as threads nao iniciarem todas juntas
	printf("Cliente %d chegando na barbearia para cortar o cabelo.\n", id);	//toda vez que um cliente chega uma mensagem eh impressa
	sem_wait(&mutex); 						//entra na regiao critica
	if(fila < 5){							//verifica se a fila ja esta cheia
		fila++;							//se nao, incrementa a fila(usuario permanece na barbearia)
		sem_post(&cabelo);					//sinaliza que tem um cliente para cortar o cabelo
		sem_post(&mutex);					//sai da regiao critica
		sem_wait(&cabelo_cortado);				//espera o sinal do barbeiro indicando que o cabelo foi cortado
		printf("Cliente %d saindo com cabelo cortado.\n", id);	//imprime mensagem indicando que o cliente esta saindo satisfeito
	}
	else{								//se a fila esta cheia
		sem_post(&mutex);					//sai da regiao critica
		printf("Cliente %d saindo insatisfeito.\n", id); 	//imprime mensagem dizendo que o cliente nao ficou na barbearia
	}
	return NULL;
}

//as funcoes de barbeiro e cliente seguintes sao analogas as primeira, 
//com o diferencial de que cada cliente eh de um tipo e cada barbeiro executa uma tarefa diferente

void* f_barbeiro_barbeiro(void *v){	
	while(1){
		sem_wait(&barba);
		sem_wait(&mutex);
		fila--;
		sem_post(&mutex);
		sem_post(&barba_feita);
	}
	return NULL;
}

void* f_cliente_barba(void *v){
	int id = *(int*) v;
	sleep(id % 5);
	printf("Cliente %d chegando na barbearia para fazer a barba.\n", id);
	sem_wait(&mutex);
	if(fila < 5){
		fila++;
		sem_post(&barba);
		sem_post(&mutex);
		sem_wait(&barba_feita);
		printf("Cliente %d saindo com a barba feita.\n", id);
	}
	else{
		sem_post(&mutex);
		printf("Cliente %d saindo insatisfeito.\n", id);
	}
	return NULL;
}

void* f_barbeiro_pintor(void *v){
	while(1){
		sem_wait(&cor);
		sem_wait(&mutex);
		fila--;
		sem_post(&mutex);
		sem_post(&cabelo_pintado);
	}
	return NULL;
}

void* f_cliente_cor(void *v){
	int id = *(int*) v;
	sleep(id % 5);
	printf("Cliente %d chegando na barbearia para pintar o cabelo.\n", id);
	sem_wait(&mutex);
	if(fila < 5){
		fila++;
		sem_post(&mutex);
		sem_post(&cor);
		sem_wait(&cabelo_pintado);
		printf("Cliente %d saindo com cabelo pintado.\n", id);
	}
	else{
		sem_post(&mutex);
		printf("Cliente %d saindo insatisfeito.\n", id);
	}
	return NULL;
}

int main() {

  printf("Barbeiros dormindo.\n");

  pthread_t thr_clientes[N_CLIENTES * 3], thr_barbeiro_1, thr_barbeiro_2, thr_barbeiro_3; //declara array de threads para os clientes e as threads dos barbeiros
  int i, id[N_CLIENTES * 3];    			//inicializa variaveis que serao auxiliares na identificacao do cliente

  sem_init(&mutex, 0, 1); 				//inicia o semaforo binario com 1 para exclusao mutua
  sem_init(&cabelo, 0, 0);				//inicia os semaforos
  sem_init(&cabelo_cortado, 0, 0);
  sem_init(&barba, 0, 0);
  sem_init(&barba_feita, 0, 0);
  sem_init(&cor, 0, 0);
  sem_init(&cabelo_pintado, 0, 0);
  
  for (i = 0; i < N_CLIENTES; i++) {			//cria as threads dos clientes que querem cortar o cabelo e passa um identificador
    id[i] = i;
    pthread_create(&thr_clientes[i], NULL, f_cliente_cabelo, (void*) &id[i]);
  }
  
  for (i = N_CLIENTES; i < 2 * N_CLIENTES; i++) {	//cria as threads dos clientes que querem fazer a barba e passa um identificador
	id[i] = i;
	pthread_create(&thr_clientes[i], NULL, f_cliente_barba, (void*) &id[i]);
  }
	
  for (i = 2 * N_CLIENTES; i < 3 * N_CLIENTES; i++) {	//cria as threads dos clientes que querem pintar o cabelo e passa um identificador
	id[i] = i;
	pthread_create(&thr_clientes[i], NULL, f_cliente_cor, (void*) &id[i]);
  }
	
  //cria threads dos barbeiros chamando suas respectivas funcoes

  pthread_create(&thr_barbeiro_1, NULL, f_barbeiro_cabelereiro, NULL);	
  pthread_create(&thr_barbeiro_2, NULL, f_barbeiro_barbeiro, NULL);
  pthread_create(&thr_barbeiro_3, NULL, f_barbeiro_pintor, NULL);
  
  for (i = 0; i < N_CLIENTES * 3; i++) 
    pthread_join(thr_clientes[i], NULL);

  printf("Fim do expediente!\n");
  return 0;
}
