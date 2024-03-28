#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <gmp.h>

#define MAX_LINE_LENGTH 256
#define NUM_CHILD_PROCESSES 3

void factorial_task(char *input_data);
void fact(mpz_t r, unsigned int n);
int Consolidate(char *argv[], const char *output_filenames[3], int num_input_lines, char *input_data[3]);
void process_list_task();
void average_task(char *input_data);
int read_input_file(char *filename, char *input_data[]);

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    char *input_data[3] = {NULL};
    int num_input_lines = read_input_file(argv[1], input_data);
    if (num_input_lines < 0)
    {
        fprintf(stderr, "Error reading input file\n");
        return 1;
    }

    pid_t child_pids[NUM_CHILD_PROCESSES];
    const char *output_filenames[NUM_CHILD_PROCESSES] = {
        "temp_factorial.txt",
        "temp_process_list.txt",
        "temp_average.txt"};
    printf("files are: %s %s %s\n", output_filenames[0], output_filenames[1], output_filenames[2]);

    for (int i = 0; i < NUM_CHILD_PROCESSES; i++)
    {
        pid_t pid = fork();
        if (pid < 0)
        {
            fprintf(stderr, "Error forking child process\n");
            return 1;
        }
        else if (pid == 0)
        {
            // Child process
            switch (i)
            {
            case 0:
                factorial_task(input_data[0]);
                break;
            case 1:
                process_list_task();
                break;
            case 2:
                average_task(input_data[1]);
                break;
            }

            exit(0);
        }

        child_pids[i] = pid;
    }

    // Parent process
    int status;
    for (int i = 0; i < NUM_CHILD_PROCESSES; i++)
    {
        waitpid(child_pids[i], &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
        {
            fprintf(stderr, "Child process %d exited abnormally\n", child_pids[i]);
        }
    }

    // Consolidate output files
    int retVal = Consolidate(argv, output_filenames, num_input_lines, input_data);
    if (retVal == 1)
        return retVal;

    return 0;
}

int Consolidate(char *argv[], const char *output_filenames[3], int num_input_lines, char *input_data[3])
{
    int retFlag = 1;
    FILE *fp = fopen(argv[2], "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Error opening output file\n");
        return 1;
    }

    for (int i = 0; i < NUM_CHILD_PROCESSES; i++)
    {
        FILE *output_file = fopen(output_filenames[i], "r");
        if (output_file == NULL)
        {
            fprintf(stderr, "Error opening output file %s\n", output_filenames[i]);
            continue;
        }

        char line[MAX_LINE_LENGTH];
        while (fgets(line, MAX_LINE_LENGTH, output_file) != NULL)
        {
            fprintf(fp, "%s", line);
        }

        fclose(output_file);
    }

    fclose(fp);

    // Clean up
    for (int i = 0; i < num_input_lines; i++)
    {
        free(input_data[i]);
    }
    retFlag = 0;
    return retFlag;
}

void factorial_task(char *input_data) {
    FILE *fp = fopen("temp_factorial.txt", "w");
    if (fp == NULL) {
        fprintf(stderr, "Error opening output file\n");
        exit(1);
    }

    unsigned long int num = strtoul(input_data, NULL, 10);
    mpz_t factorial_n;
    mpz_init(factorial_n);

    //https://github.com/crazyBaboon/GMP-examples/blob/master/factorial.c
    mpz_fac_ui(factorial_n, num);

    char *output_str = mpz_get_str(NULL, 10, factorial_n);
    fprintf(fp, "%s\n", output_str);
    free(output_str);

    mpz_clear(factorial_n);
    fclose(fp);
}

//copied from: https://stackoverflow.com/a/1388664/15986556
// void fact(mpz_t r, unsigned int n) {
//     unsigned int i;
//     mpz_t temp;
//     mpz_init(temp);
//     mpz_set_ui(r, 1);
//     for (i = 1; i <= n; i++) {
//         mpz_set_ui(temp, i);
//         mpz_mul(r, r, temp);
//     }
//     mpz_clear(temp);
// }


void process_list_task()
{
    FILE *fp = fopen("temp_process_list.txt", "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Error opening output file\n");
        exit(1);
    }

    FILE *proc = popen("ps aux", "r");
    if (proc == NULL)
    {
        fprintf(stderr, "Error executing command\n");
        fclose(fp);
        exit(1);
    }

    char buffer[MAX_LINE_LENGTH];
    while (fgets(buffer, MAX_LINE_LENGTH, proc) != NULL)
    {
        fprintf(fp, "%s", buffer);
    }

    pclose(proc);
    fclose(fp);
}

void average_task(char *input_data)
{
    FILE *fp = fopen("temp_average.txt", "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Error opening output file\n");
        exit(1);
    }

    int num_values = 0;
    double sum = 0.0;
    char *token = strtok(input_data, " ");
    while (token != NULL)
    {
        double value = atof(token);
        sum += value;
        num_values++;
        token = strtok(NULL, " ");
    }

    double average = sum / num_values;
    fprintf(fp, "%.2lf\n", average);
    fclose(fp);
}

int read_input_file(char *filename, char *input_data[])
{
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Error opening input file\n");
        return -1;
    }

    char line[MAX_LINE_LENGTH];
    int num_lines = 0;

    while (fgets(line, MAX_LINE_LENGTH, fp) != NULL)
    {
        input_data[num_lines] = strdup(line);
        num_lines++;
    }

    fclose(fp);
    return num_lines;
}