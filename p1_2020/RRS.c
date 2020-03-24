#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>
#include "my_io.h"

//#include "mythread.h"
#include "interrupt.h"

#include "queue.h"

TCB* scheduler();
void activator();
void timer_interrupt(int sig);
void disk_interrupt(int sig);


/* Array of state thread control blocks: the process allows a maximum of N threads */
static TCB t_state[N];

/* Current running thread */
static TCB* running;
static int current = 0;

/* Last run thread*/
static TCB* old_running;


/*Queue with the ready threads. One queue for high priority and other for low priority*/
static struct  queue *high_ready_list;
static struct queue *low_ready_list;

/* Variable indicating if the library is initialized (init == 1) or not (init == 0) */
static int init = 0;

/* Thread control block for the idle thread */
static TCB idle;

static void idle_function()
{
  while(1);
}

void function_thread(int sec)
{
    //time_t end = time(NULL) + sec;
    while(running->remaining_ticks)
    {
      //do something
    }
    mythread_exit();
}


/* Initialize the thread library */
void init_mythreadlib()
{
  int i;

  /* Initialize both queues*/
   high_ready_list= queue_new  ();
   low_ready_list = queue_new();

  /* Create context for the idle thread */
  if(getcontext(&idle.run_env) == -1)
  {
    perror("*** ERROR: getcontext in init_thread_lib");
    exit(-1);
  }

  idle.state = IDLE;
  idle.priority = SYSTEM;
  idle.function = idle_function;
  idle.run_env.uc_stack.ss_sp = (void *)(malloc(STACKSIZE));
  idle.tid = -1;

  if(idle.run_env.uc_stack.ss_sp == NULL)
  {
    printf("*** ERROR: thread failed to get stack space\n");
    exit(-1);
  }

  idle.run_env.uc_stack.ss_size = STACKSIZE;
  idle.run_env.uc_stack.ss_flags = 0;
  idle.ticks = QUANTUM_TICKS;
  makecontext(&idle.run_env, idle_function, 1);

  t_state[0].state = INIT;
  t_state[0].priority = LOW_PRIORITY;
  t_state[0].ticks = QUANTUM_TICKS;

  if(getcontext(&t_state[0].run_env) == -1)
  {
    perror("*** ERROR: getcontext in init_thread_lib");
    exit(5);
  }

  for(i = 1; i < N; i++)
  {
    t_state[i].state = FREE;
  }

  t_state[0].tid = 0;
  running = &t_state[0];

  /* Initialize disk and clock interrupts */
  init_disk_interrupt();
  init_interrupt();
}


/* Create and intialize a new thread with body fun_addr and one integer argument */
int mythread_create (void (*fun_addr)(), int priority, int seconds)
{
  int i;

  if (!init) { init_mythreadlib(); init = 1;}

  for (i = 0; i < N; i++)
    if (t_state[i].state == FREE) break;

  if (i == N) return(-1);

  if(getcontext(&t_state[i].run_env) == -1)
  {
    perror("*** ERROR: getcontext in my_thread_create");
    exit(-1);
  }

  t_state[i].state = INIT;
  t_state[i].priority = priority;
  t_state[i].function = fun_addr;
  t_state[i].execution_total_ticks = seconds_to_ticks(seconds);
  t_state[i].ticks = QUANTUM_TICKS;
  t_state[i].remaining_ticks = t_state[i].execution_total_ticks;
  t_state[i].run_env.uc_stack.ss_sp = (void *)(malloc(STACKSIZE));

  if(t_state[i].run_env.uc_stack.ss_sp == NULL)
  {
    printf("*** ERROR: thread failed to get stack space\n");
    exit(-1);
  }

  t_state[i].tid = i;
  t_state[i].run_env.uc_stack.ss_size = STACKSIZE;
  t_state[i].run_env.uc_stack.ss_flags = 0;
  makecontext(&t_state[i].run_env, fun_addr,2,seconds);
  TCB *padentro = &t_state[i];
  disable_interrupt();
  disable_disk_interrupt();

  //We introduce the newly created thread in the corresponding queue
  // High priority: inserted sorted according to the total execution time (SJF)
  // Low priority: inserted according to arrival order (FIFO)
  if (t_state[i].priority == HIGH_PRIORITY) sorted_enqueue(high_ready_list,padentro, t_state[i].execution_total_ticks);
  else enqueue(low_ready_list,padentro);

  enable_disk_interrupt();
  enable_interrupt();

  return i;
}
/****** End my_thread_create() ******/


/* Read disk syscall */
int read_disk()
{
   return 1;
}

/* Disk interrupt  */
void disk_interrupt(int sig)
{

}


/* Free terminated thread and exits */
void mythread_exit() {
  old_running = running;
  int tid = old_running->tid;
  t_state[tid].state = FREE;
  free(t_state[tid].run_env.uc_stack.ss_sp);
  running = scheduler();

  //Scheduler() can finish the execution of the problem, so we might not come here
  running->state = RUNNING;

  //Swap context to next thread
  activator(running);
}


void mythread_timeout(int tid) {

    printf("*** THREAD %d EJECTED\n", tid);
    t_state[tid].state = FREE;
    free(t_state[tid].run_env.uc_stack.ss_sp);

    TCB* next = scheduler();
    activator(next);
}


