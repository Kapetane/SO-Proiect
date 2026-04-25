#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>

void get_report_path(char *buffer, size_t size, const char *district);
void permissions(mode_t mode, char *s);
void log_action(const char *district, const char *role, const char *user, const char *action);
void ensure_cfg(const char *district);
void create_symlink(const char *district);

#endif