#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "types.h"
#include "operations.h"

int main(int argc, char **argv) {
    srand(time(NULL));
    Args args = {0};

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--role") && i + 1 < argc)
            args.user_role = argv[++i];
        else if (!strcmp(argv[i], "--user") && i + 1 < argc)
            args.user_name = argv[++i];
        else if (!strcmp(argv[i], "--add") && i + 1 < argc) {
            args.operation = "add";
            args.district_id = argv[++i];
        } else if (!strcmp(argv[i], "--list") && i + 1 < argc) {
            args.operation = "list";
            args.district_id = argv[++i];
        } else if(!strcmp(argv[i], "--view") && (i + 2 < argc)) {
            args.operation = "view";
            args.district_id = argv[++i];
            args.extra = argv[++i];
        } else if(!strcmp(argv[i], "--remove_report") && (i + 2 < argc)) {
            args.operation = "remove_report";
            args.district_id = argv[++i];
            args.extra = argv[++i];
        } else if(!strcmp(argv[i], "--update_threshold") && (i + 2 < argc)) {
            args.operation = "update_threshold";
            args.district_id = argv[++i];
            args.extra = argv[++i];
        } else if (!strcmp(argv[i], "--filter") && i + 2 < argc) {
            args.operation = "filter";
            args.district_id = argv[++i];
            args.conditions = &argv[i+1];
            args.num_conditions = 0;
            while(i + 1 < argc && strncmp(argv[i+1], "--", 2) != 0) {
                args.num_conditions++;
                i++;
            }
        }else if (!strcmp(argv[i], "--remove_district") && i + 1 < argc) {
            args.operation = "remove_district";
            args.district_id = argv[++i];
        }
    }

    if (!args.user_role || !args.user_name || !args.operation || !args.district_id) {
        printf("Argumente invalide\n");
        return 1;
    }

    if (!strcmp(args.operation, "add"))
        add(args.district_id, args.user_name, args.user_role);
    else if (!strcmp(args.operation, "list"))
        list(args.district_id);
    else if(!strcmp(args.operation, "view"))
        view(args.district_id, atoi(args.extra));
    else if (!strcmp(args.operation, "remove_report"))
        remove_report(args.district_id, atoi(args.extra), args.user_name, args.user_role);
    else if (!strcmp(args.operation, "update_threshold"))
        update_threshold(args.district_id, atoi(args.extra), args.user_name, args.user_role);
    else if (!strcmp(args.operation, "filter"))
        filter(args.district_id, args.conditions, args.num_conditions);
    else if (!strcmp(args.operation, "remove_district"))
        remove_district(args.district_id, args.user_name, args.user_role);
    return 0;
}