#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

// errno

static int g_errno = 0;

int *__errno(void) {
    return &g_errno;
}

long __set_errno_internal(int n) {
    g_errno = n;
    return -1;
}

// Source: bionic/libc/upstream-openbsd/lib/libc/stdlib/getenv.c
static char *__findenv(const char *name, int len, int *offset) {
    int i;
    const char *np;
    char **p, *cp;

    if (name == NULL || environ == NULL)
        return (NULL);
    for (p = environ + *offset; (cp = *p) != NULL; ++p) {
        for (np = name, i = len; i && *cp; i--)
            if (*cp++ != *np++)
                break;
        if (i == 0 && *cp++ == '=') {
            *offset = p - environ;
            return (cp);
        }
    }
    return (NULL);
}

char *getenv(const char *name) {
	int offset = 0;
	const char *np;

	for (np = name; *np && *np != '='; ++np)
		;
	return (__findenv(name, (int)(np - name), &offset));
}

// Source: bionic/libc/upstream-openbsd/lib/libc/stdlib/setenv.c
int setenv(const char *name, const char *value, int rewrite) {
    static char **lastenv;

    char *C, **P;
    const char *np;
    int l_value, offset = 0;

    if (!name || !*name) {
        errno = EINVAL;
        return (-1);
    }
    for (np = name; *np && *np != '='; ++np)
        ;
    if (*np) {
        errno = EINVAL;
        return (-1);			/* has `=' in name */
    }

    l_value = strlen(value);
    if ((C = __findenv(name, (int)(np - name), &offset)) != NULL) {
        int tmpoff = offset + 1;
        if (!rewrite)
            return (0);
#if 0 /* XXX - existing entry may not be writable */
        if (strlen(C) >= l_value) {	/* old larger; copy over */
			while ((*C++ = *value++))
				;
			return (0);
		}
#endif
        /* could be set multiple times */
        while (__findenv(name, (int)(np - name), &tmpoff)) {
            for (P = &environ[tmpoff];; ++P)
                if (!(*P = *(P + 1)))
                    break;
        }
    } else {					/* create new slot */
        size_t cnt = 0;

        if (environ != NULL) {
            for (P = environ; *P != NULL; P++)
                ;
            cnt = P - environ;
        }
        size_t new_size;
        if (__builtin_mul_overflow(cnt + 2, sizeof(char *), &new_size)) {
            errno = ENOMEM;
            return (-1);
        }
        P = realloc(lastenv, new_size);
        if (!P)
            return (-1);
        if (lastenv != environ && environ != NULL)
            memcpy(P, environ, cnt * sizeof(char *));
        lastenv = environ = P;
        offset = cnt;
        environ[cnt + 1] = NULL;
    }
    if (!(environ[offset] =			/* name + `=' + value */
                  malloc((int)(np - name) + l_value + 2)))
        return (-1);
    for (C = environ[offset]; (*C = *name++) && *C != '='; ++C)
        ;
    for (*C++ = '='; (*C++ = *value++); )
        ;
    return (0);
}

// Source: bionic/libc/bionic/libgen.cpp
static int __basename_r(const char *path, char* buffer, size_t buffer_size) {
    const char *startp = NULL;
    const char *endp = NULL;
    int len;
    int result;

    // Empty or NULL string gets treated as ".".
    if (path == NULL || *path == '\0') {
        startp = ".";
        len = 1;
        goto Exit;
    }

    // Strip trailing slashes.
    endp = path + strlen(path) - 1;
    while (endp > path && *endp == '/') {
        endp--;
    }

    // All slashes becomes "/".
    if (endp == path && *endp == '/') {
        startp = "/";
        len = 1;
        goto Exit;
    }

    // Find the start of the base.
    startp = endp;
    while (startp > path && *(startp - 1) != '/') {
        startp--;
    }

    len = endp - startp +1;

Exit:
    result = len;
    if (buffer == NULL) {
        return result;
    }
    if (len > (int) buffer_size - 1) {
        len = buffer_size - 1;
        result = -1;
        errno = ERANGE;
    }

    if (len >= 0) {
        memcpy(buffer, startp, len);
        buffer[len] = 0;
    }
    return result;
}

char *basename(const char *path) {
    static char buf[4069];
    int rc = __basename_r(path, buf, sizeof(buf));
    return (rc < 0) ? NULL : buf;
}

// Simply just abort when abort_message is called
void __wrap_abort_message(const char* format, ...) {
    abort();
}

// Don't care about C++ global destructors
int __cxa_atexit(void (*func) (void *), void * arg, void * dso_handle) {
    return 0;
}

// Emulate pthread functions

typedef struct key_data {
    void *data;
    void (*dtor)(void*);
    int used;
} key_data;

static pthread_key_t key_list_sz = 0;
static key_data *key_list;

int pthread_key_create(pthread_key_t *key_ptr, void (*dtor)(void*)) {
    if (key_list_sz == 0) {
        key_list_sz = 16;
        key_list = calloc(key_list_sz, sizeof(key_data));
    }

    pthread_key_t k = 0;

    // Find an empty slot
    for (; k < key_list_sz; ++k) {
        if (!key_list[k].used) {
            goto set_key;
        }
    }
    // Expand list
    key_list_sz *= 2;
    key_list = realloc(key_list, key_list_sz * sizeof(key_data));
    memset(&key_list[k], 0, k * sizeof(key_data));

set_key:
    *key_ptr = k;
    key_list[k].used = 1;
    key_list[k].dtor = dtor;
    return 0;
}

int pthread_key_delete(pthread_key_t key) {
    if (key < key_list_sz) {
        if (key_list[key].dtor && key_list[key].data) {
            key_list[key].dtor(key_list[key].data);
        }
        key_list[key].data = NULL;
        key_list[key].dtor = NULL;
        key_list[key].used = 0;
    }
    return 0;
}

void *pthread_getspecific(pthread_key_t key) {
    if (key >= key_list_sz || !key_list[key].used) {
        return NULL;
    }
    return key_list[key].data;
}

int pthread_setspecific(pthread_key_t key, const void *value) {
    if (key >= key_list_sz || !key_list[key].used) {
        errno = EINVAL;
        return 1;
    }
    key_list[key].data = (void *) value;
    return 0;
}

// Workaround LTO bug: https://github.com/llvm/llvm-project/issues/61101
#if defined(__i386__)
extern long *_GLOBAL_OFFSET_TABLE_;
long unused() {
    return *_GLOBAL_OFFSET_TABLE_;
}
#endif
