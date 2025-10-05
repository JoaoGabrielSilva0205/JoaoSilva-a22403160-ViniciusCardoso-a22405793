#ifndef rr_H
#define rr_H

#include "queue.h"
#include <stdint.h>

void rr_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task);

#endif