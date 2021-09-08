#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include "../file_manager/manager.h"

int REPARTIDORES_LEFT;
int TIEMPO_DE_CREACION;
int* pids_repartidores; // posición
int N_REPARTIDORES;
int pids_semaforos[3];
int pid_fabrica;
int SEMAFOROS[3][3] = {
    {0,0,0},{0,0,0},{0,0,0}
    };  // Posición, tiempo de cambio, status
int pid_main;
int bool_create_repartidor;
int BODEGA;
int REPARTIDORES_FINISHED = 0;

void handle_repartidor_finished(int sig, siginfo_t *siginfo, void *context)
{
  int number_received = siginfo->si_value.sival_int;
  printf("[%i] FABRICA: RECIBÍ LA SEÑAL DEL REPARTIDOR CON PID: %d\n", getpid(), number_received);
  if (waitpid(number_received, NULL, 0) != -1) REPARTIDORES_FINISHED +=1;
  printf("[%i] FABRICA: Han llegado %d reparidores\n", getpid(), REPARTIDORES_FINISHED);
}


// HACEMOS UN HANDLER PARA CONTROLAR LOS COLORES DE LA FABRICA.
void handle_change_color_fabrica(int sig, siginfo_t *siginfo, void *context)
{
  int number_received = siginfo->si_value.sival_int;
  printf("[%i] FABRICA: CAMBIO DE COLOR DE SEMAFORO CON ID %d\n", getpid(), number_received);
  if (SEMAFOROS[number_received][2]==0) 
  {
    SEMAFOROS[number_received][2] = 1;
  }
  else {
    SEMAFOROS[number_received][2] = 0;
  }
  for (int i=0; i<3; i++) 
  {
    printf("Semaforo ID: %i tiene status %i\n", i+1, SEMAFOROS[i][2]);
  }

  for (int i = 0; i < N_REPARTIDORES-REPARTIDORES_LEFT; i++)
  {
    send_signal_with_int(pids_repartidores[i],number_received);
  }
}

void handle_delay_semaforo(int sig, siginfo_t *siginfo, void *context)
{
  int number_received = siginfo->si_value.sival_int;
  printf("[%i] FABRIC: RECIBÍ LA SEÑAL del %d\n", getpid(), number_received);
  wait(NULL);
  printf("[%i] FABRIC: Terminé de esperar al hijo %d\n", getpid(), number_received);
}

void handle_sigabrt_fabrica()
{
  printf("[%i] Soy el proceso fábrica y me llegó un SIGABRT\n",getpid());
  int por_matar = N_REPARTIDORES-REPARTIDORES_LEFT-REPARTIDORES_FINISHED+1;
  while (por_matar >= 0) {
    kill(pids_repartidores[por_matar + REPARTIDORES_FINISHED],SIGABRT);
    waitpid(pids_repartidores[por_matar + REPARTIDORES_FINISHED], NULL, 0); 
    por_matar -= 1;
  }
  free(pids_repartidores);
  printf("[%i] FABRICA: LIBERO PIDS\n", getpid());
  //Le aviso a main que terminé
  kill(pid_main, SIGABRT);
  exit(0);
}

void handle_sigabrt_main()
{
printf("[%i] Soy el proceso main y me llegó un SIGABRT\n", getpid());
  for (int i = 0; i < 3; i++)
  {
    kill(pids_semaforos[i], SIGABRT);
    wait(NULL);
  }
  free(pids_repartidores);
  printf("[%i] MAIN: LIBERO PIDS\n", getpid());
  exit(0);
}

void handle_sigint_main()
{
  printf("[%i] Soy el proceso main y me llegó un SIGINT\n",getpid());
  kill(pid_fabrica, SIGABRT);
  wait(NULL);
  // Al terminar el proceso fábrica, tira sigabort a main
}

void handle_sigint_fabrica()
{
  printf("Soy el proceso fabrica y me llegó un SIGINT\n");
}

void handle_sigalarm_main()
{
  kill(getpid(), SIGINT);
}


void create_semaforo(int* semaforo, int id) 
{
  printf("[%i]CREATE SEMAFORO %i\n", getpid(), id);
  pid_t pid = fork();
  if (!pid) {
    free(pids_repartidores);
    printf("[%i]HIJO-SEMAFORO: LIBERO PIDS\n", getpid());
    char s_id[10];
    sprintf(s_id, "%d", id);
    char s_del[10];
    sprintf(s_del, "%d", semaforo[1]);
    char s_pid[10];
    sprintf(s_pid, "%d", pid_fabrica);
    execlp("./semaforo",s_id, s_del, s_pid, NULL);
  }
  else if (pid > 0) {
    pids_semaforos[id-1] = pid;
  }
  else {
    printf("Pedazo de error\n");
    exit(1);
  }
}

