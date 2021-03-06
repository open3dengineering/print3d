/*
 * This file is part of the Doodle3D project (http://doodle3d.com).
 *
 * Copyright (c) 2013, Doodle3D
 * This software is licensed under the terms of the GNU GPL v2 or later.
 * See file LICENSE.txt or visit http://www.gnu.org/licenses/gpl.html for full license details.
 */

#ifndef UTILS_H_SEEN
#define UTILS_H_SEEN

#include <sys/time.h>

#ifdef __cplusplus
	extern "C" {
#endif

#include <inttypes.h>

//returns an ASCIIZ string (newly allocated if buf is NULL), or NULL if an error occured
char *number_to_string(int n, char *buf);

int number_length(int n);
uint16_t read_ns(const void *p);
uint32_t read_nl(const void *p);
void store_ns(void *p, uint16_t v);
void store_nl(void *p, uint32_t v);
uint32_t getMillis();
int timeval_subtract (struct timeval *result, struct timeval *x, struct timeval *y);
int readAndAppendAvailableData(int fd, char **buf, int *buflen, int timeout, int onlyOnce);
char *readFileContents(const char *file, int *size);
int equal(const char *s1, const char *s2);
int isAbsolutePath(const char *path);

#ifdef __cplusplus
	} //extern "C"
#endif

#endif /* ! UTILS_H_SEEN */
