/*
 * Copyright (C) 2016 Nano Avionics
 *
 * Licensed under the GNU GENERAL PUBLIC LICENSE, Version 3 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License from the Free Software Foundation, Inc.
 * at
 *
 *    http://fsf.org/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "dprint.h"
#include <stdio.h>

char buff[512];

void dprint(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    int len = vsprintf(buff, fmt, args);
    buff[len] = '\0';

    uart_puts(buff);

    va_end(args);
}


