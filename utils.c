#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

void get_report_path(char *buffer, size_t size, const char *district) {
    snprintf(buffer, size, "%s/reports.dat", district);
}

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
    snprintf(buffer, sizeof(buffer), "%ld | %s | %s | %s\n", time(NULL), role, user, action);
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