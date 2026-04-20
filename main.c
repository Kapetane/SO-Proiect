#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

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
} Args;

char *CURRENT_USER = NULL;
char *CURRENT_ROLE = NULL;

void permissions(mode_t mode, char *s) {
    //user
    s[0] = (mode & S_IRUSR) ? 'r' : '-';
    s[1] = (mode & S_IWUSR) ? 'w' : '-';
    s[2] = (mode & S_IXUSR) ? 'x' : '-';

    //group
    s[3] = (mode & S_IRGRP) ? 'r' : '-';
    s[4] = (mode & S_IWGRP) ? 'w' : '-';
    s[5] = (mode & S_IXGRP) ? 'x' : '-';

    //others
    s[6] = (mode & S_IROTH) ? 'r' : '-';
    s[7] = (mode & S_IWOTH) ? 'w' : '-';
    s[8] = (mode & S_IXOTH) ? 'x' : '-';

    s[9] = '\0';
}

void log_action(const char *district, const char *role, const char *user, const char *action) {
    char path[256];
    snprintf(path, sizeof(path), "%s/logged_district", district);

    int fd = open(path, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd < 0) {
        perror("log");
        return;
    }

    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%ld | %s | %s | %s\n",
             time(NULL), role, user, action);

    write(fd, buffer, strlen(buffer));
    close(fd);

    chmod(path, 0644);
}

void ensure_cfg(const char *district) {
    char path[256];
    snprintf(path, sizeof(path), "%s/district.cfg", district);

    if (access(path, F_OK) == -1) {
        int fd = open(path, O_WRONLY | O_CREAT, 0640);
        if (fd < 0) {
            perror("cfg");
            return;
        }

        write(fd, "threshold=2\n", 12);
        close(fd);
        chmod(path, 0640);
    }
}

void create_symlink(const char *district) {
    char target[256], linkname[256];

    snprintf(target, sizeof(target), "%s/reports.dat", district);
    snprintf(linkname, sizeof(linkname), "active_reports-%s", district);

    unlink(linkname);

    if (symlink(target, linkname) < 0) {
        perror("symlink");
    }
}

void add(const char *district_id) {
    if (mkdir(district_id, 0750) < 0) {
    }

    chmod(district_id, 0750);

    ensure_cfg(district_id);

    char path[256];
    snprintf(path, sizeof(path), "%s/reports.dat", district_id);

    int fd = open(path, O_WRONLY | O_APPEND | O_CREAT, 0664);
    if (fd < 0) {
        perror("add");
        return;
    }

    chmod(path, 0664);

    Report r = {0};

    r.report_id = rand() % 100000;

    strncpy(r.inspector_name, CURRENT_USER, sizeof(r.inspector_name) - 1);
    r.inspector_name[sizeof(r.inspector_name) - 1] = '\0';

    printf("Latitude: ");
    scanf("%f", &r.latitude);

    printf("Longitude: ");
    scanf("%f", &r.longitude);

    for (;;) {
        printf("Category (road/lightning/flooding/other): ");
        scanf("%31s", r.issue_category);

        if (!strcmp(r.issue_category, "road") ||
            !strcmp(r.issue_category, "lightning") ||
            !strcmp(r.issue_category, "flooding") ||
            !strcmp(r.issue_category, "other")) {
            break;
        }

        printf("Invalid category! Try again :)\n");
    }

    for (;;) {
        printf("Severity (1/2/3): ");
        scanf("%d", &r.severity_level);

        if (r.severity_level >= 1 && r.severity_level <= 3)
            break;

        printf("Invalid level! Try again :)\n");
    }

    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    printf("Description: ");
    fgets(r.description_text, sizeof(r.description_text), stdin);
    r.description_text[strcspn(r.description_text, "\n")] = '\0';

    r.timestamp = time(NULL);

    if (write(fd, &r, sizeof(Report)) != sizeof(Report)) {
        perror("write");
    }

    close(fd);

    create_symlink(district_id);
    log_action(district_id, CURRENT_ROLE, CURRENT_USER, "ADD");

}

