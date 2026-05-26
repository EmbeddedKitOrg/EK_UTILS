/**
 * @file ek_picolibc_port.c
 * @brief picolibc 适配层
 * @author N1netyNine99
 *
 * 统一 picolibc 与 ek_utils 的内存分配器和字符输出：
 * - 场景 A (EKCFG_HEAP_TLSF=1): picolibc 的 malloc/free/realloc → ek_malloc/ek_free/ek_realloc (TLSF)
 * - 场景 B (EKCFG_HEAP_TLSF=0): ek_malloc/ek_free/ek_realloc → picolibc 的 malloc/free/realloc
 * - stdout: putchar → _ek_io_fputc
 */

#include "ek_conf.h"

#if EKCFG_PICOLIBC == 1

#    include "ek_mem.h"
#    include "ek_io.h"

/* ========== 场景 A: TLSF 分配器，picolibc 调用走 TLSF ========== */
#    if EKCFG_HEAP_TLSF == 1

void *malloc(size_t size)
{
    return ek_malloc(size);
}

void free(void *ptr)
{
    ek_free(ptr);
}

void *realloc(void *ptr, size_t size)
{
    return ek_realloc(ptr, size);
}

#    else /* EKCFG_HEAP_TLSF == 0 */

/* ========== 场景 B: picolibc 分配器，ek 调用走 picolibc ========== */

void *ek_malloc(size_t size)
{
    return malloc(size);
}

void ek_free(void *ptr)
{
    free(ptr);
}

void *ek_realloc(void *ptr, size_t size)
{
    return realloc(ptr, size);
}

#    endif /* EKCFG_HEAP_TLSF */

/* ========== stdout 输出对接 ========== */

int putchar(int ch)
{
    _ek_io_fputc(ch);
    return ch;
}

#endif /* EKCFG_PICOLIBC */
