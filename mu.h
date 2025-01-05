#ifndef _MU_H_
#define _MU_H_

#include <sys/types.h>

#include <arpa/inet.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MU_UNUSED(x) \
    do               \
    {                \
        (void)(x);   \
    } while (0)

#define MU_MIN(x, y) ({				\
	typeof(x) _min1 = (x);			\
	typeof(y) _min2 = (y);			\
	(void) (&_min1 == &_min2);		\
	_min1 < _min2 ? _min1 : _min2; })

/*
 * assumes LP64.  See:
 *  /usr/include/x86_64-linux/gnu/bits/typesizes.h
 *  /usr/include/x86_64-linux/gnu/bits/types.h
 */
#define MU_PRI_off "ld"  /* long int (s64) */
#define MU_PRI_pid "d"   /* int (s32) */
#define MU_PRI_time "ld" /* long int (s64) */

#define MU_LIMITS_MAX_TIMESTAMP_SIZE 64

#define MU_LIMITS_MAX_IP_STR_SIZE INET6_ADDRSTRLEN /* includes nul */
#define MU_LIMITS_MAX_PORT_STR_SIZE 6              /* max port is 65535, plus nul */
#define MU_LIMITS_MAX_INET_STR_SIZE \
    (MU_LIMITS_MAX_IP_STR_SIZE + MU_LIMITS_MAX_PORT_STR_SIZE)

#define mu_panic(fmt, ...)                          \
    do                                              \
    {                                               \
        fprintf(stderr, "[panic] %s:%d " fmt "\n",  \
                __func__, __LINE__, ##__VA_ARGS__); \
        exit(1);                                    \
    } while (0)

#define mu_panic_errno(fmt, ...)                                      \
    do                                                                \
    {                                                                 \
        fprintf(stderr, "[panic] %s:%d " fmt ": %s\n",                \
                __func__, __LINE__, ##__VA_ARGS__, strerror(errnum)); \
        exit(1);                                                      \
    } while (0)

#define mu_die(fmt, ...)                          \
    do                                            \
    {                                             \
        fprintf(stderr, fmt "\n", ##__VA_ARGS__); \
        exit(1);                                  \
    } while (0)

#define mu_die_errno(errnum, fmt, ...)                                  \
    do                                                                  \
    {                                                                   \
        fprintf(stderr, fmt ": %s\n", ##__VA_ARGS__, strerror(errnum)); \
        exit(1);                                                        \
    } while (0)

#define mu_stderr(fmt, ...) \
    fprintf(stderr, fmt "\n", ##__VA_ARGS__)

#define mu_stderr_errno(errnum, fmt, ...) \
    fprintf(stderr, fmt ": %s\n", ##__VA_ARGS__, strerror(errnum))

#ifdef MU_DEBUG
#define mu_pr_debug(fmt, ...) \
    fprintf(stderr, "[debug] " fmt "\n", ##__VA_ARGS__)
#else
#define mu_pr_debug(fmt, ...) (void)0
#endif

void *mu_malloc(size_t size);
void *mu_calloc(size_t nmemb, size_t size);
void *mu_zalloc(size_t n);
void *mu_realloc(void *ptr, size_t size);
void *mu_mallocarray(size_t nmemb, size_t size);
void *mu_reallocarray(void *ptr, size_t nmemb, size_t size);
char *mu_strdup(const char *s);

#define mu_memzero(ptr, len) (void)memset(ptr, 0x00, len)
#define mu_memzero_p(ptr) (void)memset(ptr, 0x00, sizeof(*ptr))

#define MU_NEW(type, varname) \
    struct type *varname = mu_zalloc(sizeof(*varname))

int mu_str_to_long(const char *s, int base, long *val);
int mu_str_to_int(const char *s, int base, int *val);
int mu_str_to_uint(const char *s, int base, unsigned int *val);
int mu_str_to_u32(const char *s, int base, uint32_t *val);
int mu_str_to_u16(const char *s, int base, uint16_t *val);

size_t mu_str_chomp(char *s);
size_t mu_strlcpy(char *dst, const char *src, size_t dsize);
size_t mu_strlcat(char *dst, const char *src, size_t dsize);
int mu_snprintf(char *str, size_t size, const char *format, ...);

int mu_read_n(int fd, void *data, size_t count, size_t *total);
int mu_pread_n(int fd, void *data, size_t count, off_t offset, size_t *total);
int mu_write_n(int fd, const void *data, size_t count, size_t *total);
int mu_pwrite_n(int fd, const void *data, size_t count, off_t offset, size_t *total);

size_t mu_timestamp_utc(void *buf, size_t buf_size);

void mu_init_sockaddr_in(struct sockaddr_in *sa, const char *ip, const char *port);
uint16_t mu_sockaddr_in_port(const struct sockaddr_in *sa);
size_t mu_sockaddr_in_to_ipstr(const struct sockaddr_in *sa, char *s, size_t size);
size_t mu_sockaddr_in_to_str(const struct sockaddr_in *sa, char *s, size_t size);

void mu_reuseaddr(int sk);
void mu_set_nonblocking(int fd);

#endif /* _MU_H_ */
