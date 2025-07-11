#define _QUEUE_C_

#include "par_queue.h"
#include <string.h>


/**
 * @brief queue init
 *
 * @param queue queue object
 */
void par_init_queue(queue_t* queue)
{
//  __disable_irq();
  queue->queue_head = queue->queue_tail = 0;
//  __enable_irq();
}

/**
 * @brief query queue full
 *
 * @param queue  queue obj
 * @return true full
 * @return false non-full
 */
uint8_t par_query_queue_full(queue_t* queue)
{
  return (((queue->queue_head + 1) % PAR_QUEUE_SIZE) == queue->queue_tail);
}

/**
 * @brief query queue empty
 *
 * @param queue queue obj
 * @return true empty
 * @return false non-empty
 */
uint8_t par_query_queue_empty(queue_t* queue)
{
  return (queue->queue_head == queue->queue_tail);
}

/**
 * @brief push queue
 *
 * @param queue queue obj
 * @param p_data en data buffer
 * @param data_size data size
 */
void par_en_queue(queue_t* queue, uint8_t* p_data, uint8_t data_size)
{
  if (par_query_queue_full(queue))
  {
    return;
  }

//  __disable_irq();
  uint8_t head = queue->queue_head;
  queue->queue_buf[head].lenght = data_size;
  memcpy(queue->queue_buf[head].dat, p_data, data_size);
  queue->queue_head = (head + 1) % PAR_QUEUE_SIZE;
//  __enable_irq();
}

/**
 * @brief query queue head
 *
 * @param queue queue obj
 * @param p_data data buffer
 * @return queue head
 */
uint8_t par_query_queue_head(queue_t* queue, uint8_t* p_data)
{
  if (par_query_queue_empty(queue))
  {
    return 0;
  }

//  __disable_irq();
  uint8_t tail = queue->queue_tail;
  uint8_t tmp = queue->queue_buf[tail].lenght;
  memcpy(p_data, queue->queue_buf[tail].dat, queue->queue_buf[tail].lenght);
  queue->queue_tail = (tail + 1) % PAR_QUEUE_SIZE;
//  __enable_irq();

  return tmp;
}

/**
 * @brief queue pop
 *
 * @param queue queue obj
 */
void par_de_queue(queue_t* queue)
{
  if (par_query_queue_empty(queue))
  {
    return ;
  }

//  __disable_irq();
  queue->queue_tail = (queue->queue_tail + 1) % PAR_QUEUE_SIZE;
//  __enable_irq();
}



