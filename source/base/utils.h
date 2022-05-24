/* date = March 11th 2022 2:07 pm */

#ifndef UTILS_H
#define UTILS_H

#include "defines.h"
#include "mem.h"
#include "str.h"

//~ Time

typedef u64 U_DenseTime;

typedef struct U_DateTime {
    u16 ms;
    u8  sec;
    u8  minute;
    u8  hour;
    u8  day;
    u8  month;
    s32 year;
} U_DateTime;

U_DenseTime U_DenseTimeFromDateTime(U_DateTime* datetime);
U_DateTime  U_DateTimeFromDenseTime(U_DenseTime densetime);

//~ Filepaths

string U_FixFilepath(M_Arena* arena, string filepath);
string U_GetFullFilepath(M_Arena* arena, string filename);
string U_GetFilenameFromFilepath(string filepath);
string U_GetDirectoryFromFilepath(string filepath);

//~ Optional values

#define Optional_Define(type) \
typedef struct type##_optional {\
b32 valid;\
type value;\
} type##_optional;
#define Optional_Set(var,val) do { var.value = val; var.valid = true; } while(0)

// 4coder syntax highlighting
#if 0
typedef u8_optional; typedef u16_optional; typedef u32_optional; typedef u64_optional;
typedef i8_optional; typedef i16_optional; typedef i32_optional; typedef i64_optional;
typedef b8_optional; typedef b32_optional;
#endif

Optional_Define(u8);
Optional_Define(u16);
Optional_Define(u32);
Optional_Define(u64);
Optional_Define(i8);
Optional_Define(i16);
Optional_Define(i32);
Optional_Define(i64);
Optional_Define(b8);
Optional_Define(b32);

#endif //UTILS_H
