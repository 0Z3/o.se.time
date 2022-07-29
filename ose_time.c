/*
  Copyright (c) 2019-22 John MacCallum Permission is hereby granted,
  free of charge, to any person obtaining a copy of this software
  and associated documentation files (the "Software"), to deal in
  the Software without restriction, including without limitation the
  rights to use, copy, modify, merge, publish, distribute,
  sublicense, and/or sell copies of the Software, and to permit
  persons to whom the Software is furnished to do so, subject to the
  following conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.
*/

/* memset */
#include <string.h>

#include "ose.h"
#include "ose_stackops.h"
#include "ose_assert.h"
#include "ose_vm.h"

#if defined(_WIN32) || defined(_WIN64)

//#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <time.h>

#ifndef __GNUC__
#define EPOCHFILETIME (116444736000000000i64)
#else
#define EPOCHFILETIME (116444736000000000LL)
#endif

/* struct timezone { */
/*     int tz_minuteswest; /\* minutes W of Greenwich *\/ */
/*     int tz_dsttime;     /\* type of dst correction *\/ */
/* }; */

__inline int gettimeofday_win(struct timeval *tv, struct timezone *tz)
{
    FILETIME        ft;
    LARGE_INTEGER   li;
    __int64         t;
    static int      tzflag;

    if (tv)
    {
        GetSystemTimePreciseAsFileTime(&ft);
        li.LowPart  = ft.dwLowDateTime;
        li.HighPart = ft.dwHighDateTime;
        t  = li.QuadPart;       /* In 100-nanosecond intervals */
        t -= EPOCHFILETIME;     /* Offset to the Epoch time */
        t /= 10;                /* In microseconds */
        tv->tv_sec  = (long)(t / 1000000);
        tv->tv_usec = (long)(t % 1000000);
    }

    if (tz)
    {
        if (!tzflag)
        {
            tzset();
            tzflag++;
        }
        tz->tz_minuteswest = timezone / 60;
        tz->tz_dsttime = daylight;
    }

    return 0;
}
#define ose_time_gettimeofday gettimeofday_win

#else
/* not _WIN32 or _WIN64 */
#define __USE_BSD
#include <sys/time.h>
#include <time.h>
#define ose_time_gettimeofday gettimeofday

#endif

static void ose_time_now(ose_bundle osevm)
{
    ose_bundle vm_s = OSEVM_STACK(osevm);
    struct timeval tv;
	struct timezone tz;
	struct ose_timetag n;

	ose_time_gettimeofday(&tv, &tz);
    
	n.sec = (uint32_t)2208988800UL + (uint32_t)tv.tv_sec;
	n.fsec = (uint32_t)(tv.tv_usec * 4295);
    ose_pushTimetag(vm_s, n.sec, n.fsec);
}

void ose_main(ose_bundle osevm)
{
    ose_bundle vm_s = OSEVM_STACK(osevm);
    ose_pushBundle(vm_s);
    ose_pushMessage(vm_s, "/time/now", strlen("/time/now"), 1,
                    OSETT_ALIGNEDPTR, ose_time_now);
    ose_push(vm_s);
}
