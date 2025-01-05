#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "mu.h"


void *
mu_malloc(size_t size)
{
    void *p;

    p = malloc(size);
    if (p == NULL)
        mu_panic("out of memory");

    return p;
}


void *
mu_calloc(size_t nmemb, size_t size)
{
    void *p;

    p = calloc(nmemb, size);
    if (p == NULL)
        mu_panic("out of memory");

    return p;
}


void *
mu_zalloc(size_t n)
{
    return mu_calloc(1, n);
}


void *
mu_realloc(void *ptr, size_t size)
{
    void *p = realloc(ptr, size);
    if (!p)
        mu_panic("out of memory");

    return p;
}


void *
mu_mallocarray(size_t nmemb, size_t size)
{
    void *p = NULL;
    size_t n = 0;

    if (__builtin_umull_overflow(nmemb, size, &n))
        mu_panic("integer overflow: %zu * %zu", nmemb, size);

    p = malloc(n);
    if (p == NULL)
        mu_panic("out of memory");

    return p;
}


void *
mu_reallocarray(void *ptr, size_t nmemb, size_t size)
{
    void *p = NULL;
    size_t n = 0;

    if (__builtin_umull_overflow(nmemb, size, &n))
        mu_panic("integer overflow: %zu * %zu", nmemb, size);

    p = mu_realloc(ptr, n);
    return p;
}


char *
mu_strdup(const char *s)
{
    char *p;

    p = strdup(s);
    if (p == NULL)
        mu_panic("out of memory");

    return p;
}


/* 
 * On success, return 0 and set val to the parsed value.
 * On failure, return a negative errno value.
 */
int
mu_str_to_long(const char *s, int base, long *val)
{
    char *endptr;

    errno = 0;
    *val = strtol(s, &endptr, base);
    if (errno != 0) {
        /* EINVAL for bad base, or ERANGE for value to big or small */
        return -errno;
    }

    if (endptr == s) {
        /* no digits at all -- not a number */
        return -EINVAL;
    }

    if (*endptr != '\0') {
        /* trailing garbage */
        return -EINVAL;
    }

    return 0;
}


/* 
 * On success, return 0 and set val to the parsed value.
 * On failure, return a negative errno value.
 */
int
mu_str_to_int(const char *s, int base, int *val)
{
    int ret;
    long tmp;

    ret = mu_str_to_long(s, base, &tmp); 
    if (ret < 0)
        return ret;

    if (tmp < INT_MIN || tmp > INT_MAX)
        return -ERANGE;

    *val = (int)tmp;
    return 0;
}


int
mu_str_to_uint(const char *s, int base, unsigned int *val)
{
    int ret;
    long tmp = 0;

    ret = mu_str_to_long(s, base, &tmp); 
    if (ret < 0)
        return ret;

    if (tmp < 0 || tmp > UINT_MAX)
        return -ERANGE;

    *val = (unsigned int)tmp;
    return 0;
}


int
mu_str_to_u32(const char *s, int base, uint32_t *val)
{
    int ret;
    long tmp = 0;

    ret = mu_str_to_long(s, base, &tmp); 
    if (ret < 0)
        return ret;

    if (tmp < 0 || tmp > UINT32_MAX)
        return -ERANGE;

    *val = (uint32_t)tmp;
    return 0;
}


int
mu_str_to_u16(const char *s, int base, uint16_t *val)
{
    int ret;
    long tmp = 0;

    ret = mu_str_to_long(s, base, &tmp); 
    if (ret < 0)
        return ret;

    if (tmp < 0 || tmp > UINT16_MAX)
        return -ERANGE;

    *val = (uint16_t)tmp;
    return 0;
}



/*
 * If the last char of `s` is a newline, remove it (overwrite it with a
 * nul-byte).
 *
 * Return 1 if a newline was removed, and 0 otherwise.
 */
size_t
mu_str_chomp(char *s)
{
    size_t len = strlen(s);

    if ((len > 0) && (s[len-1] == '\n')) {
        s[len-1] = '\0';
        return 1;
    } else {
        return 0;
    }
}


/*
 * mu_strlcpy and mu_strlcat are taken from OpenBSD 6.2's
 * lib/libc/string/strlcpy.c and lib/libc/string/strlcat.c, respectively.
 */

/*
 * Copy string src to buffer dst of size dsize.  At most dsize-1
 * chars will be copied.  Always NUL terminates (unless dsize == 0).
 * Returns strlen(src); if retval >= dsize, truncation occurred.
 */
