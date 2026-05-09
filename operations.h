#ifndef OPERATIONS_H
#define OPERATIONS_H

void add(const char *district_id, const char *user, const char *role);
void list(const char *district_id);
void view(const char *district_id, int report_id);
void remove_report(const char *district_id, int report_id, const char *user, const char *role);
void update_threshold(const char *district_id, int value, const char *user, const char *role);
void filter(const char *district_id, char **conditions, int num_conditions);
void remove_district(const char *district_id, const char *user, const char *role);
#endif 