void list(const char *district_id) {
    char path[256];
    snprintf(path, sizeof(path), "%s/reports.dat", district_id);

    struct stat st;
    if (stat(path, &st) < 0) {
        perror("stat");
        return;
    }

    char perm[10];
    permissions(st.st_mode, perm);

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return;
    }

    printf("Rapoarte in %s:\n", district_id);
    Report r;
    int cnt = 0;

    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        printf("Numar Raport: %d\n", ++cnt);
        printf("ID: %d\n", r.report_id);
        printf("Inspector: %s\n", r.inspector_name);
        printf("Coordonate: (%f, %f)\n", r.latitude, r.longitude);
        printf("Categorie: %s\n", r.issue_category);
        printf("Severitate: %d\n", r.severity_level);
        printf("Timestamp: %ld\n", r.timestamp);
        printf("Descriere: %s\n\n", r.description_text);
    }

    if (cnt == 0)
        printf("Nu s-au gasit rapoarte.\n");

    close(fd);

    printf("\nInformatii fisier:\n");
    printf("Dimensiune: %ld bytes\n", st.st_size);
    printf("Permisiuni: %s\n", perm);
    printf("Ultima modificare: %s", ctime(&st.st_mtime));
}

void view(const char *district_id, int report_id) {
    char path[256];
    snprintf(path, sizeof(path), "%s/reports.dat", district_id);

    int fd = open(path, O_RDONLY);
    if(fd < 0) {
        perror("open");
        return;
    }

    Report r;
    int gasit =0;

    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        if (r.report_id == report_id) {
            printf("Raport Gasit:\n");
            printf("ID: %d\n", r.report_id);
            printf("Inspector: %s\n", r.inspector_name);
            printf("Coordonate: (%f, %f)\n", r.latitude, r.longitude);
            printf("Categorie: %s\n", r.issue_category);
            printf("Severitate: %d\n", r.severity_level);
            printf("Timestamp: %ld\n", r.timestamp);
            printf("Descriere: %s\n", r.description_text);
            gasit = 1;
            break;
        }
    }

    if (gasit == 0) {
        printf("Report cu ID %d nu a fost gasit.\n", report_id);
    }

    close(fd);
}

void remove_report(const char *district_id, int report_id) {
    if(strcmp(CURRENT_ROLE, "manager") != 0) {
        printf("Manager role only!\n");
        return;
    }

    char path[256];
    snprintf(path, sizeof(path), "%s/reports.dat", district_id);

    int fd = open(path, O_RDWR);
    if(fd < 0) {
        perror("open");
        return;
    }

    Report r;
    off_t pos = 0;
    off_t found_pos = -1;

    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        if (r.report_id == report_id) {
            found_pos = pos;
            break;
        }
        pos += sizeof(Report);
    }

    if (found_pos == -1) {
        printf("Report cu ID %d nu exista.\n", report_id);
        close(fd);
        return;
    }

    off_t read_pos = found_pos + sizeof(Report);
    off_t write_pos = found_pos;

    Report temp;

    while (1) {
        lseek(fd, read_pos, SEEK_SET);
        ssize_t bytes = read(fd, &temp, sizeof(Report));
        if (bytes != sizeof(Report))
            break;
        lseek(fd, write_pos, SEEK_SET);
        write(fd, &temp, sizeof(Report));
        read_pos += sizeof(Report);
        write_pos += sizeof(Report);
    }

    struct stat st;
    fstat(fd, &st);

    if (ftruncate(fd, st.st_size - sizeof(Report)) < 0) {
        perror("ftruncate");
    }

    close(fd);

    printf("Report %d a fost sters.\n", report_id);

    log_action(district_id, CURRENT_ROLE, CURRENT_USER, "REMOVE");
}

void update_threshold(const char *district_id, int value) {
    if(strcmp(CURRENT_ROLE, "manager") != 0) {
        printf("Manager role only!");
        return;
    }

    char path[256];
    snprintf(path, sizeof(path), "%s/district.cfg", district_id);

    struct stat st;
    if(stat(path, &st) < 0){
        perror("stat");
        return;
    }

    if((st.st_mode & 0777) != 0640) {
        printf("permission bits != 640");
        return;
    }

    int fd = open(path, O_WRONLY | O_TRUNC);
    if(fd<0) {
        perror("open");
        return;
    }

    char buf[128];
    snprintf(buf, sizeof(buf), "threshold=%d\n", value);

    write(fd, buf, strlen(buf));
    close(fd);

    printf("New Threshold: %d\n", value);

    log_action(district_id, CURRENT_ROLE, CURRENT_USER, "UPDATE_THRESHOLD");

}

