#pragma once

#include <_cheader.h>

_Begin_C_Header

extern void err(int, const char *, ...);
extern void errx(int, const char *, ...);
extern void warn(const char *, ...);
extern void warnx(const char *, ...);

_End_C_Header
