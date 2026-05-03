#include "operations.h"
#include "types.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

void add(const char *district_id, const char *user, const char *role) {
    if (mkdir(district_id, 0750) < 0) { }
    chmod(district_id, 0750);
    ensure_cfg(district_id);

    char path[256];
    get_report_path(path, sizeof(path), district_id);

    int fd = open(path, O_WRONLY | O_APPEND | O_CREAT, 0664);
    if (fd < 0) {
        perror("add");
         return; 
    }
    chmod(path, 0664);

    Report r = {0};
    r.report_id = rand() % 1000000;
    strncpy(r.inspector_name, user, sizeof(r.inspector_name) - 1);

    printf("Latitude X: "); scanf("%f", &r.latitude);
    printf("Longitude Y: "); scanf("%f", &r.longitude);

    for (;;) {
        printf("Category (road/lightning/flooding/other): ");
        scanf("%31s", r.issue_category);
        if (!strcmp(r.issue_category, "road") || !strcmp(r.issue_category, "lightning") ||
            !strcmp(r.issue_category, "flooding") || !strcmp(r.issue_category, "other")) break;
        printf("Invalid category! Try again :)\n");
    }

    for (;;) {
        printf("Severity (1/2/3): ");
        scanf("%d", &r.severity_level);
        if (r.severity_level >= 1 && r.severity_level <= 3) break;
        printf("Invalid level! Try again :)\n");
    }

    int c; 
    while ((c = getchar()) != '\n' && c != EOF);
    printf("Description: ");
    fgets(r.description_text, sizeof(r.description_text), stdin);
    r.description_text[strcspn(r.description_text, "\n")] = '\0';

    r.timestamp = time(NULL);
    if (write(fd, &r, sizeof(Report)) != sizeof(Report)) perror("write");
    close(fd);

    create_symlink(district_id);
    log_action(district_id, role, user, "ADD");
}

void list(const char *district_id){
    char path[256];
    get_report_path(path, sizeof(path), district_id);

    struct stat st;
    if (stat(path, &st) < 0) { perror("stat"); return; }

    char perm[10];
    permissions(st.st_mode, perm);

    int fd = open(path, O_RDONLY);
    if (fd < 0) { perror("open"); return; }

    printf("Rapoarte in %s:\n", district_id);
    Report r; int cnt = 0;

    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        printf("Numar Raport: %d\nID: %d\nInspector: %s\nCoordonate: (%f, %f)\nCategorie: %s\nSeveritate: %d\nTimestamp: %ld\nDescriere: %s\n\n",
               ++cnt, r.report_id, r.inspector_name, r.latitude, r.longitude, r.issue_category, r.severity_level, r.timestamp, r.description_text);
    }

    if (cnt == 0) printf("Nu s-au gasit rapoarte.\n");
    close(fd);

    printf("\nInformatii fisier:\nDimensiune: %ld bytes\nPermisiuni: %s\nUltima modificare: %s", st.st_size, perm, ctime(&st.st_mtime));
}

void view(const char *district_id, int report_id) {
    char path[256];
    get_report_path(path, sizeof(path), district_id);

    int fd = open(path, O_RDONLY);
    if(fd < 0) { perror("open"); return; }

    Report r; int gasit = 0;
    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        if (r.report_id == report_id) {
            printf(
                "Raport Gasit:\nID: %d\nInspector: %s\nCoordonate: (%f, %f)\nCategorie: %s\nSeveritate: %d\nTimestamp: %ld\nDescriere: %s\n",
                   r.report_id, r.inspector_name, r.latitude,
                    r.longitude, r.issue_category, r.severity_level,
                     r.timestamp, r.description_text
                    );
            gasit = 1; 
            break;
        }
    }
    if (!gasit) printf("Report cu ID %d nu a fost gasit.\n", report_id);
    close(fd);
}

void remove_report(const char *district_id, int report_id, const char *user, const char *role) {
    if(strcmp(role, "manager") != 0) { 
            printf("Manager role only!\n");
            return; 
        }

    char path[256];
    get_report_path(path, sizeof(path), district_id);

    int fd = open(path, O_RDWR);
    if(fd < 0) {
        perror("open");
        return; 
        }

    Report r; off_t pos = 0, found_pos = -1;
    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        if (r.report_id == report_id) {   
            found_pos = pos;
            break; 
            }
        pos += sizeof(Report);
    }

    if (found_pos == -1) {
        printf("Report cu ID %d nu exista.\n", report_id);
        close(fd); return;
    }

    off_t read_pos = found_pos + sizeof(Report), write_pos = found_pos;
    Report temp;
    while (1) {
        lseek(fd, read_pos, SEEK_SET);
        if (read(fd, &temp, sizeof(Report)) != sizeof(Report)) break;
        lseek(fd, write_pos, SEEK_SET);
        write(fd, &temp, sizeof(Report));
        read_pos += sizeof(Report); write_pos += sizeof(Report);
    }

    struct stat st; fstat(fd, &st);
    if (ftruncate(fd, st.st_size - sizeof(Report)) < 0) perror("ftruncate");
    close(fd);

    printf("Report %d a fost sters.\n", report_id);
    log_action(district_id, role, user, "REMOVE");
}

