/* cma_api.c - API for contigous memory allocation driver.
 *
 *
 * The MIT License (MIT)
 *
 * COPYRIGHT (C) 2017 Institute of Electronics and Computer Science (EDI), Latvia.
 * AUTHOR: Rihards Novickis (rihards.novickis@edi.lv)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 * DESCRIPTION:
 * For API interface documentation refer to "include/cma_api.h" header file.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "cma.h"


#ifndef CMA_DEBUG
	#define CMA_DEBUG 			0
#endif
#ifndef DRIVER_NODE_NAME
	#define DRIVER_NODE_NAME 	"cma"
#endif



#if CMA_DEBUG == 1
	#define __DEBUG(fmt, args...)	printf("CMA_API_DEBUG: " fmt, ##args)
#else
	#define __DEBUG(fmt,args...)
#endif 

#define ROUND_UP(N, S) 		((((N) + (S) - 1) / (S)) * (S))


/* Private functions */
void *cma_alloc(size_t size, unsigned ioctl_cmd);

/* Global file descriptor */
int cma_fd = 0;



int cma_init(void)
{
	__DEBUG("Opening \"/dev/" DRIVER_NODE_NAME "\" file\n");
	
	cma_fd = open("/dev/"DRIVER_NODE_NAME, O_RDWR);
	if(cma_fd == -1){
		__DEBUG("Failed to initialize api - \"%s\"\n", strerror(errno));
		return -1;
	}

	return 0;
}

int cma_release(void)
{
	__DEBUG("Closing \"/dev/" DRIVER_NODE_NAME "\" file\n");

	if(close(cma_fd) == -1){
		__DEBUG("Failed to finilize api - \"%s\"\n", strerror(errno));
		return -1;
	}
	
	return 0;
}



void *cma_alloc_cached(size_t size)
{
	return cma_alloc(size, CMA_ALLOC_CACHED);
}

void *cma_alloc_noncached(size_t size)
{
	return cma_alloc(size, CMA_ALLOC_NONCACHED);
}


int cma_free(void *mem)
{
	unsigned data, v_addr;

	/* save user space pointer value */
	data   = (unsigned)mem;
	v_addr = (unsigned)mem;

	if( ioctl(cma_fd, CMA_GET_SIZE, &data) == -1){
		__DEBUG("cma_free - ioctl command unsuccsessful - \"%s\"\n", strerror(errno));
		return -1;
	}
	/* data now contains size */

	/* unmap memory */
	munmap(mem, data);

	/* free cma entry */
	if( ioctl(cma_fd, CMA_FREE, &v_addr) == -1){
		__DEBUG("cma_free - ioctl command unsuccsessful - \"%s\"\n", strerror(errno));
		return -1;
	}

	return 0;
}


unsigned cma_get_phy_addr(void *mem)
{
	unsigned data;

	/* save user space pointer value */
	data = (unsigned)mem;

	/* get physical address */
	if( ioctl(cma_fd, CMA_GET_PHY_ADDR, &data) == -1){
		__DEBUG("cma_get_phy_addr - ioctl command unsuccsessful - \"%s\"\n", strerror(errno));
		return 0;
	}
	/* data now contains physical address */

	return data;
}


void *cma_alloc(size_t size, unsigned ioctl_cmd)
{
	unsigned data;
	void 	*mem;
	__DEBUG("Allocating 0x%x bytes of contigous memory\n", size);

	/* Page align size */
	size = ROUND_UP(size, getpagesize());

	/* ioctl cmd to allocate contigous memory */
	data = (unsigned)size;
	if( ioctl(cma_fd, ioctl_cmd, &data) == -1){
		__DEBUG("cma_alloc - ioctl command unsuccsessful - \"%s\"\n", strerror(errno));
		return NULL;
	}

	/* at this point phy_addr is written to data */


	/* mmap memory */
	mem = mmap(NULL, size, PROT_WRITE | PROT_READ, MAP_SHARED, cma_fd, data);
	if(mem == MAP_FAILED){
		__DEBUG("cma_alloc - mmap unsuccsessful - \"%s\"\n", strerror(errno));
		return NULL;
	}

	return mem;
}
