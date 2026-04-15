#include <stdio.h>
#include <time.h>
#include <string.h>

#define NAME_SIZE 20
#define FIELD 50

typedef struct {
    int report_id;
    char inspector_name[NAME_SIZE];
    float latitude;
    float longitude;
    char issue_category[FIELD];
    int severity_level;
    time_t timestamp;
    char description_text[FIELD];
}Report;

typedef struct {
    char *user_role;
    char *user_name;
    char *operation;
    char *district_id;
    char *extra;
}Args;

int main(int argc, char **argv) {

    Args args={0};
    for (int i = 1; i < argc; i++) {

    if (!strcmp(argv[i], "--role") && i + 1 < argc) {
    args.user_role = argv[++i];

    } else if (!strcmp(argv[i], "--user") && i + 1 < argc) {
    args.user_name = argv[++i];

    } else if (!strcmp(argv[i], "--add") && i + 1 < argc) {
    args.operation = "add";
    args.district_id = argv[++i];

    } else if (!strcmp(argv[i], "--list") && i + 1 < argc) {
    args.operation = "list";
    args.district_id = argv[++i];

    } else if (!strcmp(argv[i], "--view") && i + 2 < argc) {
    args.operation = "view";
    args.district_id = argv[++i];
    args.extra = argv[++i];

    } else if (!strcmp(argv[i], "--remove_report") && i + 2 < argc) {
    args.operation = "remove_report";
    args.district_id = argv[++i];
    args.extra = argv[++i];

    } else if (!strcmp(argv[i], "--update_threshold") && i + 2 < argc) {
    args.operation = "update_threshold";
    args.district_id = argv[++i];
    args.extra = argv[++i];

    } else if (!strcmp(argv[i], "--filter") && i + 2 < argc) {
    args.operation = "filter";
    args.district_id = argv[++i];
    args.extra = argv[++i];
    }
    }

    if (!args.user_role || !args.user_name || !args.operation || !args.district_id) {
    printf("Argumente invalide\n");
    return 1;}

    if(!strcmp(args.operation, "add")) {
        printf("%s %s %s %s\n",
            args.operation,
            args.district_id,
            args.user_name,
            args.user_role);
    }

    return 0;
}