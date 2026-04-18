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

// 🔴 GLOBALE (corect)
char *CURRENT_USER = NULL;
char *CURRENT_ROLE = NULL;

void permissions(mode_t mode, char *s) {
    s[0] = (mode & S_IRUSR) ? 'r' : '-';
    s[1] = (mode & S_IWUSR) ? 'w' : '-';
    s[2] = (mode & S_IXUSR) ? 'x' : '-';

    s[3] = (mode & S_IRGRP) ? 'r' : '-';
    s[4] = (mode & S_IWGRP) ? 'w' : '-';
    s[5] = (mode & S_IXGRP) ? 'x' : '-';

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
    mkdir(district_id, 0750);
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

    r.latitude = 46.0;
    r.longitude = 21.0;
    strcpy(r.issue_category, "road");
    r.severity_level = 2;
    r.timestamp = time(NULL);
    strcpy(r.description_text, "Default description");

    printf("Adding report for %s in %s...\n", r.inspector_name, district_id);

    write(fd, &r, sizeof(Report));
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
        printf("Descriere: %s\n", r.description_text);
    }

    if (cnt == 0)
        printf("Nu s-au gasit rapoarte.\n");

    close(fd);

    printf("\nInformatii fisier:\n");
    printf("Dimensiune: %ld bytes\n", st.st_size);
    printf("Permisiuni: %s\n", perm);
    printf("Ultima modificare: %s", ctime(&st.st_mtime));
}

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

    return 0;
}