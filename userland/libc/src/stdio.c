#include "stdio.h"
#include "string.h"
#include "unistd.h"

int putchar(int c)
{
    char ch = (char)c;
    return sys_write(1, &ch, 1);
}

int puts(const char *s)
{
    int len = sys_write(1, s, strlen(s));
    sys_write(1, "\n", 1);
    return len + 1;
}

char *gets(char *buf, unsigned int max_size)
{
    int bytes = sys_read(0, buf, max_size);
    if (bytes <= 0) {
        return (char *)0;
    }
    return buf;
}

static void print_number(int num, int base)
{
    char buf[32];
    int i = 0;
    unsigned int n;

    if (base == 10 && num < 0) {
        putchar('-');
        n = (unsigned int)(-num);
    } else {
        n = (unsigned int)num;
    }

    if (n == 0) {
        putchar('0');
        return;
    }

    while (n > 0) {
        int rem = n % base;
        buf[i++] = (rem < 10) ? ('0' + rem) : ('a' + rem - 10);
        n /= base;
    }

    while (i > 0) {
        putchar(buf[--i]);
    }
}

int printf(const char *format, ...)
{
    __builtin_va_list args;
    __builtin_va_start(args, format);

    while (*format) {
        if (*format == '%') {
            format++;
            switch (*format) {
            case 'c': {
                char c = (char)__builtin_va_arg(args, int);
                putchar(c);
                break;
            }
            case 's': {
                const char *s = __builtin_va_arg(args, const char *);
                if (!s) {
                    s = "(null)";
                }
                sys_write(1, s, strlen(s));
                break;
            }
            case 'd':
            case 'i': {
                int d = __builtin_va_arg(args, int);
                print_number(d, 10);
                break;
            }
            case 'x':
            case 'p': {
                int x = __builtin_va_arg(args, int);
                print_number(x, 16);
                break;
            }
            case '%': {
                putchar('%');
                break;
            }
            default:
                putchar('%');
                putchar(*format);
                break;
            }
        } else {
            putchar(*format);
        }
        format++;
    }

    __builtin_va_end(args);
    return 0;
}
