/*
 * command_queue.h
 *
 *  Created on: Mar 2, 2020
 *      Author: gufu
 */

#ifndef INC_COMMAND_QUEUE_H_
#define INC_COMMAND_QUEUE_H_

#include "inc/state_machine.h"
#include <stdbool.h>

#define COMMAND_QUEUE_SIZE    10

typedef struct {
    Uint8  head;
    Uint8  tail;
    Uint32 size;
    commands_e command_queue[COMMAND_QUEUE_SIZE];
} command_queue_t;

void queue_push(command_queue_t * p_queue, commands_e command);

bool queue_pop(command_queue_t * p_queue, commands_e * command);

bool queue_isempty(command_queue_t * p_queue);

bool queue_isfull(command_queue_t * p_queue);

#endif /* INC_COMMAND_QUEUE_H_ */
