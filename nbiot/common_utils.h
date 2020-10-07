//
//  common_utils.h
//  ML
//
//  Created by Kios on 2020/2/27.
//  Copyright © 2020 Supersure. All rights reserved.
//

#ifndef common_utils_h
#define common_utils_h

#include <stdlib.h>
#include <string.h>


#define __MUST_FREE		
#define ASSERT_UTILS(ptr)       	(ptr != NULL)

#define util_mem_init()			
#define util_alloc(num, type)   	calloc(num, sizeof(type))
#define util_realloc(num, ptr, type)	realloc(ptr, num * sizeof(type))
#define util_memcpy(des, src, n)	memcpy(des, src, n)
#define util_memset(s, c, count)	memset(s, c, count)
#define util_free(p)            	free(p)

#define container_of(ptr, type, member) \
        ((type*)(((unsigned long)ptr) - (unsigned long)(&(((type*)0)->member))))

#endif /* common_utils_h */
