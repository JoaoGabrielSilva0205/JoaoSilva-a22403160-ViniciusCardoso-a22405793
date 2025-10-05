#include "mlfq.h"
#include "msg.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void mlfq_init(mlfq_t *mlfq) {
    for (int i = 0; i < MLFQ_LEVELS; i++) {
        mlfq->queues[i].head = NULL;
        mlfq->queues[i].tail = NULL;
    }
    // Time slices: nível 0 (maior prioridade) tem quantum menor
    mlfq->time_slices[0] = TIME_SLICE_MS;      // 500ms
    mlfq->time_slices[1] = TIME_SLICE_MS * 2;  // 1000ms
    mlfq->time_slices[2] = 0;                  // 0 = FIFO (sem preempção)
}

void mlfq_enqueue(mlfq_t *mlfq, pcb_t *task, int level) {
    if (level < 0) level = 0;
    if (level >= MLFQ_LEVELS) level = MLFQ_LEVELS - 1;
    
    enqueue_pcb(&mlfq->queues[level], task);
}

pcb_t* mlfq_dequeue(mlfq_t *mlfq) {
    // Procura da fila de maior prioridade (0) para a menor
    for (int i = 0; i < MLFQ_LEVELS; i++) {
        pcb_t *task = dequeue_pcb(&mlfq->queues[i]);
        if (task) return task;
    }
    return NULL;
}

void mlfq_scheduler(uint32_t current_time_ms, mlfq_t *mlfq, pcb_t **cpu_task) {
    if (*cpu_task) {
        pcb_t *current = *cpu_task;
        
        // Atualiza tempo decorrido
        uint32_t delta = current_time_ms - current->last_update_time_ms;
        current->ellapsed_time_ms += delta;
        current->last_update_time_ms = current_time_ms;
        
        // Verifica se terminou
        if (current->ellapsed_time_ms >= current->time_ms) {
            msg_t msg = {
                .pid = current->pid,
                .request = PROCESS_REQUEST_DONE,
                .time_ms = current_time_ms
            };
            if (write(current->sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t)) {
                perror("write");
            }
            free(current);
            *cpu_task = NULL;
        }
        // Verifica se expirou o quantum
        else if (current->current_level < MLFQ_LEVELS - 1) { // Não aplica ao último nível
            uint32_t quantum = mlfq->time_slices[current->current_level];
            if (current_time_ms - current->slice_start_ms >= quantum) {
                // Rebaixa para próxima fila (menor prioridade)
                current->current_level++;
                mlfq_enqueue(mlfq, current, current->current_level);
                *cpu_task = NULL;
            }
        }
    }
    
    // CPU livre? Escolhe próxima tarefa
    if (*cpu_task == NULL) {
        pcb_t *next = mlfq_dequeue(mlfq);
        if (next) {
            next->slice_start_ms = current_time_ms;
            next->last_update_time_ms = current_time_ms;
            *cpu_task = next;
        }
    }
}