#ifndef SJF_H
#define SJF_H

#include <stdint.h>
#include "queue.h"

/**
 * SJF (Shortest Job First) não-preemptivo.
 * - Se o CPU está ocupado, deixa a tarefa atual continuar (e o loop/ticker chamará de novo).
 * - Se o CPU está livre, escolhe na ready queue o PCB com menor tempo restante (pcb->time_ms).
 */
void sjf_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task);

#endif // SJF_H