/* Sets the priority of the calling thread */
void mythread_setpriority(int priority)
{
  int tid = mythread_gettid();
  t_state[tid].priority = priority;
  if(priority ==  HIGH_PRIORITY){
    t_state[tid].remaining_ticks = 195;
  }
}

/* Returns the priority of the calling thread */
int mythread_getpriority(int priority)
{
  int tid = mythread_gettid();
  return t_state[tid].priority;
}


/* Get the current thread id.  */
int mythread_gettid(){
  if (!init) { init_mythreadlib(); init = 1;}
  return current;
}


/* SJF para alta prioridad, RR para baja */
TCB* scheduler()
{

  disable_interrupt();
  disable_disk_interrupt();

  if (queue_empty(high_ready_list)) {
    if (queue_empty(low_ready_list)) {
      //If both queues are empty, we have finish the problem
      enable_disk_interrupt();
      enable_interrupt();
      printf("*** THREAD %d FINISHED\n", old_running->tid);
      printf("\nFINISH\n");
      exit(1);
    }

    //If high-prio queue is empty but low-prio is not, we take the first low-pri thread is returned
    else {
      TCB *process = dequeue(low_ready_list);
      enable_disk_interrupt();
      enable_interrupt();
      return process;
    }
  }

  //If there are threads in the high-prio queue
  else {
    TCB *process = dequeue(high_ready_list);
    enable_disk_interrupt();
    enable_interrupt();
    return process;
  }
}


/* Timer interrupt */
void timer_interrupt(int sig){
  running->ticks -= 1;
  running->remaining_ticks -= 1;

  //IF thread finishes its number of ticks, we end it
  if(running->remaining_ticks == 0 ){
    mythread_exit();
  }

  if (!queue_empty(high_ready_list)){//high-prio queue not empty
    if (running->priority == LOW_PRIORITY){
      //Save the context of the low priority thread and run the high priority one
      running->state = INIT;
      running->ticks = QUANTUM_TICKS;
      disable_interrupt();
      disable_disk_interrupt();

      //We store the thread in our queue
      enqueue(low_ready_list,running);
      old_running = running;

      //Call for the next thread to come
      running = scheduler();
      running->state = RUNNING;

      enable_disk_interrupt();
      enable_interrupt();

      //Swap context
      activator(running);
    }
    else if (running->priority == HIGH_PRIORITY){
      /*IF the current high-pri thread needs more time to execute than the first thread in the
       high_ready_queue (the one with sortest execution time) then the running thread is enqueued and the ready one is set to run
      */
      if (running->remaining_ticks > high_ready_list->head->sort){
        running->state = INIT;
        running->ticks = QUANTUM_TICKS;
        disable_interrupt();
        disable_disk_interrupt();

        //We store the thread in the high-pri queue, sorted by its remaining execution time
        sorted_enqueue(high_ready_list, running, running->remaining_ticks);
        old_running = running;

        //Call for the next thread to come
        running = scheduler();
        running->state = RUNNING;

        enable_disk_interrupt();
        enable_interrupt();

        //Swap context
        activator(running);
      }

    }
  }
  //If high-prio queue is empty
  //If a low priority thread is running AND its slice ends
  else if(running->priority == LOW_PRIORITY && running->ticks == 0){
    //Save the context of the low priority thread and run the high priority one
    running->state = INIT;
    running->ticks = QUANTUM_TICKS;
    disable_interrupt();
    disable_disk_interrupt();

    //We store the thread in our queue
    enqueue(low_ready_list,running);
    old_running = running;

    //Call for the next thread to come
    running = scheduler();
    running->state = RUNNING;

    enable_disk_interrupt();
    enable_interrupt();

    //Swap context
    activator(running);
  }
}

/* Activator */
void activator(TCB* next)
{
  switch (old_running->state)
  {
  case INIT:
    /* If both threads have the same priority normal message will be displayed*/
    if(old_running->priority == next->priority){
      printf("*** SWAPCONTEXT FROM %d TO %d\n", old_running->tid, next->tid);
    }
    /* The only remaining case is that old = LOW & next = HIGH
    Because the case of old = HIGH & next = LOW only will possible when the HIGH-prio thread
    has finished, and that is chased with case FREE */

    /*old->priority   next->priority    message
        L                   L           SWAPCONTEXT
        L                   H           PREEMPTED
        H                   L           FINISHED
        H                   H           SWAPCONTEXT
    */
    else {
      printf("*** THREAD %d PREEMPTED: SET CONTEXT OF %d\n",old_running->tid, next->tid);
    }

    //swapcontext returns -1 on error
    if(swapcontext (&(old_running->run_env), &(next->run_env))) perror("Not possible to swap context");
    break;

  case FREE:
    printf("*** THREAD %d FINISHED: SET CONTEXT OF %d\n", old_running->tid, next->tid);
    //setcontext returns -1 on error
    if(setcontext(&(next->run_env))) perror("Not possible to swap context");
    printf("mythread_free: After setcontext, should never get here!!...\n");
    break;

  case IDLE:
    printf("*** THREAD READY: SET CONTEXT TO %d\n", next->tid);
    break;

  default:
    break;
  }
}
