#include "rr.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "msg.h"
#include "queue.h"

void rr_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task) {
    const uint32_t QUANTUM_MS = 500;

    if (*cpu_task) {
        pcb_t *current = *cpu_task;

        // Acumula CPU pelo delta desde a última atualização
        uint32_t delta = current_time_ms - current->last_update_time_ms;
        current->ellapsed_time_ms += delta;
        current->last_update_time_ms = current_time_ms;

        // Terminou o burst?
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
        // Expirou o quantum?
        else if (current_time_ms - current->slice_start_ms >= QUANTUM_MS) {
            if (rq && rq->head) {
                // Há alguém à espera: preempta e reenfileira
                (void)enqueue_pcb(rq, current);
                *cpu_task = NULL;
            } else {
                // Sozinha: só renova o slice
                current->slice_start_ms = current_time_ms;
            }
        }
    }

    // CPU livre? despachar próxima
    if (*cpu_task == NULL) {
        pcb_t *next = dequeue_pcb(rq);
        if (next) {
            next->slice_start_ms = current_time_ms;
            next->last_update_time_ms = current_time_ms; // importante para o delta
            *cpu_task = next;
        }
    }
}
