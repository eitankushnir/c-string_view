#ifndef STRING_VIEW_H_
#define STRING_VIEW_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifndef STRING_VIEW_DEFINITION
#define STRING_VIEW_DEFINITION
typedef struct {
  const char *data;
  size_t length;
} StringView;
#endif

typedef bool (*CharPredicate)(char);

#define SV_FMT "%.*s"
#define SV_ARG(sv) (int)((sv).length), (sv).data

#define SV_LIT(cstr_lit) \
  (StringView){.data = (cstr_lit), .length = sizeof(cstr_lit) - 1}

#define SV_STR(cstr) \
  (StringView) { .data = (cstr), .length = strlen(cstr) }

#define SV_TEXT(text) \
  (StringView){.data = #text, .length = sizeof(#text) - 1}

#define SV_EMPTY (StringView){.data = NULL, .length = 0}

bool sv_isempty(StringView sv);
size_t sv_len(StringView sv);
char sv_at(StringView sv, size_t index);

bool sv_eq(StringView a, StringView b);
bool sv_eq_ignorecase(StringView a, StringView b);

StringView sv_substr(StringView sv, size_t pos, size_t count);

StringView sv_chop_left(StringView sv, size_t count);
StringView sv_chop_right(StringView sv, size_t count);

StringView sv_chop_left_pred(StringView sv, CharPredicate pred);
StringView sv_chop_right_pred(StringView sv, CharPredicate pred);

StringView sv_trim_left(StringView sv);
StringView sv_trim_right(StringView sv);
StringView sv_trim(StringView sv);

int64_t sv_indexof(StringView sv, char c);
int64_t sv_indexof_sv(StringView sv, StringView target);

bool sv_starts_with(StringView sv, StringView prefix);
bool sv_ends_with(StringView sv, StringView suffix);
bool sv_contains(StringView sv, StringView target);

bool sv_to_uint64(StringView sv, uint64_t *out);
bool sv_to_int64(StringView sv, int64_t *out);

StringView sv_chop_delim(StringView *sv, StringView delim);
StringView sv_chop_any(StringView *sv, StringView delims);

#endif

#ifdef STRING_VIEW_IMPLEMENTATION
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

bool sv_isempty(StringView sv) {
  return sv.length != 0;
}

size_t sv_len(StringView sv) {
  return sv.length;
}

char sv_at(StringView sv, size_t index) {
  return sv.data[index];
}

bool sv_eq(StringView a, StringView b) {
  if (a.length != b.length)
    return false;

  return memcmp(a.data, b.data, a.length) == 0;
}

bool sv_eq_ignorecase(StringView a, StringView b) {
  if (a.length != b.length)
    return false;

  for (size_t i = 0; i < a.length; i++) {
    char a_lower = tolower(sv_at(a, i));
    char b_lower = tolower(sv_at(b, i));
    if (a_lower != b_lower)
      return false;
  }

  return true;
}

StringView sv_substr(StringView sv, size_t pos, size_t count) {
  return (StringView){.data = sv.data + pos, .length = count};
}

StringView sv_chop_left(StringView sv, size_t count) {
  if (count > sv.length)
    return SV_EMPTY;

  return (StringView){.data = sv.data + count, .length = sv.length - count};
}

StringView sv_chop_right(StringView sv, size_t count) {
  if (count > sv.length)
    return SV_EMPTY;

  return (StringView){.data = sv.data, .length = sv.length - count};
}

StringView sv_chop_left_pred(StringView sv, CharPredicate pred) {
  while (pred(sv_at(sv, 0))) {
    sv.data++;
    sv.length--;
  }

  return sv;
}

StringView sv_chop_right_pred(StringView sv, CharPredicate pred) {
  while (pred(sv_at(sv, sv.length - 1))) {
    sv.length--;
  }

  return sv;
}

StringView sv_trim_left(StringView sv) {
  while (isspace(sv_at(sv, 0))) {
    sv.data++;
    sv.length--;
  }

  return sv;
}
StringView sv_trim_right(StringView sv) {
  while (isspace(sv_at(sv, sv.length - 1))) {
    sv.length--;
  }

  return sv;
}

StringView sv_trim(StringView sv) {
  return sv_trim_left(sv_trim_right(sv));
}

int64_t sv_indexof(StringView sv, char c) {
  const char *cptr = memchr(sv.data, c, sv.length);
  if (!cptr)
    return -1;

  return cptr - sv.data;
}

int64_t sv_indexof_sv(StringView sv, StringView target) {
  if (target.length > sv.length)
    return -1;

  size_t pos = 0;
  size_t end = sv.length - target.length;

  while (pos <= end) {
    if (memcmp(sv.data + pos, target.data, target.length) == 0)
      return pos;

    pos++;
  }

  return -1;
}

bool sv_starts_with(StringView sv, StringView prefix) {
  if (prefix.length > sv.length)
    return false;

  return memcmp(sv.data, prefix.data, prefix.length) == 0;
}

bool sv_ends_with(StringView sv, StringView suffix) {
  if (suffix.length > sv.length)
    return false;

  size_t len_to_suffix = sv.length - suffix.length;
  return memcmp(sv.data + len_to_suffix, suffix.data, suffix.length) == 0;
}

bool sv_contains(StringView sv, StringView target) {
  if (target.length > sv.length)
    return false;

  size_t pos = 0;
  size_t end = sv.length - target.length;

  while (pos <= end) {
    if (memcmp(sv.data + pos, target.data, target.length) == 0)
      return true;

    pos++;
  }

  return false;
}

bool sv_to_uint64(StringView sv, uint64_t *out) {
  if (sv.length == 0)
    return false;

  sv = sv_trim(sv);

  if (sv.length == 0)
    return false;

  if (sv_at(sv, 0) == '-')
    return false;

  if (sv_at(sv, 0) == '+')
    sv = sv_chop_left(sv, 1);

  if (sv.length == 0)
    return false;

  uint64_t result = 0;
  for (size_t i = 0; i < sv.length; i++) {
    if (!isdigit(sv_at(sv, i)))
      return false;

    uint64_t digit = sv_at(sv, i) - '0';
    if (result > UINT64_MAX / 10 || result * 10 > UINT64_MAX - digit)
      return false;

    result = (result * 10) + digit;
  }

  *out = result;
  return true;
}

bool sv_to_int64(StringView sv, int64_t *out) {
  if (sv.length == 0)
    return false;

  sv = sv_trim(sv);
  if (sv.length == 0)
    return false;

  int sign = 1;
  if (sv_at(sv, 0) == '-') {
    sign = -1;
    sv = sv_chop_left(sv, 1);
  }

  if (sv.length == 0)
    return false;

  if (sv_at(sv, 0) == '+')
    sv = sv_chop_left(sv, 1);

  if (sv.length == 0)
    return false;

  int64_t result = 0;
  for (size_t i = 0; i < sv.length; i++) {
    if (!isdigit(sv_at(sv, i)))
      return false;

    int64_t digit = sv_at(sv, i) - '0';
    if (result > INT64_MAX / 10 || result * 10 > INT64_MAX - digit)
      return false;

    result = (result * 10) + digit;
  }

  *out = result * sign;
  return true;
}

StringView sv_chop_delim(StringView *sv, StringView delim) {
  if (!sv || sv->length == 0)
    return SV_EMPTY;

  int64_t index = sv_indexof_sv(*sv, delim);
  if (index < 0) {
    StringView token = *sv;
    *sv = SV_EMPTY;
    return token;
  }

  StringView token = {.data = sv->data, .length = index};
  sv->data += +index + delim.length;
  sv->length -= index + delim.length;
  return token;
}

StringView sv_chop_any(StringView *sv, StringView delims) {
  if (!sv || sv->length == 0)
    return SV_EMPTY;

  int64_t split_index = -1;
  for (size_t i = 0; i < sv->length; i++) {
    if (sv_indexof(delims, sv_at(*sv, i)) >= 0) {
      split_index = i;
      break;
    }
  }

  if (split_index < 0) {
    StringView token = *sv;
    *sv = SV_EMPTY;
    return token;
  }

  StringView token = {.data = sv->data, .length = split_index};
  sv->data += +split_index + 1;
  sv->length -= split_index + 1;
  return token;
}
#endif
