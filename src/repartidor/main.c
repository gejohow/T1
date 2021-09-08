#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include "../file_manager/manager.h"


// SIGABRT: ESCRIBIR OUTPUT Y EXIT(0)
int posicion = 0;
int turno = 0;
int bodega;
int semaforos[3][2];
int id;
int semaforos_restantes = 3;
int pid_fabrica;

void escribir_archivo_repartidor(){
  const char* fileName = "repartidor_";
  const char* fileType = ".txt";
  char name_buffer[512];
  FILE* f = NULL;
  sprintf(name_buffer,"%s%d%s",fileName,id,fileType);
  f = fopen(name_buffer,"a");
  fprintf(f, "%d,", turno);
  fclose(f);
}

void handle_walk()
{
  int avanzar = 0;
  if (posicion != bodega)
  {
    alarm(1);
  }
  else 
  {
    printf("[%i] Repartidor  %i llegó!\n",getpid(), id);
    const char* fileName = "repartidor_";
    const char* fileType = ".txt";
    char name_buffer[512];
    FILE* f = NULL;
    sprintf(name_buffer,"%s%d%s",fileName,id,fileType);
    f = fopen(name_buffer,"a");
    fprintf(f, "%d", turno);
    fclose(f);
    kill(getpid(), SIGABRT);
  }
  turno +=1;
  for (int i=0; i<3; i++) 
  {
    if (posicion+1 == semaforos[i][0])
      {
        if (semaforos[i][1])
        {
          avanzar = 1;
          semaforos_restantes -= 1;
          escribir_archivo_repartidor();
        }
        else 
        {
          avanzar = -1;
        }
      }
  }
  if (avanzar >=0)
  {
    posicion +=1;
  }
  printf("[%i] EN POSICIÓN %i, TURNO %i\n", getpid(), posicion, turno);
}

void handle_sigabrt_repartidor(){
  printf("[%i]Repartidor %i: Me llegó SIGABORT\nPOSICIÓN: %i\nTURNO: %i\n",getpid(),id ,posicion, turno);
  if (semaforos_restantes > 0){
    const char* fileName = "repartidor_";
    const char* fileType = ".txt";
    char name_buffer[512];
    FILE* f = NULL;
    sprintf(name_buffer,"%s%d%s",fileName,id,fileType);
    f = fopen(name_buffer,"a");
    for (int i=0; i<semaforos_restantes; i++) {
      fprintf(f, "%d,", -1);
    }
  }
  if ((posicion != bodega)){
    const char* fileName = "repartidor_";
    const char* fileType = ".txt";
    char name_buffer[512];
    FILE* f = NULL;
    sprintf(name_buffer,"%s%d%s",fileName,id,fileType);
    f = fopen(name_buffer,"a");
    fprintf(f, "%d", -1);
  }
  union sigval sig = {};
  sig.sival_int = getpid();
  sigqueue(pid_fabrica, SIGUSR2, sig);
  //kill(pid_fabrica, SIGUSR2);
  printf("[%i] ENVIÉ SEÑAL A FÁBRICA %i\n",getpid(),pid_fabrica);
  exit(0);
}

void handle_change_color(int sig, siginfo_t *siginfo, void *context){
  int sem_id = siginfo->si_value.sival_int;
  if (semaforos[sem_id][1]==1) {
    semaforos[sem_id][1] = 0;
  }
  else {
    semaforos[sem_id][1] = 1;
  }
}

void handle_sigint()
{
  printf("[%i] Soy el proceso repartidor y me llegó un SIGINT\n",getpid());
}

int main(int argc, char const *argv[])
{
  connect_sigaction(SIGALRM, handle_walk);
  connect_sigaction(SIGUSR1, handle_change_color);
  signal(SIGINT, handle_sigint);
  signal(SIGABRT, handle_sigabrt_repartidor);
  printf("[%i] EXEC: PROCESO-REPARTIDOR: Mi padre %i\n",getpid() ,getppid());
  int s1pos = atoi(argv[0]);
  int s1stat = atoi(argv[1]);
  int s2pos = atoi(argv[2]);
  int s2stat = atoi(argv[3]);
  int s3pos = atoi(argv[4]);
  int s3stat = atoi(argv[5]);
  bodega = atoi(argv[6]);
  id = atoi(argv[7]);
  pid_fabrica = atoi(argv[8]);
  semaforos[0][0] = s1pos;
  semaforos[0][1] = s1stat;
  semaforos[1][0] = s2pos;
  semaforos[1][1] = s2stat;
  semaforos[2][0] = s3pos;
  semaforos[2][1] = s3stat;
  alarm(1);
  while (1){
    sleep(1);
  };
  
}