void update_threshold(const char *district_id, int value, const char *user, const char *role) {
    if(strcmp(role, "manager") != 0) { printf("Manager role only!\n"); return; }

    char path[256];
    snprintf(path, sizeof(path), "%s/district.cfg", district_id);

    struct stat st;
    if(stat(path, &st) < 0) {
        perror("stat"); 
        return; 
    }
    if((st.st_mode & 0777) != 0640) { 
        printf("permission bits != 640\n"); 
        return; 
    }

    int fd = open(path, O_WRONLY | O_TRUNC);
    if(fd < 0) { 
        perror("open"); 
        return; 
    }

    char buf[128];
    snprintf(buf, sizeof(buf), "threshold=%d\n", value);
    write(fd, buf, strlen(buf));
    close(fd);

    printf("New Threshold: %d\n", value);
    log_action(district_id, role, user, "UPDATE_THRESHOLD");
}

//FILTER 💀

int parse_condition(const char *input, char *field, char *op, char *value) {
    char temp[128];
    strncpy(temp, input, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';

    char *token = strtok(temp, ":");
    if (!token) return 0;
    strcpy(field, token);

    token = strtok(NULL, ":");
    if (!token) return 0;
    strcpy(op, token);

    token = strtok(NULL, ":");
    if (!token) return 0;
    strcpy(value, token);

    return 1;
}

int match_condition(Report *r, const char *field, const char *op, const char *value) {
    if (strcmp(field, "severity") == 0) {
        int val = atoi(value);
        if (strcmp(op, "==") == 0) return r->severity_level == val;
        if (strcmp(op, "!=") == 0) return r->severity_level != val;
        if (strcmp(op, ">") == 0) return r->severity_level > val;
        if (strcmp(op, ">=") == 0) return r->severity_level >= val;
        if (strcmp(op, "<") == 0) return r->severity_level < val;
        if (strcmp(op, "<=") == 0) return r->severity_level <= val;
    }
    if (strcmp(field, "category") == 0) {
        if (strcmp(op, "==") == 0) return strcmp(r->issue_category, value) == 0;
        if (strcmp(op, "!=") == 0) return strcmp(r->issue_category, value) != 0;
    }
    if (strcmp(field, "inspector") == 0) {
        if (strcmp(op, "==") == 0) return strcmp(r->inspector_name, value) == 0;
        if (strcmp(op, "!=") == 0) return strcmp(r->inspector_name, value) != 0;
    }
    if (strcmp(field, "timestamp") == 0) {
        long val = atol(value);
        if (strcmp(op, "==") == 0) return r->timestamp == val;
        if (strcmp(op, "!=") == 0) return r->timestamp != val;
        if (strcmp(op, ">") == 0) return r->timestamp > val;
        if (strcmp(op, ">=") == 0) return r->timestamp >= val;
        if (strcmp(op, "<") == 0) return r->timestamp < val;
        if (strcmp(op, "<=") == 0) return r->timestamp <= val;
    }
    return 0;
}

void filter(const char *district_id, char **conditions, int num_conditions) {
    char path[256];
    get_report_path(path, sizeof(path), district_id);

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return;
    }

    Report r;
    int found = 0;

    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        int match_all = 1;
        for (int i = 0; i < num_conditions; i++) {
            char field[32], op[4], value[64];
            if (!parse_condition(conditions[i], field, op, value)) {
                printf("Conditie invalida: %s\n", conditions[i]);
                match_all = 0;
                break;
            }
            if (!match_condition(&r, field, op, value)) {
                match_all = 0;
                break;
            }
        }

        if (match_all) {
            printf("\n=== MATCH ===\nID: %d\nInspector: %s\nCoordonate: (%f, %f)\nCategorie: %s\nSeveritate: %d\nTimestamp: %ld\nDescriere: %s\n",
                   r.report_id, r.inspector_name, r.latitude, r.longitude, r.issue_category, r.severity_level, r.timestamp, r.description_text);
            found = 1;
        }
    }

    if (!found) printf("Nu s-au gasit rezultate.\n");
    close(fd);
}
//END FILTER 💀
void remove_district(const char *district_id, const char *user, const char *role) {
    if(strcmp(role, "manager") != 0) { 
        printf("Manager role only!\n");
        return; 
    }

    char linkname[256];
    snprintf(linkname, sizeof(linkname), "active_reports-%s", district_id);
    
    if(unlink(linkname)<0) {
        perror("unlink la symlink");
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    } 
    else if (pid == 0) {
        execlp("rm", "rm", "-rf", district_id, NULL);
        exit(1); 
    } 
    else {
        int status;
        waitpid(pid, &status, 0);
    }
}