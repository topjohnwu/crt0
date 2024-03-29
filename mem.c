#include <string.h>
#include <malloc.h>

void *memset(void *dst, int ch, size_t n) {
    uint8_t *d = dst;
    uint8_t c = ch;
    while (n--)
        (*d++) = c;
    return dst;
}

void *memmove(void *dst, const void *src, size_t n) {
    if (dst < src) {
        return memcpy(dst, src, n);
    }
    // Copy backwards
    uint8_t *d = dst + n;
    const uint8_t *s = src + n;
    while (n--)
        *--d = *--s;
    return dst;
}

void *memcpy(void * restrict dst, const void * restrict src, size_t n) {
    uint8_t *d = dst;
    const uint8_t *s = src;
    while (n--)
        *d++ = *s++;
    return dst;
}

int memcmp(const void *lhs, const void *rhs, size_t n) {
    const uint8_t *l = lhs;
    const uint8_t *r = rhs;
    while (n--) {
        if (*l != *r) {
            return *l - *r;
        } else {
            l++;
            r++;
        }
    }
    return 0;
}

void *memchr(const void *ptr, int ch, size_t n) {
    const uint8_t *p = ptr;
    uint8_t c = ch;
    while (n--) {
        if (*p != c)
            ++p;
        else
            return (void *) p;
    }
    return NULL;
}

char *strchr(const char *s, int ch) {
    char c = ch;
    while (*s != c)
        if (!*s++)
            return NULL;
    return (char *) s;
}

char *strrchr(const char *s, int ch) {
    char c = ch;
    const char *ret = NULL;
    do {
        if(*s == c)
            ret = s;
    } while(*s++);
    return (char *) ret;
}

int strcmp(const char *lhs, const char *rhs) {
    while (*lhs && (*lhs == *rhs)) {
        ++lhs;
        ++rhs;
    }
    return *(uint8_t *)lhs - *(uint8_t *)rhs;
}

size_t strlen(const char *str) {
    size_t l = 0;
    while (str[l])
        ++l;
    return l;
}

size_t strnlen(const char *s, size_t maxlen) {
    size_t l = 0;
    while (l < maxlen && s[l])
        ++l;
    return l;
}

char *strcpy(char *restrict dest, const char *restrict src) {
    char *ret = dest;
    while ((*dest++ = *src++)) {}
    return ret;
}

char *strdup(const char *str) {
    size_t siz;
    char *copy;
    siz = strlen(str) + 1;
    if ((copy = malloc(siz)) == NULL)
        return NULL;
    memcpy(copy, str, siz);
    return copy;
}

// Source: bionic/libc/upstream-openbsd/lib/libc/string/strlcpy.c
size_t strlcpy(char *dst, const char *src, size_t dsize) {
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

// Source: bionic/libc/upstream-openbsd/lib/libc/string/strtok.c
char *strtok_r(char *s, const char *delim, char **last) {
    const char *spanp;
    int c, sc;
    char *tok;

    if (s == NULL && (s = *last) == NULL)
        return (NULL);

    /*
     * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
     */
    cont:
    c = *s++;
    for (spanp = delim; (sc = *spanp++) != 0;) {
        if (c == sc)
            goto cont;
    }

    if (c == 0) {		/* no non-delimiter characters */
        *last = NULL;
        return (NULL);
    }
    tok = s - 1;

    /*
     * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
     * Note that delim must have one NUL; we stop if we see that, too.
     */
    for (;;) {
        c = *s++;
        spanp = delim;
        do {
            if ((sc = *spanp++) == c) {
                if (c == 0)
                    s = NULL;
                else
                    s[-1] = '\0';
                *last = s;
                return (tok);
            }
        } while (sc != 0);
    }
    /* NOTREACHED */
}

// strcasecmp source: bionic/libc/upstream-openbsd/lib/libc/string/strcasecmp.c

typedef unsigned char u_char;

/*
 * This array is designed for mapping upper and lower case letter
 * together for a case independent comparison.  The mappings are
 * based upon ascii character sequences.
 */
static const u_char charmap[] = {
        '\000', '\001', '\002', '\003', '\004', '\005', '\006', '\007',
        '\010', '\011', '\012', '\013', '\014', '\015', '\016', '\017',
        '\020', '\021', '\022', '\023', '\024', '\025', '\026', '\027',
        '\030', '\031', '\032', '\033', '\034', '\035', '\036', '\037',
        '\040', '\041', '\042', '\043', '\044', '\045', '\046', '\047',
        '\050', '\051', '\052', '\053', '\054', '\055', '\056', '\057',
        '\060', '\061', '\062', '\063', '\064', '\065', '\066', '\067',
        '\070', '\071', '\072', '\073', '\074', '\075', '\076', '\077',
        '\100', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
        '\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
        '\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
        '\170', '\171', '\172', '\133', '\134', '\135', '\136', '\137',
        '\140', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
        '\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
        '\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
        '\170', '\171', '\172', '\173', '\174', '\175', '\176', '\177',
        '\200', '\201', '\202', '\203', '\204', '\205', '\206', '\207',
        '\210', '\211', '\212', '\213', '\214', '\215', '\216', '\217',
        '\220', '\221', '\222', '\223', '\224', '\225', '\226', '\227',
        '\230', '\231', '\232', '\233', '\234', '\235', '\236', '\237',
        '\240', '\241', '\242', '\243', '\244', '\245', '\246', '\247',
        '\250', '\251', '\252', '\253', '\254', '\255', '\256', '\257',
        '\260', '\261', '\262', '\263', '\264', '\265', '\266', '\267',
        '\270', '\271', '\272', '\273', '\274', '\275', '\276', '\277',
        '\300', '\301', '\302', '\303', '\304', '\305', '\306', '\307',
        '\310', '\311', '\312', '\313', '\314', '\315', '\316', '\317',
        '\320', '\321', '\322', '\323', '\324', '\325', '\326', '\327',
        '\330', '\331', '\332', '\333', '\334', '\335', '\336', '\337',
        '\340', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
        '\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
        '\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
        '\370', '\371', '\372', '\373', '\374', '\375', '\376', '\377',
};

int strcasecmp(const char *s1, const char *s2) {
    const u_char *cm = charmap;
    const u_char *us1 = (const u_char *)s1;
    const u_char *us2 = (const u_char *)s2;

    while (cm[*us1] == cm[*us2++])
        if (*us1++ == '\0')
            return (0);
    return (cm[*us1] - cm[*--us2]);
}