//FILTER ☠

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

    // --- SEVERITY ---
    if (strcmp(field, "severity") == 0) {
        int val = atoi(value);

        if (strcmp(op, "==") == 0) return r->severity_level == val;
        if (strcmp(op, "!=") == 0) return r->severity_level != val;
        if (strcmp(op, ">") == 0) return r->severity_level > val;
        if (strcmp(op, ">=") == 0) return r->severity_level >= val;
        if (strcmp(op, "<") == 0) return r->severity_level < val;
        if (strcmp(op, "<=") == 0) return r->severity_level <= val;
    }

    // --- CATEGORY ---
    if (strcmp(field, "category") == 0) {
        if (strcmp(op, "==") == 0) return strcmp(r->issue_category, value) == 0;
        if (strcmp(op, "!=") == 0) return strcmp(r->issue_category, value) != 0;
    }

    // --- INSPECTOR ---
    if (strcmp(field, "inspector") == 0) {
        if (strcmp(op, "==") == 0) return strcmp(r->inspector_name, value) == 0;
        if (strcmp(op, "!=") == 0) return strcmp(r->inspector_name, value) != 0;
    }

    // --- TIMESTAMP ---
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
void filter(const char *district_id, const char *condition) {
    char path[256];
    snprintf(path, sizeof(path), "%s/reports.dat", district_id);

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return;
    }

    char field[32], op[4], value[64];

    if (!parse_condition(condition, field, op, value)) {
        printf("Conditie invalida!\n");
        close(fd);
        return;
    }

    Report r;
    int found = 0;

    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        if (match_condition(&r, field, op, value)) {
            printf("\n=== MATCH ===\n");
            printf("ID: %d\n", r.report_id);
            printf("Inspector: %s\n", r.inspector_name);
            printf("Coordonate: (%f, %f)\n", r.latitude, r.longitude);
            printf("Categorie: %s\n", r.issue_category);
            printf("Severitate: %d\n", r.severity_level);
            printf("Timestamp: %ld\n", r.timestamp);
            printf("Descriere: %s\n", r.description_text);

            found = 1;
        }
    }

    if (!found)
        printf("Nu s-au gasit rezultate.\n");

    close(fd);
}
//END FILTER ☠

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
        }
        else if(!strcmp(argv[i], "--view") && (i + 2 < argc)) {
            args.operation = "view";
            args.district_id = argv[++i];
            args.extra = argv[++i]; //report_id
        }
        else if(!strcmp(argv[i], "--remove_report") && (i + 2 < argc)) {
            args.operation = "remove_report";
            args.district_id = argv[++i];
            args.extra = argv[++i]; //report_id
        }
        else if(!strcmp(argv[i], "--update_threshold") && (i + 2 < argc)) {
            args.operation = "update_threshold";
            args.district_id = argv[++i];
            args.extra = argv[++i]; //report_id
        }
        else if (!strcmp(argv[i], "--filter") && i + 2 < argc) {
            args.operation = "filter";
            args.district_id = argv[++i];
            args.extra = argv[++i];
        }
    }

    if (!args.user_role || !args.user_name || !args.operation || !args.district_id) {
        printf("Argumente invalide\n");
        return 1;
    }

    CURRENT_USER = args.user_name;
    CURRENT_ROLE = args.user_role;

    if (!strcmp(args.operation, "add"))
        add(args.district_id);

    if (!strcmp(args.operation, "list"))
        list(args.district_id);

    if(!strcmp(args.operation, "view"))
        view(args.district_id, atoi(args.extra));

    if (!strcmp(args.operation, "remove_report"))
        remove_report(args.district_id, atoi(args.extra));

    if (!strcmp(args.operation, "update_threshold"))
        update_threshold(args.district_id, atoi(args.extra));

    if (!strcmp(args.operation, "filter"))
        filter(args.district_id, args.extra);
    return 0;
}