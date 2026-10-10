#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include "jobs.h"
#include "job_control.h"

static pid_t shell_pgid;
static struct termios shell_tmodes;

void init_job_control(void) {
    shell_pgid = getpid();
    if (setpgid(shell_pgid, shell_pgid) < 0) {
        if (errno != EACCES) perror("setpgid");
    }
    tcsetpgrp(STDIN_FILENO, shell_pgid);
    tcgetattr(STDIN_FILENO, &shell_tmodes);
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    init_jobs();
}

int launch_job(char **argv, const char *command, int background) {
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return -1; }

    if (pid == 0) {
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);
        setpgid(0, 0);
        if (!background) tcsetpgrp(STDIN_FILENO, getpid());
        execvp(argv[0], argv);
        perror("execvp");
        _exit(127);
    }

    setpgid(pid, pid);

    if (background) {
        int job_id = add_job(pid, command, JOB_RUNNING);
        printf("[%d] %d\n", job_id, pid);
        return 0;
    }

    tcsetpgrp(STDIN_FILENO, pid);

    int status;
    do {
        if (waitpid(pid, &status, WUNTRACED) < 0) {
            if (errno != EINTR) { perror("waitpid"); break; }
        }
    } while (!WIFEXITED(status) && !WIFSIGNALED(status) && !WIFSTOPPED(status));

    tcsetpgrp(STDIN_FILENO, shell_pgid);
    tcgetattr(STDIN_FILENO, &shell_tmodes);

    if (WIFSTOPPED(status)) {
        int job_id = add_job(pid, command, JOB_STOPPED);
        printf("\n[%d]+ Stopped %s\n", job_id, command);
    }
    return 0;
}

void continue_job_background(int job_id) {
    Job *job = get_job(job_id);
    if (job == NULL) { printf("No such job: %d\n", job_id); return; }
    if (job->state != JOB_STOPPED) { printf("Job %d is not stopped\n", job_id); return; }
    if (kill(-job->pgid, SIGCONT) < 0) { perror("SIGCONT"); return; }
    job->state = JOB_RUNNING;
    printf("[%d]+ Running %s &\n", job->id, job->command);
}

void bring_job_foreground(int job_id) {
    Job *job = get_job(job_id);
    if (job == NULL) { printf("No such job: %d\n", job_id); return; }

    tcsetpgrp(STDIN_FILENO, job->pgid);
    if (job->state == JOB_STOPPED) {
        if (kill(-job->pgid, SIGCONT) < 0) perror("SIGCONT");
        job->state = JOB_RUNNING;
    }

    int status;
    waitpid(job->pgid, &status, WUNTRACED);
    tcsetpgrp(STDIN_FILENO, shell_pgid);

    if (WIFSTOPPED(status)) {
        job->state = JOB_STOPPED;
    } else {
        remove_job(job->id);
    }
}

void handle_child_status(void) {
    update_job_status();
}
