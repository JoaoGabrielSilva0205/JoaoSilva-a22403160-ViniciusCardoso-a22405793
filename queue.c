#include "queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>   // UINT32_MAX

pcb_t *new_pcb(int32_t pid, uint32_t sockfd, uint32_t time_ms) {
    pcb_t * new_task = malloc(sizeof(pcb_t));
    if (!new_task) return NULL;

    new_task->pid = pid;
    new_task->status = TASK_COMMAND;
    new_task->slice_start_ms = 0;
    new_task->sockfd = sockfd;
    new_task->time_ms = time_ms;
    new_task->ellapsed_time_ms = 0;
    new_task->last_update_time_ms = 0;
    return new_task;
}

int enqueue_pcb(queue_t* q, pcb_t* task) {
    queue_elem_t* elem = malloc(sizeof(queue_elem_t));
    if (!elem) return 0;

    elem->pcb = task;
    elem->next = NULL;

    if (q->tail) {
        q->tail->next = elem;
    } else {
        q->head = elem;
    }
    q->tail = elem;
    return 1;
}


pcb_t* dequeue_pcb(queue_t* q) {
    if (!q || !q->head) return NULL;

    queue_elem_t* node = q->head;
    pcb_t* task = node->pcb;

    q->head = node->next;
    if (!q->head)
        q->tail = NULL;

    free(node);
    return task;
}

/* === NOVO: devolve o PCB com menor tempo restante (SJF) === */
pcb_t* dequeue_shortest_ready(queue_t* q) {
    if (!q || !q->head) return NULL;

    queue_elem_t *best = NULL;
    queue_elem_t *best_prev = NULL;
    queue_elem_t *prev = NULL;
    queue_elem_t *it = q->head;

    uint32_t best_remaining = UINT32_MAX;

    while (it) {
        pcb_t *p = it->pcb;
        if (p && p->status == TASK_RUNNING) {
            uint32_t used = p->ellapsed_time_ms;
            uint32_t remaining = (p->time_ms > used) ? (p->time_ms - used) : 0;
            if (remaining < best_remaining) {
                best_remaining = remaining;
                best = it;
                best_prev = prev;
                if (remaining == 0) break; // não há menor do que 0
            }
        }
        prev = it;
        it = it->next;
    }

    if (!best) return NULL;

    // Desencaixar 'best' da fila
    if (best_prev) best_prev->next = best->next;
    else q->head = best->next;

    if (q->tail == best) q->tail = best_prev;

    pcb_t *res = best->pcb;
    free(best);  // libertar o nó da fila (consistente com dequeue_pcb)
    return res;
}


queue_elem_t *remove_queue_elem(queue_t* q, queue_elem_t* elem) {
    queue_elem_t* it = q->head;
    queue_elem_t* prev = NULL;
    while (it != NULL) {
        if (it == elem) {
            // Remove elem from queue
            if (prev) {
                prev->next = it->next;
            } else {
                q->head = it->next;
            }
            if (it == q->tail) {
                q->tail = prev;
            }
            return it;
        }
        prev = it;
        it = it->next;
    }
    printf("Queue element not found in queue\n");
    return NULL;
}