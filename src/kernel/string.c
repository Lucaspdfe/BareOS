#include "string.h"

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}
char* strcpy(char* dest, const char* src) {
    char* originalDest = dest;
    while ((*dest++ = *src++) != '\0');
    return originalDest;
}
char* strncpy(char* dest, const char* src, size_t n) {
    char* originalDest = dest;
    while (n-- && (*dest++ = *src++) != '\0');
    while (n--) *dest++ = '\0'; // pad with nulls
    return originalDest;
}
char* strcat(char* dest, const char* src) {
    char* originalDest = dest;
    while (*dest) dest++;
    while ((*dest++ = *src++) != '\0');
    return originalDest;
}
char* strncat(char* dest, const char* src, size_t n) {
    char* originalDest = dest;
    while (*dest) dest++;
    while (n-- && (*dest++ = *src++) != '\0');
    *dest = '\0'; // null-terminate
    return originalDest;
}
int strcmp(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}
int strncmp(const char* str1, const char* str2, size_t n) {
    while (n-- && *str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    if (n == (size_t)-1) return 0; // equal up to n characters
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}
void* memset(void* s, int c, size_t n) {
    unsigned char* p = s;
    while (n--) {
        *p++ = (unsigned char)c;
    }
    return s;
}
void* memcpy(void* dest, const void* src, size_t n) {
    unsigned char* d = dest;
    const unsigned char* s = src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}
void* memmove(void* dest, const void* src, size_t n) {
    unsigned char* d = dest;
    const unsigned char* s = src;
    if (d < s) {
        while (n--) {
            *d++ = *s++;
        }
    } else {
        d += n;
        s += n;
        while (n--) {
            *(--d) = *(--s);
        }
    }
    return dest;
}
int memcmp(const void* s1, const void* s2, size_t n) {
    const unsigned char* p1 = s1;
    const unsigned char* p2 = s2;
    while (n--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    return 0;
}
char* strchr(const char* str, int c) {
    while (*str) {
        if (*str == (char)c) {
            return (char*)str;
        }
        str++;
    }
    return NULL;
}
char* strrchr(const char* str, int c) {
    const char* last = NULL;
    while (*str) {
        if (*str == (char)c) {
            last = str;
        }
        str++;
    }
    return (char*)last;
}
char* strstr(const char* haystack, const char* needle) {
    if (!*needle) return (char*)haystack;
    for (; *haystack; haystack++) {
        if ((*haystack == *needle) && !strncmp(haystack, needle, strlen(needle))) {
            return (char*)haystack;
        }
    }
    return NULL;
}
size_t strspn(const char* str1, const char* str2) {
    size_t len = 0;
    while (*str1 && strchr(str2, *str1)) {
        len++;
        str1++;
    }
    return len;
}
size_t strcspn(const char* str1, const char* str2) {
    size_t len = 0;
    while (*str1 && !strchr(str2, *str1)) {
        len++;
        str1++;
    }
    return len;
}
char* strtok(char* str, const char* delim) {
    static char* next = NULL; // keeps track of next start position
    if (str) next = str;
    if (!next) return NULL;

    // Skip leading delimiters
    while (*next && strchr(delim, *next)) {
        next++;
    }

    if (!*next) {
        next = NULL;
        return NULL;
    }

    // Start of token
    char* token = next;

    // Find end of token
    while (*next && !strchr(delim, *next)) {
        next++;
    }

    if (*next) {
        *next = '\0'; // null-terminate token
        next++;       // move past delimiter for next call
    } else {
        next = NULL;  // no more tokens
    }

    return token;
}
