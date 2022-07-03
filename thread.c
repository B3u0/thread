#include <pthread.h> 
#include <semaphore.h> 
#include <stdio.h>

/*Estado de los autos*/
#define EN_CAMINO 0	//Cuando esta en camino
#define CRUZAR    1	//Cuando intenta cruzar
#define EN_COLA   2	//Cuando esta en cola
#define CRUZO     3	//cuando ya cruzo

/*Los cuadrantes del cruce*/
#define NORTE 0 	//cuadrante 1
#define ESTE  1		//cuadrante 2
#define OESTE 2		//cuadrante 3
#define SUR   3		//cuadrante 4

/*Retardo en el cruce*/
#define RETARDO 2

int Cuadrante[4]={0,0,0,0};// 0= no hay autos pasando, 1= hay un auto pasando, la posicion indica el cuadrante o direccion en el cruce

/*La ley es un parametro que regula a los autos que pueden entrar al cruce que en este caso esta representado por el array Cuadrante[]
, Si la es Ley=0, significa que todos los autos que provengan de la direccion NORTE o SUR, pueden pasar y los demas no
, Si la es Ley=1, significa que todos los autos que provengan de la direccion Este u Oeste, pueden pasar y los demas no
*/ 
int Ley=0; 

sem_t Cruce[4];		//Es el semaforo del cruce para todos los cuadrantes

/*En esta funcion se accede al cruce, en el caso de que mucho hilos intenten entrar 4 de todas las direcciones por ejemplo,
sucedera lo siguiente, todos los hilos que no puedan pasar segun la ley seran puestos en cola,en caso contrario, si la direccion
a la que va el hilo ya esta siendo ocupada por otro sera puesto en cola, finalmente si no hay autos a los lados , el auto pasaria */
int cruce(int estado,int direccion,int automovil){
	if((direccion==NORTE || direccion== SUR)&& Ley){
			printf("Auto %d en cola, no puede cruzar \n", automovil);
			return EN_COLA;

	}
	else if((direccion==OESTE || direccion== ESTE)&& !Ley){
			printf("Auto %d en cola, no puede cruzar \n", automovil);
			return EN_COLA;
	}
	if(!Cuadrante[direccion]){
		if(direccion==NORTE || direccion== SUR)
		{
			if(!(Cuadrante[ESTE] || Cuadrante[OESTE])){
			Cuadrante[direccion]=1;
			printf("Estado del cruce para el auto %d, con direccion %d: Cruce=[ Norte: %d,Este: %d,Oeste: %d,SUR: %d] \n",automovil,direccion+1, Cuadrante[0],Cuadrante[1],Cuadrante[2],Cuadrante[3]);
			sleep(RETARDO);
			printf("Auto %d cruzo \n", automovil);
			Cuadrante[direccion]=0;			
			return CRUZO;
			}
			else{
			printf("Auto %d en cola, no puede cruzar \n", automovil);
			return EN_COLA;
			}
		}
		else
		{
			if(!(Cuadrante[NORTE] || Cuadrante[SUR])){
			Cuadrante[direccion]=1;
			printf("Estado del cruce para el auto %d, con direccion %d: Cruce=[ Norte: %d,Este: %d,Oeste: %d,SUR: %d] \n",automovil,direccion+1, Cuadrante[0],Cuadrante[1],Cuadrante[2],Cuadrante[3]);
			sleep(RETARDO);
			printf("Auto %d cruzo \n", automovil);
			Cuadrante[direccion]=0;			
			return CRUZO;
			}
			else{
			printf("Auto %d en cola, no puede cruzar \n", automovil);
			return EN_COLA;
			}
		}
	}
	else{
	printf("Auto %d en cola, no puede cruzar \n", automovil);
	return EN_COLA;
	}
}

/*Aqui los autos cruzan el cruce o esperan a que haya oportunidad de cruzar*/
void* carretera (void * num)
{
	int* p=num;												//numero del auto
	int estado=EN_CAMINO;											//estado del auto 
	int tiempo=rand()%11;											//tiempo de llegada al cruce, de 0 a 10 segundos, incluye el 0
	int direccion=rand()%4;											//direccion de auto o el cuadrante al que va
        printf("Auto %d llegando en %d segundos al cruce, por el cuadrante %d \n", *p , tiempo,direccion+1);	//se printea el numero del auto, el tiempo que demorara en llegar y el cuadrante al que va a ingresar
	while(!(estado==CRUZO))											//cruza solo si el estado cambio a Cruzo
	{
		if(tiempo>0){											//Si el tiempo es mayor a 0 el auto sigue en camino
		tiempo--;
		sleep(1);
		}
		else{
			if(estado==EN_CAMINO){									//Una vez acabado el tiempo de espera se en el if
			estado=CRUZAR;										//se cambia el estado a cruzar
			printf("Auto %d llego al cruce \n",*p);							//Se printe que se llego al cruce
			sem_wait(&Cruce[direccion]);								//se decrementa el semaforo para la direccion del auto, en caso de que este ocupado esperara a que se desocupe
			estado=cruce(estado,direccion,*p);							//entra en el cruce
			sem_post(&Cruce[direccion]);								//se libera el semaforo para esa direccion
			}
			else{
				if(estado==EN_COLA){								//En caso de que el auto no haya podido pasar por la ley, quedara en cola y entrara al if
				sleep(1);									//se detetiene 1 segundo para hacer tiempo mientras espera
				
				/*Intenta entrar al cruce denuevo*/
				sem_wait(&Cruce[direccion]);							
				estado=cruce(estado,direccion,*p);
				sem_post(&Cruce[direccion]);
				}
				else{
				
				printf("Auto %d en cola \n",*p);
				}
			}	
		}
	}

}

/*Esta funcion cambia cada 2*RETARDO[s] la Ley, la cual regula si pasaran autos de norte a sur o de este a oeste*/
void* control_cruce(void * param){
  sleep(5);
  while(1){
	if(!(Cuadrante[0] || Cuadrante[1] || Cuadrante[2] || Cuadrante[3])){		//revisa que todos los Cuadrantes esten en 0
		Ley=!Ley;
		sleep(RETARDO*2);
		Ley=!Ley;
		if(!(Cuadrante[0] || Cuadrante[1] || Cuadrante[2] || Cuadrante[3]))    // se asegura que ningun cuadrante este ocupado para asi poder terminar
		break;
		else continue;
	}
	else{
	sleep(RETARDO*2);
	Ley=!Ley;
	printf("la ley es %d\n", Ley);
	}	
  }
}

int main(int argc, char** argv)
{	
	/*Se declaran las variables a trabjar*/
	int i;			
	int N=atoi(argv[1]);					//Numero de autos
	int automovil[N];					//para guardar el numero del automovil

	/*Se declaran los hilos*/
	pthread_t thread_auto[N];				
	pthread_t ley_cruce;

	/*Se inicializan los semaforos*/
	sem_init(&Cruce[0], 0, 1);
	sem_init(&Cruce[1], 0, 1);
	sem_init(&Cruce[2], 0, 1);
	sem_init(&Cruce[3], 0, 1);

	/*hilo para la conjestion de autos*/
	char *param;
        pthread_create(&ley_cruce, NULL, control_cruce,&param); 	

        /* creación de hilos */
	for (i = 0; i < N; i++) {
	automovil[i]=i;
        printf("Auto %d partiendo\n", i);			//Aviso de que los autos parten
        pthread_create(&thread_auto[i], NULL, carretera, &automovil[i]);
    	}
 	for (i = 0; i < N; i++)
        pthread_join(thread_auto[i], NULL);
	return 1;
        pthread_join(ley_cruce, NULL);
	
}
