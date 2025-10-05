#include "sjf.h"
#include "queue.h"
#include "msg.h"     // TICKS_MS, msg_t, PROCESS_REQUEST_DONE
#include <unistd.h>  // write
#include <stdlib.h>  // free
#include <stdio.h>   // perror

void sjf_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task) {
    // Avança a tarefa atual (mesma lógica do FIFO)
    if (*cpu_task) {
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;

        if ((*cpu_task)->ellapsed_time_ms >= (*cpu_task)->time_ms) {
            msg_t msg = {
                .pid     = (*cpu_task)->pid,
                .request = PROCESS_REQUEST_DONE,
                .time_ms = current_time_ms
            };
            if (write((*cpu_task)->sockfd, &msg, sizeof(msg)) != sizeof(msg)) {
                perror("write");
            }
            free(*cpu_task);
            *cpu_task = NULL;
        }
        if (*cpu_task) return; // não-preemptivo: continua a mesma até acabar
    }

    // CPU livre -> escolhe o mais curto da ready queue
    *cpu_task = dequeue_shortest_ready(rq);
}
