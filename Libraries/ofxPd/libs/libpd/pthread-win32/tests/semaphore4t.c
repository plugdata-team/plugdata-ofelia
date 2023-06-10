/*
 * File: semaphore4t.c
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
 * Test Synopsis: Verify sem_getvalue returns the correct number of waiters
 * after threads are cancelled.
 * -
 *
 * Test Method (Validation or Falsification):
 * - Validation
 *
 * Requirements Tested:
 * - sem_timedwait cancellation.
 *
 * Features Tested:
 * -
 *
 * Cases Tested:
 * -
 *
 * Description:
 * -
 *
 * Environment:
 * -
 *
 * Input:
 * - None.
 *
 * Output:
 * - File name, Line number, and failed expression on failure.
 * - No output on success.
 *
 * Assumptions:
 * -
 *
 * Pass Criteria:
 * - Process returns zero exit status.
 *
 * Fail Criteria:
 * - Process returns non-zero exit status.
 */

#include "test.h"

#define MAX_COUNT 100

static const long NANOSEC_PER_SEC = 1000000000L;

static sem_t s;

static void *
thr (void * arg)
{
  assert(sem_timedwait(&s, NULL) == 0);
  return NULL;
}

static int
timeoutwithnanos(sem_t sem, int nanoseconds)
{
  struct timespec ts, rel;
  FILETIME ft_before, ft_after;
  int rc;

  rel.tv_sec = 0;
  rel.tv_nsec = nanoseconds;

  GetSystemTimeAsFileTime(&ft_before);
  rc = sem_timedwait(&sem, pthread_win32_getabstime_np(&ts, &rel));

  /* This should have timed out */
  assert(rc != 0);
  assert(errno == ETIMEDOUT);
  GetSystemTimeAsFileTime(&ft_after);
  // We specified a non-zero wait. Time must advance.
  if (ft_before.dwLowDateTime == ft_after.dwLowDateTime && ft_before.dwHighDateTime == ft_after.dwHighDateTime)
    {
      printf("nanoseconds: %d, rc: %d, errno: %d. before filetime: %d, %d; after filetime: %d, %d\n",
          nanoseconds, rc, errno,
          (int)ft_before.dwLowDateTime, (int)ft_before.dwHighDateTime,
          (int)ft_after.dwLowDateTime, (int)ft_after.dwHighDateTime);
      printf("time must advance during sem_timedwait.");
      return 1;
    }
  return 0;
}

static int
testtimeout()
{
  int rc = 0;
  sem_t s2;
  int value = 0;
  assert(sem_init(&s2, PTHREAD_PROCESS_PRIVATE, 0) == 0);
  assert(sem_getvalue(&s2, &value) == 0);
  assert(value == 0);

  rc += timeoutwithnanos(s2, 1000);        // 1 microsecond
  rc += timeoutwithnanos(s2, 10 * 1000);   // 10 microseconds
  rc += timeoutwithnanos(s2, 100 * 1000);  // 100 microseconds
  rc += timeoutwithnanos(s2, 1000 * 1000); // 1 millisecond

  return rc;
}

static int
testmainstuff()
{
  int value = 0;
  int i;
  pthread_t t[MAX_COUNT+1];

  assert(sem_init(&s, PTHREAD_PROCESS_PRIVATE, 0) == 0);
  assert(sem_getvalue(&s, &value) == 0);
  assert(value == 0);

  for (i = 1; i <= MAX_COUNT; i++)
    {
      assert(pthread_create(&t[i], NULL, thr, NULL) == 0);
      do {
          sched_yield();
          assert(sem_getvalue(&s, &value) == 0);
      } while (value != -i);
      assert(-value == i);
    }

  assert(sem_getvalue(&s, &value) == 0);
  assert(-value == MAX_COUNT);
  assert(pthread_cancel(t[50]) == 0);
  assert(pthread_join(t[50], NULL) == 0);
  assert(sem_getvalue(&s, &value) == 0);
  assert(-value == MAX_COUNT - 1);

  for (i = MAX_COUNT - 2; i >= 0; i--)
    {
      assert(sem_post(&s) == 0);
      assert(sem_getvalue(&s, &value) == 0);
      assert(-value == i);
    }

  for (i = 1; i <= MAX_COUNT; i++)
    {
      if (i != 50)
        {
          assert(pthread_join(t[i], NULL) == 0);
        }
    }

  return 0;
}

#ifndef MONOLITHIC_PTHREAD_TESTS
int
main()
#else
int
test_semaphore4t(void)
#endif
{
  int rc = 0;

  rc += testmainstuff();
  rc += testtimeout();

  return rc;
}

