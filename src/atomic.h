/***************************************************************************
 * Some atomic related functions and bariers definitions
 **************************************************************************/
#ifndef ATOMIC_H
#define ATOMIC_H

/* Compiler memory barrier instructs the compiler to not cache any
   memory data in registers beyond the barrier.
   NOTE: avr/cpufunc.h contains compile error, so use this declaration */
#define MEM_BARRIER asm volatile ("" ::: "memory")

#endif //ATOMIC_H
