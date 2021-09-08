#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../file_manager/manager.h"

// SIGABRT: ESCRIBIR OUTPUT Y EXIT(0)i<

int status = 1;
int id;
int delay;
int conteo;
int pid_fabrica;
int bool_change_color;


void change_color(){
  printf("[%i] SEMAFORO CON ID %i CAMBIE DE COLOR\n",getpid() , id);
  status = !status;
  if (status){
    union sigval sig = {};
    sig.sival_int = id-1;
    sigqueue(pid_fabrica, SIGUSR1, sig);
    //printf("[%i] Mandé señal del cambio a %i\n",getpid() ,pid_fabrica);
  }
  if (!status){
    union sigval sig = {};
    sig.sival_int = id-1;
    sigqueue(pid_fabrica, SIGUSR1, sig);
    //printf("[%i] Mandé señal del cambio a %i\n",getpid() ,pid_fabrica);
  }
  conteo += 1;
  alarm(delay);
}

void handle_sigint()
{
  printf("[%i] Soy el proceso semaforo y me llegó un SIGINT\n",getpid());
}

void handle_alarm()
{
  bool_change_color = 1;
}

void handle_sigabrt_semaforo(){
  printf("[%i] Soy el proceso semaforo y me llegó un SIGABRT\n",getpid());
  const char* fileName = "semaforo";
  const char* fileType = ".txt";
  char name_buffer[512];
  FILE* f = NULL;
  sprintf(name_buffer,"%s%d%s",fileName,id,fileType);
  f = fopen(name_buffer,"w");
  fprintf(f, "%d", conteo);
  fclose(f);
  exit(0);
}


int main(int argc, char const *argv[])
{
  signal(SIGINT, handle_sigint);
  signal(SIGABRT, handle_sigabrt_semaforo);
  connect_sigaction(SIGALRM, handle_alarm);
  id = atoi(argv[0]);
  delay = atoi(argv[1]);
  pid_fabrica = atoi(argv[2]);
  //printf("ID FABRICA %s: %i\n\n\n",argv[2] ,pid_fabrica);
  alarm(delay);
  pid_t pid = getpid();
  printf("[%i] EXEC: I'm the SEMAFORO process and my PID is: %i\n",getpid() ,pid);
  printf("[%i] EXEC: PROCESO-SEMAFORO: Mi padre es el %i\n",getpid() ,getppid());
  /* union sigval sig = {};
  sig.sival_int = pid;
  sigqueue(getppid(), SIGUSR1, sig); */
  while (1) {
    if (bool_change_color == 1)
    {
      change_color();
      bool_change_color = 0;
    }
  }
    ;
}
