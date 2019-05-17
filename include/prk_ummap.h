/*
Copyright (c) 2013, Intel Corporation

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions 
are met:

* Redistributions of source code must retain the above copyright 
      notice, this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above 
      copyright notice, this list of conditions and the following 
      disclaimer in the documentation and/or other materials provided 
      with the distribution.
      * Neither the name of Intel Corporation nor the names of its 
      contributors may be used to endorse or promote products 
      derived from this software without specific prior written 
      permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PRK_UMMAP_H
#define PRK_UMMAP_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <mpi.h>
#include "ummap.h"
#include "util.h"
#include <par-res-kern_general.h>

#define ESUCCESS    0
#define TRUE        1
#define FALSE       0
#define PROT_FULL   (PROT_READ  | PROT_WRITE)
#define MMAP_FLAGS  (MAP_SHARED | MAP_NORESERVE)
#define POSIX_FLAGS (O_CREAT    | O_RDWR)

typedef enum
{
  PRK_ALLOC_MEM = 0,
  PRK_ALLOC_MMAP,
  PRK_ALLOC_UMMAP
} prk_alloc_type_t;

typedef struct
{
  prk_alloc_type_t type;
  size_t           seg_size;
  char             path[PATH_MAX];
} prk_alloc_settings_t;

static prk_alloc_settings_t settings = { .seg_size = 16777216,
                                         .path     = "./tmp" };

static int get_env(const char *name, const char *format, void *target)
{
    const char *buffer = getenv(name);
    return ((buffer != NULL && sscanf(buffer, format, target) != 1) * EINVAL);
}

static inline prk_alloc_type_t get_alloc_type()
{
  const char *buffer = getenv("PRK_ALLOCTYPE");
  
  return (buffer == NULL || !strcmp(buffer, "mem"))  ? PRK_ALLOC_MEM  :
                           (!strcmp(buffer, "mmap")) ? PRK_ALLOC_MMAP :
                                                       PRK_ALLOC_UMMAP;
}

static void* prk_malloc_v2(size_t size)
{
  void *baseptr = NULL;
  
  settings.type = get_alloc_type();
  
  if (settings.type == PRK_ALLOC_MEM)
  {
    baseptr = prk_malloc(size);
  }
  else
  {
    static uint32_t alloc_id              = 0;
    int             rank                  = 0;
    int             fd                    = -1;
    char            filename[PATH_MAX<<1] = { 0 };
    
    get_env("PRK_SEGSIZE", "%zu", &settings.seg_size);
    get_env("PRK_PATH",    "%s",  &settings.path);
    
    if (settings.type == PRK_ALLOC_MMAP)
    {
      settings.seg_size = sysconf(_SC_PAGESIZE);
    }
    
    size = (size + (settings.seg_size - 1)) & ~(settings.seg_size - 1);
    
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    sprintf(filename, "%s/n%d/p%d", settings.path, (int)(rank/32), rank);
    if (createDir(filename) != ESUCCESS) return NULL;
    sprintf(filename, "%s/n%d/p%d/prk_alloc_%d.tmp",
                               settings.path, (int)(rank/32), rank, alloc_id++);
    
    if (openFile(filename, POSIX_FLAGS, TRUE, size, &fd) != ESUCCESS)
      return NULL;

    if (settings.type == PRK_ALLOC_MMAP)
    {
      baseptr = mmap(NULL, size, PROT_FULL, MMAP_FLAGS, fd, 0);
      if (baseptr == MAP_FAILED) return NULL;
    }
    else
    {
      if (ummap(size, settings.seg_size, PROT_FULL, fd, 0, UINT32_MAX, FALSE, 0,
                (void **)&baseptr) != ESUCCESS) return NULL;
    }
    
    close(fd);
  }
  
  return baseptr;
}

static int prk_sync(void *baseptr, size_t size)
{
  size = (size + (settings.seg_size - 1)) & ~(settings.seg_size - 1);
  
  return (settings.type == PRK_ALLOC_MMAP)  ? msync(baseptr, size, MS_SYNC) :
         (settings.type == PRK_ALLOC_UMMAP) ? umsync(baseptr, FALSE) :
                                              ESUCCESS;
}

static void prk_free_v2(void *baseptr, size_t size)
{
  if (settings.type == PRK_ALLOC_MEM)
  {
    prk_free(baseptr);
  }
  else
  {
    int  rank               = 0;
    char filename[PATH_MAX] = { 0 };
    
    if (settings.type == PRK_ALLOC_MMAP)
    {
      size = (size + (settings.seg_size - 1)) & ~(settings.seg_size - 1);
      munmap(baseptr, size);
    }
    else
    {
      umunmap(baseptr, FALSE);
    }
    
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    sprintf(filename, "%s/n%d/p%d", settings.path, (int)(rank/32), rank);
    deleteDir(filename);
  }
}
#endif /* PRK_UMMAP_H */

