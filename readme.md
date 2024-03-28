# Program Flow and Code Walkthrough

This C program is designed to perform three tasks in parallel using child processes: calculating the factorial of a number, listing the running processes, and computing the average of a set of numbers. The program takes two command-line arguments: the input file and the output file.

## Program Flow

1. The program checks if the correct number of command-line arguments is provided. If not, it prints a usage message and exits.
2. It reads the input file and stores the lines in an array of strings (`input_data`).
3. The program forks three child processes using the `fork()` system call.
4. Each child process performs one of the three tasks based on its index:
   - Child 0: Calculates the factorial of the number read from the input file.
   - Child 1: Lists the currently running processes using the `ps aux` command.
   - Child 2: Computes the average of the numbers read from the input file.
5. The child processes write their output to temporary files (`temp_factorial.txt`, `temp_process_list.txt`, and `temp_average.txt`).
6. The parent process waits for all child processes to complete using the `waitpid()` system call.
7. The `Consolidate()` function is called to combine the outputs from the temporary files into the specified output file.
8. The program cleans up the dynamically allocated memory and exits.

## Code Walkthrough

### Main Function

```c
int main(int argc, char *argv[])
{
    // ... (check command-line arguments and read input file)

    pid_t child_pids[NUM_CHILD_PROCESSES];
    const char *output_filenames[NUM_CHILD_PROCESSES] = {
        "temp_factorial.txt",
        "temp_process_list.txt",
        "temp_average.txt"
    };

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
```

The `main()` function initializes an array of child process IDs (`child_pids`) and an array of temporary output filenames (`output_filenames`). It then forks three child processes using a loop. In each iteration of the loop, the child process checks its index (`i`) and calls the corresponding task function (`factorial_task()`, `process_list_task()`, or `average_task()`). The child process ID is stored in the `child_pids` array.

After forking the child processes, the parent process waits for all child processes to complete using the `waitpid()` system call. If any child process exits abnormally, an error message is printed.

Finally, the `Consolidate()` function is called to combine the outputs from the temporary files into the specified output file.

### Consolidate Function

```c
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
```

The `Consolidate()` function takes the command-line arguments (`argv`), the array of temporary output filenames (`output_filenames`), the number of input lines (`num_input_lines`), and the array of input data (`input_data`).

It opens the specified output file (`argv[2]`) for writing. Then, it iterates over the temporary output files and appends their contents to the output file. After writing all the data, it closes the output file.

Finally, the function cleans up by freeing the dynamically allocated memory used to store the input data lines.

### Factorial Task

```c
void factorial_task(char *input_data) {
    FILE *fp = fopen("temp_factorial.txt", "w");
    if (fp == NULL) {
        fprintf(stderr, "Error opening output file\n");
        exit(1);
    }

    unsigned long int num = strtoul(input_data, NULL, 10);
    mpz_t factorial_n;
    mpz_init(factorial_n);

    mpz_fac_ui(factorial_n, num);

    char *output_str = mpz_get_str(NULL, 10, factorial_n);
    fprintf(fp, "%s\n", output_str);
    free(output_str);

    mpz_clear(factorial_n);
    fclose(fp);
}
```

The `factorial_task()` function calculates the factorial of a number read from the input file. It opens the `temp_factorial.txt` file for writing, converts the input string to an unsigned long integer (`num`), and initializes a multiple-precision integer (`mpz_t`) variable `factorial_n`.

The `mpz_fac_ui()` function from the GMP library is used to calculate the factorial of `num` and store the result in `factorial_n`. The factorial is then converted to a string using `mpz_get_str()`, and the string is written to the output file.

Finally, the function cleans up by freeing the dynamically allocated memory used for the string and closing the output file.

### Process List Task

```c
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
```

The `process_list_task()` function lists the currently running processes by executing the `ps aux` command and writing its output to the `temp_process_list.txt` file.

It opens the `temp_process_list.txt` file for writing and uses the `popen()` function to execute the `ps aux` command and capture its output. If the command execution fails, an error message is printed, and the function exits.

The function then reads the output of the `ps aux` command line by line using `fgets()` and writes each line to the output file using `fprintf()`.

Finally, the function closes the output file and the pipe to the `ps aux` command using `pclose()`.

### Average Task

```c
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
```

The `average_task()` function computes the average of a set of numbers read from the input file. It opens the `temp_average.txt` file for writing and initializes variables to store the sum and count of the numbers.

The function uses `strtok()` to tokenize the input string based on whitespace characters and converts each token to a double-precision floating-point number using `atof()`. It accumulates the sum of the numbers and keeps track of the count.

After processing all the numbers, the function calculates the average by dividing the sum by the count of numbers. The average is then written to the output file with two decimal places using `fprintf()`.

Finally, the function closes the output file.

### Helper Function

```c
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
```

The `read_input_file()` function is a helper function that reads the contents of the input file and stores each line in the `input_data` array. It opens the input file for reading, reads each line using `fgets()`, and dynamically allocates memory to store the line using `strdup()`.

After reading all the lines, the function closes the input file and returns the number of lines read.

## Conclusion

This program demonstrates the use of child processes and inter-process communication in C. It showcases how to fork child processes, execute tasks in parallel, and consolidate the outputs from multiple processes into a single file. The program also utilizes external libraries like GMP for working with large integers and the `popen()` function for executing system commands.
