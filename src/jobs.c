#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include "jobs.h"

static Job jobs[MAX_JOBS];
static int next_id = 1;

void init_jobs(void) {
    memset(jobs, 0, sizeof(jobs));
    next_id = 1;
}

int add_job(pid_t pgid, const char *command, int state) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].id == 0) {
            jobs[i].id = next_id++;
            jobs[i].pgid = pgid;
            strncpy(jobs[i].command, command, sizeof(jobs[i].command) - 1);
            jobs[i].state = state;
            return jobs[i].id;
        }
    }
    return -1;
}

void remove_job(int id) {
    if (id < 1 || id > MAX_JOBS) return;
    memset(&jobs[id - 1], 0, sizeof(Job));
}

Job *get_job(int id) {
    if (id < 1 || id > MAX_JOBS) return NULL;
    if (jobs[id - 1].id == 0) return NULL;
    return &jobs[id - 1];
}

void list_jobs(void) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].id == 0) continue;
        const char *state;
        switch (jobs[i].state) {
            case JOB_RUNNING: state = "Running"; break;
            case JOB_STOPPED: state = "Stopped"; break;
            case JOB_DONE: state = "Done"; break;
            default: state = "Unknown";
        }
        printf("[%d] %s %s\n", jobs[i].id, state, jobs[i].command);
    }
}

void update_job_status(void) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0) {
        for (int i = 0; i < MAX_JOBS; i++) {
            if (jobs[i].pgid == pid) {
                if (WIFEXITED(status) || WIFSIGNALED(status))
                    jobs[i].state = JOB_DONE;
                else if (WIFSTOPPED(status))
                    jobs[i].state = JOB_STOPPED;
                else if (WIFCONTINUED(status))
                    jobs[i].state = JOB_RUNNING;
            }
        }
    }
}
