/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This is an example on how to use the gdb support found on jennic modules used
 * through a serial connection. Contiki has printf support through the gdb
 * serial connection, compiled-in breakpoint and support for processor
 * exceptions.
 *
 * To use compile this program with:
 *  make DEBUG=true
 * and make sure that you cleaned the build beforehand by doing:
 *  make clean
 *
 * After programming start the jennic module and start the debugger on your host
 * machine with:
 *  ba-elf-gdb -x cpu/jennic/gdbUSB0.gdb gdb-support
 */

#include "contiki.h"
#include <stdint.h>
#include <gdb2.h>

PROCESS(gdb_example, "GDB example program");
AUTOSTART_PROCESSES(&gdb_example);

PROCESS_THREAD(gdb_example, ev, data)
{
  static char arr[8];
  static uint32_t *p;

  PROCESS_BEGIN();

  /* print something on the gdb console */
  printf("Hello World!\n");

  /* stop here, when reaching this point */
  HAL_BREAKPOINT();

  /* jennic modules do not allow unaligned memory access, on the JN5139 an
   * exception handler is available which emulates this access. This is a big
   * performance hog, since a single instruction will be replaced by an
   * interrupt handler which result in about 60 instructions being executed. */
  p  = (uint32_t*) &arr[1];

  printf("Trying access an int at 0x%x, offset to nearest alignment is %ld\n",
         (int) p, ((int) p)%sizeof(uint32_t));

  /* On the JN5148 (and on the JN5139 if the handler is not activated) the debugger
   * will break at this point. */
  *p = 0xdead;

  PROCESS_END();
}
