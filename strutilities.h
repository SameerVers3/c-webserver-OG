#ifndef __STRUTILITIES_H
#define __STRUTILITIES_H

#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>

int starts_with(char *str, const char *prefix) {
  return strncmp(str, prefix, strlen(prefix)) == 0;
}

int ends_with(const char *str, const char *suffix) {
  int str_length = strlen(str);
  int suffix_length = strlen(suffix);

  if (suffix_length <= str_length) {
    return strncmp(str + str_length - suffix_length, suffix, suffix_length) == 0;
  } else {
    return 0;
  }
}

int contains(const char *str1, const char *str2) {
  return strstr(str1, str2) != NULL;
}

char* substr(const char* input, int offset, int length, char* dest) {
  int input_length = strlen(input);

  if (offset + length > input_length) {
    return NULL;
  }

  strncpy(dest, input + offset, length);
  return dest;
}

int ends_with_extension(const char *input) {
  int end_position = strlen(input);

  while (--end_position >= 0) {
    if (input[end_position] == '.') return 1;
    if (!isalpha(input[end_position])) return 0;
  }

  return 0;
}

char *concat(const char *str1, const char *str2) {
  char *result = malloc(strlen(str1) + strlen(str2) + 1);
  strcpy(result, str1);
  strcat(result, str2);
  return result;
}

char *concat3(const char *str1, const char *str2, const char *str3) {
  char *result = malloc(strlen(str1) + strlen(str2) + strlen(str3) + 1);
  strcpy(result, str1);
  strcat(result, str2);
  strcat(result, str3);
  return result;
}

char *concat4(const char *str1, const char *str2, const char *str3, const char *str4) {
  char *result = malloc(strlen(str1) + strlen(str2) + strlen(str3) + strlen(str4) + 1);
  strcpy(result, str1);
  strcat(result, str2);
  strcat(result, str3);
  strcat(result, str4);
  return result;
}

char *strappend(const char *str1, const char *str2) {
  char *result = malloc(strlen(str1) + strlen(str2) + 1);
  strcpy(result, str1);
  strcat(result, str2);
  assert(strlen(result) == strlen(str1) + strlen(str2));
  return result;
}

#endif