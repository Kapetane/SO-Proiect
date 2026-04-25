#ifndef TYPES_H
#define TYPES_H

#include <time.h>

typedef struct {
    int report_id;
    char inspector_name[40];
    float latitude;
    float longitude;
    char issue_category[32];
    int severity_level;
    time_t timestamp;
    char description_text[112];
} Report;

typedef struct {
    char *user_role;
    char *user_name;
    char *operation;
    char *district_id;
    char *extra;
    char **conditions;
    int num_conditions;
} Args;

#endif 