void handle_alarm()
{
  bool_create_repartidor = 1;
}

void create_repartidor()
{
  printf("REPARTIDORES LEFT: %i\n", REPARTIDORES_LEFT);
  REPARTIDORES_LEFT -= 1;
  if (REPARTIDORES_LEFT > 0)
  {
    alarm(TIEMPO_DE_CREACION);
  }
  else {
    printf("NO QUEDAN REPARTIDORES POR CREAR!\n");
    bool_create_repartidor = -1;
  }
  int index = N_REPARTIDORES-(REPARTIDORES_LEFT+1);
  pid_t pid = fork();
  if (!pid) {
    free(pids_repartidores);
    printf("[%i] REPARTIDOR: LIBERO PIDS\n", getpid());
    char s1pos[10];
    sprintf(s1pos, "%d", SEMAFOROS[0][0]);
    char s2pos[10];
    sprintf(s2pos, "%d", SEMAFOROS[1][0]);
    char s3pos[10];
    sprintf(s3pos, "%d", SEMAFOROS[2][0]);
    char s1stat[10];
    sprintf(s1stat, "%d", SEMAFOROS[0][2]);
    char s2stat[10];
    sprintf(s2stat, "%d", SEMAFOROS[1][2]);
    char s3stat[10];
    sprintf(s3stat, "%d", SEMAFOROS[2][2]);
    char bodega_string[10];
    sprintf(bodega_string, "%d", BODEGA);
    char index_string[10];
    sprintf(index_string, "%d", index);
    char fabrica_string[10];
    sprintf(fabrica_string, "%d", pid_fabrica);
    char* argv[10] = {s1pos, s1stat, s2pos, s2stat, s3pos, s3stat, bodega_string, index_string, fabrica_string ,NULL};
    execvp("./repartidor", argv);
  }
  else {
    connect_sigaction(SIGUSR2, handle_repartidor_finished);
    connect_sigaction(SIGUSR1, handle_change_color_fabrica);
    pids_repartidores[index] = pid;
  }

}

void create_fabric(int N_REPARTIDORES)
{
  printf("[%i] FABRIC\n", getpid());
  pid_t pid = fork();
  if (!pid) {
    signal(SIGABRT, handle_sigabrt_fabrica);
    signal(SIGINT, handle_sigint_fabrica);
    pid_fabrica = getpid();
    printf("[%d] FABRICA: I'm the FABRIC process and my PID is: %d\n",getpid() ,getpid());
    printf("[%d] FABRICA: Mi padre (MAIN) es %i\n",getpid(), getppid());
    REPARTIDORES_LEFT = N_REPARTIDORES;
    connect_sigaction(SIGALRM, handle_alarm);
    connect_sigaction(SIGUSR2, handle_repartidor_finished);
    alarm(TIEMPO_DE_CREACION);
    while (bool_create_repartidor > -1)
    {
      if (bool_create_repartidor == 1)
      {
        bool_create_repartidor = 0;
        create_repartidor();
      }
      if (bool_create_repartidor == -1) {
      }
    };
    while (REPARTIDORES_FINISHED < N_REPARTIDORES)
    {
      if (wait(NULL)!=-1) REPARTIDORES_FINISHED +=1;
      printf("Han muerto %i repartidores\n", REPARTIDORES_FINISHED);
    };
    kill(pid_main, SIGABRT);
    free(pids_repartidores);
    printf("[%i] FABRICA: LIBERO PIDS\n", getpid());
    exit(0);
  }
  else if (pid > 0) {
    pid_fabrica = pid;
  }
  else {
    printf("Pedazo de error\n");
    exit(1);
  }
}


int main(int argc, char const *argv[])
{
  printf("I'm the DCCUBER process and my PID is: %i\n", getpid());
  pid_main = getpid();
  char *filename = "input.txt";
  InputFile *data_in = read_file(filename);

  for (int i = 0; i < 3; i++)
  {
    SEMAFOROS[i][0] = atoi(data_in->lines[0][i]);
  }
  BODEGA = atoi(data_in->lines[0][3]);
  TIEMPO_DE_CREACION = atoi(data_in->lines[1][0]);
  N_REPARTIDORES = atoi(data_in->lines[1][1]);
  pids_repartidores = (int *) malloc(N_REPARTIDORES* sizeof(int));

  for (int i=2; i<5; i++)
  {
    SEMAFOROS[i-2][1] = atoi(data_in->lines[1][i]);
  }
  input_file_destroy(data_in);
  create_fabric(N_REPARTIDORES);
  signal(SIGABRT, handle_sigabrt_main);
  signal(SIGINT, handle_sigint_main);
  signal(SIGALRM, handle_sigalarm_main);
  for (int i = 0; i < 3; i++) 
  {
    create_semaforo(SEMAFOROS[i],i+1);
  }
  while (1)
  {
  };
}