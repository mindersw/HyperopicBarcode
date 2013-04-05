#pragma once

static inline int min(int a, int b) { return a < b ? a : b; }

static inline int max(int a, int b) { return a > b ? a : b; }

int compare_int(const void* a_ptr, const void* b_ptr);

int compare_unsigned_char(const void* a_ptr, const void* b_ptr);

int compare_float(const void* a_ptr, const void* b_ptr);
