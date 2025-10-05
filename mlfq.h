#ifndef MLFQ_H
#define MLFQ_H

#include "queue.h"
#include <stdint.h>

#define MLFQ_LEVELS 3
#define TIME_SLICE_MS 500

// Estrutura principal do MLFQ
typedef struct {
    queue_t queues[MLFQ_LEVELS];      // Array de filas (0 = maior prioridade)
    uint32_t time_slices[MLFQ_LEVELS]; // Quantum para cada nível
} mlfq_t;

/**
 * @brief Inicializa a estrutura MLFQ
 *
 * @param mlfq Ponteiro para a estrutura MLFQ a ser inicializada
 */
void mlfq_init(mlfq_t *mlfq);

/**
 * @brief Adiciona uma tarefa a uma fila específica do MLFQ
 *
 * @param mlfq Ponteiro para a estrutura MLFQ
 * @param task Tarefa a ser enfileirada
 * @param level Nível da fila (0 = maior prioridade)
 */
void mlfq_enqueue(mlfq_t *mlfq, pcb_t *task, int level);

/**
 * @brief Remove e retorna a próxima tarefa do MLFQ (da fila de maior prioridade não vazia)
 *
 * @param mlfq Ponteiro para a estrutura MLFQ
 * @return pcb_t* Próxima tarefa a ser executada, ou NULL se todas as filas estiverem vazias
 */
pcb_t* mlfq_dequeue(mlfq_t *mlfq);

/**
 * @brief Algoritmo de escalonamento MLFQ (Multi-Level Feedback Queue)
 *
 * @param current_time_ms Tempo atual em milissegundos
 * @param mlfq Ponteiro para a estrutura MLFQ
 * @param cpu_task Ponteiro para a tarefa atualmente em execução na CPU
 */
void mlfq_scheduler(uint32_t current_time_ms, mlfq_t *mlfq, pcb_t **cpu_task);

/**
 * @brief Promove uma tarefa para uma fila de maior prioridade
 *
 * @param mlfq Ponteiro para a estrutura MLFQ
 * @param task Tarefa a ser promovida
 */
void mlfq_promote(mlfq_t *mlfq, pcb_t *task);

#endif // MLFQ_H