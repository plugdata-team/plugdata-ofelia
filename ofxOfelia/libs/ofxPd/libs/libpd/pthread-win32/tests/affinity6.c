/*
 * affinity6.c
 *
 *
 * --------------------------------------------------------------------------
 *
 *      pthreads-win32 - POSIX Threads Library for Win32
 *      Copyright(C) 1998 John E. Bossom
 *      Copyright(C) 1999-2021 pthreads-win32 / pthreads4w contributors
 *
 *      Homepage1: http://sourceware.org/pthreads-win32/
 *      Homepage2: http://sourceforge.net/projects/pthreads4w/
 *
 *      The current list of contributors is contained
 *      in the file CONTRIBUTORS included with the source
 *      code distribution. The list can also be seen at the
 *      following World Wide Web location:
 *      http://sources.redhat.com/pthreads-win32/contributors.html
 * 
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License as published by the Free Software Foundation; either
 *      version 2 of the License, or (at your option) any later version.
 * 
 *      This library is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 * 
 *      You should have received a copy of the GNU Lesser General Public
 *      License along with this library in the file COPYING.LIB;
 *      if not, write to the Free Software Foundation, Inc.,
 *      59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *
 * --------------------------------------------------------------------------
 *
 * Test thread CPU affinity from thread attributes.
 *
 */

#if ! defined(WINCE)

#include "test.h"

typedef union
{
        /* Violates opacity */
        cpu_set_t cpuset;
        unsigned long int bits;  /* To stop GCC complaining about %lx args to printf */
} cpuset_to_ulint;

void *
mythread(void * arg)
{
  pthread_attr_t *attrPtr = (pthread_attr_t *) arg;
  cpu_set_t threadCpus, attrCpus;

  assert(pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t), &threadCpus) == 0);
  assert(pthread_attr_getaffinity_np(attrPtr, sizeof(cpu_set_t), &attrCpus) == 0);
  assert(CPU_EQUAL(&attrCpus, &threadCpus));

  return (void*) 0;
}

#ifndef MONOLITHIC_PTHREAD_TESTS
int
main()
#else 
int
test_affinity6(void)
#endif
{
  unsigned int cpu;
  pthread_t tid;
  pthread_attr_t attr1, attr2;
  cpu_set_t threadCpus;
  cpu_set_t keepCpus;
  pthread_t self = pthread_self();

  if (pthread_getaffinity_np(self, sizeof(cpu_set_t), &threadCpus) == ENOSYS)
    {
      printf("pthread_get/set_affinity_np API not supported for this platform: skipping test.");
      return 0;
    }

  assert(pthread_attr_init(&attr1) == 0);
  assert(pthread_attr_init(&attr2) == 0);

  CPU_ZERO(&keepCpus);
  for (cpu = 1; cpu < sizeof(cpu_set_t)*8; cpu += 2)
    {
          CPU_SET(cpu, &keepCpus);                                      /* 0b10101010101010101010101010101010 */
    }

  assert(pthread_getaffinity_np(self, sizeof(cpu_set_t), &threadCpus) == 0);

  if (CPU_COUNT(&threadCpus) > 1)
    {
          assert(pthread_attr_setaffinity_np(&attr1, sizeof(cpu_set_t), &threadCpus) == 0);
          CPU_AND(&threadCpus, &threadCpus, &keepCpus);
          assert(pthread_attr_setaffinity_np(&attr2, sizeof(cpu_set_t), &threadCpus) == 0);

          assert(pthread_create(&tid, &attr1, mythread, (void *) &attr1) == 0);
          assert(pthread_join(tid, NULL) == 0);
          assert(pthread_create(&tid, &attr2, mythread, (void *) &attr2) == 0);
          assert(pthread_join(tid, NULL) == 0);
    }
  assert(pthread_attr_destroy(&attr1) == 0);
  assert(pthread_attr_destroy(&attr2) == 0);
  return 0;
}

#else

#include <stdio.h>

int
main()
{
  fprintf(stderr, "Test N/A for this target environment.\n");
  return 0;
}

#endif
