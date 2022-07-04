#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>

/*La funcion que encuentra los numeros primos*/
void criba(unsigned int*, int);

int main(int argc, char **argv){
	/*Se verifica que se ha ingresado los parametros*/
	if (argc <= 1) { 
		printf("No ha ingresado parametros\n");
		return -1;  
	}
	/*Se crean las variables i para los for y N que contendra
	 el numero ingresado*/
	int i,N;
	N=atoi(argv[1]);   //se convierte el parametro ingresado en entero
	int m[N-1];		// declaro un arreglo para guardar los enteros entre 2 y N
	for(i = 2; i <= N; ++i){ //se guardan
	m[i-2]=i;
	}

	/*Se crea la memoria compatida para luego almacenar el arreglo m*/
	const int SIZE = sizeof(m);
	const char *name = "noprimos";
	int shm_fd;
	unsigned int *ptr;
	shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
	ftruncate(shm_fd,SIZE);
	ptr =(unsigned int*) mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	/*Se revisa que no haya errores*/
	if (ptr == MAP_FAILED) {
		printf("Map failed\n");
		return -1;
	}	

	//escribiendo en memoria compartida
	for(i = 2; i <= N; ++i){ 
	*ptr=m[i-2];
	ptr += sizeof(int);
	} 

	//creando proceso hijo
	pid_t pid1;
	int status1;
	pid1=fork();
	if(pid1<0){ //se revisa que el proceso hijo se haya hecho bien
		fprintf(stderr, "Fork failed");
		return -1;
	}
	
	/*PROCESO HIJO*/
	if (pid1 == 0) {
	/*Se obtiene el puntero de la memoria compartida*/
	ptr =(unsigned int *) mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	N=SIZE/sizeof(int);// se calcula el porte del arreglo
	criba(ptr,N);	   //La funcion criba encontrara los numeros primos y borrara los no primos en la memoria compartida, dejara solo los numeros primos en la memoria compartida

		
	}
	else{
	/*PROCESO PADRE*/
	waitpid(pid1, &status1, WUNTRACED); //espera a que el hijo termine evitando proceso huerfanos o zombies
	unsigned int *primos =(unsigned int*) mmap(0,SIZE, PROT_READ, MAP_SHARED, shm_fd, 0); //se pide un puntero a la memoria compartida
	/*Se printean los numeros primos que se encuentren en la memoria compartida*/
	printf("Los numeros primos son:\n");
	for(i = 0; *(primos+i*4)!=0; ++i)printf("%d \n",*(primos+i*4));

	/* Por ultimo se remueve la memoria compatida y se ve si hay error */
	if (shm_unlink(name) == -1) {
		printf("Error removing %s\n",name);
		exit(-1);
	 }
	}
return 0;
}

void criba(unsigned int*nums , int n){
/*Esta funcion encontrara los numeros primos*/
unsigned int i,x=0,p,prims[n];
for(i=0;i<=n;i++) {
       	if(*(nums+i*4) != 0 || *(nums+i*4)  == 2) {
        	prims[x] =*(nums+i*4);
            	for(p=2;(p*(i+2))<=n+1;p++) {
                	*( nums+((p*(i+2))-2)*4 ) = 0;
            	}
           	 x++;
        	}
	    }
/*En esta parte se dejan solo los nuemros primos en la meoria compartida*/
for(i=0;i<x;i++) *(nums+i*4)=prims[i];
for(i=x;i<=n;i++) *(nums+i*4)=0;

}