size_t
mu_strlcpy(char *dst, const char *src, size_t dsize)
{
	const char *osrc = src;
	size_t nleft = dsize;

	/* Copy as many bytes as will fit. */
	if (nleft != 0) {
		while (--nleft != 0) {
			if ((*dst++ = *src++) == '\0')
				break;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src. */
	if (nleft == 0) {
		if (dsize != 0)
			*dst = '\0';		/* NUL-terminate dst */
		while (*src++)
			;
	}

	return(src - osrc - 1);	/* count does not include NUL */
}

/*
 * Appends src to string dst of size dsize (unlike strncat, dsize is the
 * full size of dst, not space left).  At most dsize-1 characters
 * will be copied.  Always NUL terminates (unless dsize <= strlen(dst)).
 * Returns strlen(src) + MIN(dsize, strlen(initial dst)).
 * If retval >= dsize, truncation occurred.
 *
 * d = "abcd" 0 0 0 0
 * strlcpy(d, 8, "ef")  This would return 6
 * 8 - 4 - 1 = 3
 */
size_t
mu_strlcat(char *dst, const char *src, size_t dsize)
{
	const char *odst = dst;
	const char *osrc = src;
	size_t n = dsize;
	size_t dlen;

	/* Find the end of dst and adjust bytes left but don't go past end. */
	while (n-- != 0 && *dst != '\0')
		dst++;
	dlen = dst - odst;
	n = dsize - dlen;

	if (n-- == 0)
		return(dlen + strlen(src));
	while (*src != '\0') {
		if (n != 0) {
			*dst++ = *src;
			n--;
		}
		src++;
	}
	*dst = '\0';

	return(dlen + (src - osrc));	/* count does not include NUL */
}


/* terminates if snprintf errors or truncates */
int
mu_snprintf(char *str, size_t size, const char *format, ...)
{
    va_list ap;
    int len;

    va_start(ap, format);
    len = vsnprintf(str, size, format, ap);
    va_end(ap);

    if (len < 0) {
        mu_panic("snprintf(size=%zu, format=\"%s\") failed (returned %d)",
                size, format, len);
    }

    if ((size_t)len >= size) {
        mu_panic("snprintf(size=%zu, format=\"%s\") truncated (returned %d)",
                size, format, len);
    } 

    return len;
}




/*
 * Read `count` bytes from fd.
 *
 * On success, return 0.  On failure, return a negative errno.  In either case,
 * if the `total` argument is non-NULL, store the total number of bytes read.
 * Thus, if the function returns 0 and total == count, then all bytes were
 * read, but if total < count, then EOF was reached before reading all
 * requested bytes.
 *
 * The read is restarted in the event of interruption.
 */
int
mu_read_n(int fd, void *data, size_t count, size_t *total)
{
    int err = 0;
    ssize_t n;
    size_t avail = count;
    size_t tot = 0;

    do {
retry:
        n = read(fd, (uint8_t *)data + tot, avail);
        if (n == -1) {
            if (errno == EINTR) {
                goto retry;
            } else {
                err = -errno;
                goto out;
            }
        } else if (n == 0) {
            goto out;
        } else {
            avail -= (size_t)n;
            tot += (size_t)n;
        }
    } while (avail);

out:
    if (total != NULL)
        *total = tot;
    return err;
}


/*
 * Read `count` bytes from fd without changing the file's offset.
 *
 * On success, return 0.  On failure, return a negative errno.  In either case,
 * if the `total` argument is non-NULL, store the total number of bytes read.
 * Thus, if the function returns 0 and total == count, then all bytes were
 * read, but if total < count, then EOF was reached before reading all
 * requested bytes.
 *
 * The read is restarted in the event of interruption.
 */
int
mu_pread_n(int fd, void *data, size_t count, off_t offset, size_t *total)
{
    int err = 0;
    ssize_t n;
    size_t avail = count;
    size_t tot = 0;

    do {
retry:
        n = pread(fd, (uint8_t *)data + tot, avail, offset + (ssize_t)tot);
        if (n == -1) {
            if (errno == EINTR) {
                goto retry;
            } else {
                err = -errno;
                goto out;
            }
        } else if (n == 0) {
            goto out;
        } else {
            avail -= (size_t)n;
            tot += (size_t)n;
        }
    } while (avail);

out:
    if (total != NULL)
        *total = tot;
    return err;
}


/*
 * Write `count` bytes to fd.
 *
 * On success, return 0.  On failure, return a negative errno.  In either case,
 * if the `total` argument is non-NULL, store the total number of bytes written.
 *
 * The write is restarted in the event of interruption.
 */
int
mu_write_n(int fd, const void *data, size_t count, size_t *total)
{
    int err = 0;
    ssize_t n = 0;
    size_t left = count;
    size_t tot = 0;

    do {
retry:
        n = write(fd, (uint8_t *)data + tot, left);
        if (n == -1) {
            if (errno == EINTR) {
                goto retry;
            } else {
                err = -errno;
                goto out;
            }
        } else {
            left -= (size_t)n;
            tot += (size_t)n;
        }
    } while (left);

out:
    if (total != NULL)
        *total = tot;
    return err;
}


/*
 * Write `count` bytes to fd without changing file offset.
 *
 * On success, return 0.  On failure, return a negative errno.  In either case,
 * if the `total` argument is non-NULL, store the total number of bytes written.
 *
 * The write is restarted in the event of interruption.
 */
int
mu_pwrite_n(int fd, const void *data, size_t count, off_t offset, size_t *total)
{
    int err = 0;
    ssize_t n = 0;
    size_t left = count;
    size_t tot = 0;

    do {
retry:
        n = pwrite(fd, (uint8_t *)data + tot, left, offset + (ssize_t)tot);
        if (n == -1) {
            if (errno == EINTR) {
                goto retry;
            } else {
                err = -errno;
                goto out;
            }
        } else {
            left -= (size_t)n;
            tot += (size_t)n;
        }
    } while (left);

out:
    if (total != NULL)
        *total = tot;
    return err;
}


size_t
mu_timestamp_utc(void *buf, size_t buf_size)
{
    time_t t;
    struct tm tm;
    size_t n;
    char stamp[MU_LIMITS_MAX_TIMESTAMP_SIZE] = { 0 };

    (void)time(&t);
    (void)gmtime_r(&t, &tm);

    n = strftime(stamp, sizeof(stamp), "%Y/%m/%d %H:%M:%S UTC", &tm);
    if (n == 0)
        mu_panic("strftime");

    return mu_strlcpy(buf, stamp, buf_size);
}


void
mu_init_sockaddr_in(struct sockaddr_in *sa, const char *ip, const char *port)
{
    uint16_t tmp;

    memset(sa, 0x00, sizeof(*sa));    
    sa->sin_family = AF_INET;

    if (inet_pton(AF_INET, ip, &sa->sin_addr) != 1)
        mu_die("invalid IP address");

    if (mu_str_to_u16(port, 10, &tmp) != 0)
        mu_die("invalid port number");

    sa->sin_port = htons(tmp);
}



/* Return the port number for a struct sockaddr_in in host byte-order */
uint16_t
mu_sockaddr_in_port(const struct sockaddr_in *sa)
{
    return ntohs(sa->sin_port);
}


/* 
 * Convert the address part of struct sockaddr_in to a string.
 * The string does not include the port number.
 *
 * Returns the actual strlen of the address.  If this is >= size, then the
 * result was truncated.  Always nul-terminates the output string.
 */
size_t
mu_sockaddr_in_to_ipstr(const struct sockaddr_in *sa, char *s, size_t size)
{
    char buf[MU_LIMITS_MAX_IP_STR_SIZE] = { 0 };
    const char *real = NULL;
    size_t real_len, real_size, ncopy;

    real = inet_ntop(AF_INET, &sa->sin_addr, buf, sizeof(buf));

    assert(real != NULL);

    real_len = strlen(real);
    real_size = real_len + 1;
    ncopy = MU_MIN(size, real_size);
    memcpy(s, real, ncopy);
    if (ncopy > 0)
        s[ncopy - 1] = '\0';
    
    return real_len;
}


/* 
 * Convert a struct sockaddr_in oto a string string. The string is of the form addr:port.
 *
 * Returns the actual strlen of the address.  If this is >= size, then the
 * result was truncated.  Always nul terminates the output string.
 */
size_t
mu_sockaddr_in_to_str(const struct sockaddr_in *sa, char *s, size_t size)
{
    char buf[MU_LIMITS_MAX_INET_STR_SIZE] = { 0 };
    size_t buf_size = sizeof(buf);
    uint16_t port;
    size_t len, ncopy;
    int n;

    len = mu_sockaddr_in_to_ipstr(sa, buf, buf_size);
    assert(len < buf_size);

    port = mu_sockaddr_in_port(sa);

    n = snprintf(buf + len, buf_size - len, ":%d", port);
    assert(n >= 0 && ((size_t)n < (buf_size - len)));

    len += (size_t)n;

    ncopy = MU_MIN(len + 1, size);
    memcpy(s, buf, ncopy);
    if (ncopy > 0)
        s[ncopy - 1] = '\0';

    return len;
}


void
mu_reuseaddr(int sk)
{
    int optval = 1;
    if (setsockopt(sk, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
        mu_die_errno(errno, "setsockopt(%d, SOL_SOCKET, SO_REUSEADDR)", sk);
} 


void
mu_set_nonblocking(int fd)
{
    int err = 0;
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        mu_die_errno(errno, "fcntl(%d, F_GETFL)", fd);

    err = fcntl(fd, F_SETFL, flags | O_NONBLOCK); 
    if (err == -1)
        mu_die_errno(errno, "fcntl(%d, F_SETFL, O_NONBLOCK)", fd);
}
