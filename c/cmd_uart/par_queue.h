#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <stdint.h>


#define PAR_QUEUE_MAX_DATA_SIZE 21
#define PAR_QUEUE_SIZE 40

typedef struct
{
  uint8_t lenght;
  uint8_t dat[PAR_QUEUE_MAX_DATA_SIZE];
} queue_dat_t;

/* Typedef */
typedef struct
{
  uint8_t queue_head;
  uint8_t queue_tail;
  queue_dat_t queue_buf[PAR_QUEUE_SIZE];
} queue_t;

/**
 * @brief queue init
 *
 * @param queue queue object
 */
void par_init_queue(queue_t* queue);

/**
 * @brief query queue full
 *
 * @param queue  queue obj
 * @return true full
 * @return false non-full
 */
uint8_t par_query_queue_full(queue_t* queue);

/**
 * @brief query queue empty
 *
 * @param queue queue obj
 * @return true empty
 * @return false non-empty
 */
uint8_t par_query_queue_empty(queue_t* queue);

/**
 * @brief push queue
 *
 * @param queue queue obj
 * @param p_data en data buffer
 * @param data_size data size
 */
void par_en_queue(queue_t* queue, uint8_t* p_data, uint8_t data_size);

/**
 * @brief query queue head
 *
 * @param queue queue obj
 * @param p_data data buffer
 * @return queue head
 */
uint8_t par_query_queue_head(queue_t* queue, uint8_t* p_data);

/**
 * @brief queue pop
 *
 * @param queue queue obj
 */
void par_de_queue(queue_t* queue);

#endif
