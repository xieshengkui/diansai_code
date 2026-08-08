#ifndef QUESTION_TASK_H
#define QUESTION_TASK_H

#include "headfile.h"
void question1_task(void);
void question2_task(void);
void question3_task(void);
void question4_task(void);

typedef struct{
    int Main_State;
    int Q1_State;
    int Q2_State;
    int Q3_State;
    int Q4_State;
} state_machine;

#define STOP_STATE 0
#define Q1_STATE 1
#define Q2_STATE 2
#define Q3_STATE 3
#define Q4_STATE 4

extern uint8_t q1_first_flag;
extern uint8_t q2_first_flag;
extern uint8_t q3_first_flag;
extern uint8_t q4_first_flag;
extern state_machine STATE_MACHINE;

void question_task_init(void);


#endif
