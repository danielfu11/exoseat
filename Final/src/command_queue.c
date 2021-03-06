/*
 * command_queue.c
 *
 *  Created on: Mar 2, 2020
 *      Author: gufu
 */

#include "DSP28x_Project.h"
#include "inc/command_queue.h"

void queue_push(command_queue_t * p_queue, commands_e command)
{
    if (!queue_isfull(p_queue))
    {
        p_queue->command_queue[p_queue->tail++] = command;
        if (p_queue->tail == COMMAND_QUEUE_SIZE)
        {
            p_queue->tail = 0;
        }
        p_queue->size++;
    }
    else
    {
        // Should we overwrite when the
    }
}

bool queue_pop(command_queue_t * p_queue, commands_e * command)
{
    if (!queue_isempty(p_queue))
    {
        *command = p_queue->command_queue[p_queue->head++];
        if (p_queue->head == COMMAND_QUEUE_SIZE)
        {
            p_queue->head = 0;
        }
        p_queue->size--;
        return true;
    }
    else
    {
        return false;
    }
}

bool queue_isempty(command_queue_t * p_queue)
{
    if (p_queue->head == p_queue->tail && p_queue->size == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool queue_isfull(command_queue_t * p_queue)
{
    if (p_queue->head == p_queue->tail && p_queue->size == COMMAND_QUEUE_SIZE)
    {
        return true;
    }
    else
    {
        return false;
    }
}
