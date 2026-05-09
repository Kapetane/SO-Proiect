#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

volatile sig_atomic_t tip_semnal = 0;

void handle(int sig) {
    tip_semnal = sig;
}

int main() {
    int fd = open(".monitor_pid", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(fd == -1) {
        perror("Eroare la deschidere!");
        return 1;
    }

    char pid_str[100];
    int len = sprintf(pid_str, "%d", getpid());
    write(fd, pid_str, len);
    close(fd);

    printf("Merge PID: %d\n", getpid());

    struct sigaction sa;
    sa.sa_handler = handle;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);  //CTRL+C
    sigaction(SIGUSR1, &sa, NULL); //Rapoarte noi

    while (1) {
        pause();
        if (tip_semnal == SIGUSR1) {
            write(1, "[MONITOR] Notificare: Un nou raport a fost adăugat!\n", 53);
            tip_semnal = 0; 
        } 
        else if (tip_semnal == SIGINT) {
            write(1, "\n[MONITOR] SIGINT primit. Incheiere program.\n", 45);
            break;
        }
    }
    if (unlink(".monitor_pid") == 0) {
        write(1, "[MONITOR] Fisierul .monitor_pid a fost sters.\n", 46);
    }

    return 0;
}