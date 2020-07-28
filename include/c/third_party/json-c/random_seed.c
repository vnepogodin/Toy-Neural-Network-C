/*
 * random_seed.c
 *
 * Copyright (c) 2013 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */
#include "config.h"

#include "random_seed.h"

#include <errno.h>
#include <stdio.h>

#define DEBUG_SEED(s)

/* has_dev_urandom */
#if defined(__APPLE__) || defined(__unix__) || defined(__linux__)

#include <fcntl.h>
#include <string.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <stdlib.h>
#include <sys/stat.h>

#define HAVE_DEV_RANDOM 1

static int has_dev_urandom(void) {
	struct stat buf;
	if (stat("/dev/urandom", &buf))
		return 0;

	return ((buf.st_mode & S_IFCHR) != 0);
}

/* get_dev_random_seed */
static int get_dev_random_seed(void) {

	int fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "error opening /dev/urandom: %s", strerror(errno));
		exit(1);
	}

	int r;
	register long nread = read(fd, &r, sizeof(r));
	if (nread != sizeof(r))
	{
		fprintf(stderr, "error short read /dev/urandom: %s", strerror(errno));
		exit(1);
	}

	close(fd);
	return r;
}

#endif

/* get_cryptgenrandom_seed */

#ifdef WIN32

#define HAVE_CRYPTGENRANDOM 1

/* clang-format off */
#include <windows.h>

/* Caution: these blank lines must remain so clang-format doesn't reorder
   includes to put windows.h after wincrypt.h */

#include <wincrypt.h>
/* clang-format on */
#ifndef __GNUC__
#pragma comment(lib, "advapi32.lib")
#endif

static int get_time_seed(void);

static int get_cryptgenrandom_seed(void)
{
	HCRYPTPROV hProvider = 0;
	DWORD dwFlags = CRYPT_VERIFYCONTEXT;
	int r;

	DEBUG_SEED("get_cryptgenrandom_seed");

	/* WinNT 4 and Win98 do no support CRYPT_SILENT */
	if (LOBYTE(LOWORD(GetVersion())) > 4)
		dwFlags |= CRYPT_SILENT;

	if (!CryptAcquireContextA(&hProvider, 0, 0, PROV_RSA_FULL, dwFlags))
	{
		fprintf(stderr, "error CryptAcquireContextA 0x%08lx", GetLastError());
		r = get_time_seed();
	}
	else
	{
		BOOL ret = CryptGenRandom(hProvider, sizeof(r), (BYTE*)&r);
		CryptReleaseContext(hProvider, 0);
		if (!ret)
		{
			fprintf(stderr, "error CryptGenRandom 0x%08lx", GetLastError());
			r = get_time_seed();
		}
	}

	return r;
}

#endif

/* get_time_seed */
#include <time.h>

static int get_time_seed(void) {
	return (int)time(NULL) * 433494437;
}

/* json_c_get_random_seed */

int json_c_get_random_seed(void) {
#ifdef OVERRIDE_GET_RANDOM_SEED
	OVERRIDE_GET_RANDOM_SEED;
#endif
#if defined HAVE_RDRAND && HAVE_RDRAND
	if (has_rdrand())
		return get_rdrand_seed();
#endif
#if defined HAVE_DEV_RANDOM && HAVE_DEV_RANDOM
	if (has_dev_urandom())
		return get_dev_random_seed();
#endif
#if defined HAVE_CRYPTGENRANDOM && HAVE_CRYPTGENRANDOM
	return get_cryptgenrandom_seed();
#endif
	return get_time_seed();
}
