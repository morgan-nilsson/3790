#include <stdio.h>
#include <ctype.h>
#include "utils/logger.h"
#include "utils/FIFO.h"
#include "utils/vector.h"

#define PASSWORD_FILE "passwords.txt"
#define SALT "$6$myS4ltStr1ng$"

struct user {
    char username[256];
    char password[256];
};

int user_cmp(const void* a, const void* b) {
    const struct user* user_a = (const struct user*)a;
    const struct user* user_b = (const struct user*)b;
    return strcmp(user_a->username, user_b->username) ||
           strcmp(user_a->password, user_b->password);
}

void print_user(const struct user* usr) {
    printf("User: %s, Password: %s\n", usr->username, usr->password);
}

// return 0 to close connection
int parse_and_handle_request(char* return_buffer, size_t return_buffer_size, const char* buffer, const Vec_t* password_vector, int* user_is_authed) {
    // Placeholder for request parsing and handling logic
    LOG_MESSAGE(LOG_LEVEL_INFO, buffer);

    // parse first word
    char command[10];
    sscanf(buffer, "%9s", command);
    for (int i = 0; command[i]; i++) {
        command[i] = tolower((unsigned char)command[i]);
    }

    if (strcmp(command, "goodbye") == 0) {
        LOG_MESSAGE(LOG_LEVEL_INFO, "Client requested to close connection.");
        snprintf(return_buffer, return_buffer_size, "+Goodbye\n");
        return 0;
    } else if (strcmp(command, "user") == 0) {

        // read username and password
        char username[256];
        char password[256];
        const int matched = sscanf(buffer + strlen(command), "%255s %255s", username, password);
        if (matched != 2) {
            LOG_MESSAGE(LOG_LEVEL_WARNING, "Malformed USER command received.");
            snprintf(return_buffer, return_buffer_size, "-Invalid command\n");
            return 1;
        }

        char* hash = crypt(password, SALT);
        if (!hash) {
            LOG_MESSAGE(LOG_LEVEL_ERROR, "crypt() failed.");
            snprintf(return_buffer, return_buffer_size, "-Server internal error\n");
            return 1;
        }

        // verify username and password
        struct user usr;
        strcpy(usr.username, username);
        strcpy(usr.password, hash);
        void* el = vector_find(password_vector, &usr, user_cmp);
        if (el != NULL) {
            *user_is_authed = 1;
            snprintf(return_buffer, return_buffer_size, "+Account valid\n");
            LOG_MESSAGE(LOG_LEVEL_INFO, return_buffer);
            return 1;
        } else {
            *user_is_authed = 0;
            snprintf(return_buffer, return_buffer_size, "-Invalid account\n");
            LOG_MESSAGE(LOG_LEVEL_WARNING, return_buffer);
            return 1;
        }

    } else if (strcmp(command, "exec") == 0) {
        if (!(*user_is_authed)) {
            LOG_MESSAGE(LOG_LEVEL_WARNING, "Unauthorized EXEC command attempt.");
            snprintf(return_buffer, return_buffer_size, "-Unauthorized. +Goodbye\n");
            return 0;
        }

        // EXEC <path of command> < command> <par1>  .  .   .   <parN>
        char exec_command[512];
        sscanf(buffer + strlen(command), "%511[^\n]", exec_command);

        char* args[128];
        int arg_count = 0;
        char* exec_ptr = exec_command;
        do {
            args[arg_count] = malloc(128);
            if (args[arg_count] == NULL) {
                LOG_MESSAGE(LOG_LEVEL_ERROR, "Memory allocation failure while parsing EXEC command.");
                snprintf(return_buffer, return_buffer_size, "-Server internal error malloc\n");
                return 1;
            }
            sscanf(exec_ptr, "%127s", args[arg_count]);
            arg_count++;
            char* next_space = strchr(exec_ptr, ' ');
            if (next_space == NULL) {
                break;
            }
            exec_ptr = next_space + 1;
        } while (1);

        args[arg_count] = NULL; // NULL-terminate the args array

        int fork_value = fork();
        if (fork_value == 0) {
            execvp(args[0], &args[1]);
            snprintf(return_buffer, return_buffer_size, "-EXEC failed. Goodbye\n");
            LOG_MESSAGE(LOG_LEVEL_ERROR, return_buffer);
            return -1;
        } else if (fork_value < 0) {
            LOG_MESSAGE(LOG_LEVEL_ERROR, "Fork failed for EXEC command.");
            snprintf(return_buffer, return_buffer_size, "-Server internal error fork. Goodbye\n");
            return 1;
        }

        wait(NULL);
        for (int i = 0; i < arg_count; i++) {
            free(args[i]);
        }

        // +EXEC SUCCESSFUL PID <XXXX>
        snprintf(return_buffer, return_buffer_size, "+EXEC SUCCESSFUL PID %d\n", fork_value);
        LOG_MESSAGE(LOG_LEVEL_INFO, return_buffer);
        return 1;

    } else {
        LOG_MESSAGE(LOG_LEVEL_WARNING, "Unknown command received.");
        snprintf(return_buffer, return_buffer_size, "-Unknown command\n");
        return 1;
    }
}

Vec_t* parse_password_file(const char* filename) {
    FILE* file = fopen(filename, "r+");
    if (file == NULL) {
        LOG_MESSAGE(LOG_LEVEL_ERROR, "Failed to open password file.");
        return NULL;
    }

    Vec_t* password_vector = malloc(sizeof(Vec_t));
    if (password_vector == NULL) {
        LOG_MESSAGE(LOG_LEVEL_ERROR, "Failed to allocate memory for password vector.");
        fclose(file);
        return NULL;
    }

    vector_init(password_vector, 10, sizeof(struct user));
    if (password_vector->error != VECTOR_SUCCESS) {
        LOG_MESSAGE(LOG_LEVEL_ERROR, "Failed to initialize password vector.");
        fclose(file);
        free(password_vector);
        return NULL;
    }

    char line[512];
    while (fgets(line, sizeof(line), file) != NULL) {
        struct user usr;
        if (sscanf(line, "%255s %255s", usr.username, usr.password) == 2) {
            vector_push_back(password_vector, &usr);
            if (password_vector->error != VECTOR_SUCCESS) {
                LOG_MESSAGE(LOG_LEVEL_ERROR, "Failed to add user to password vector.");
            }
        }
    }

    fclose(file);
    return password_vector;
}

int main(void) {
    CREATE_LOGGER("server.log", LOG_LEVEL_DEBUG);

    LOG_MESSAGE(LOG_LEVEL_INFO, "Server is running...");

    LOG_MESSAGE(LOG_LEVEL_INFO, "Loading password file...");
    Vec_t* password_vector = parse_password_file(PASSWORD_FILE);
    if (password_vector == NULL) {
        LOG_MESSAGE(LOG_LEVEL_ERROR, "Failed to load password file.");
        CLOSE_LOGGER();
        return 1;
    }
    LOG_MESSAGE(LOG_LEVEL_INFO, "Password file loaded successfully.");

    ensure_fifo_exists(FIFO_S_NAME, FIFO_READWRITE_PERMISSIONS);
    ensure_fifo_exists(FIFO_C_NAME, FIFO_READWRITE_PERMISSIONS);

    LOG_MESSAGE(LOG_LEVEL_INFO, "FIFO created or already exists.");

    FILE* fifo_r = fopen(FIFO_C_NAME, "r+");
    if (fifo_r == NULL) {
        LOG_MESSAGE(LOG_LEVEL_ERROR, "Failed to open FIFO for reading.");
        fclose(fifo_r);
        CLOSE_LOGGER();
        return 1;
    }

    FILE* fifo_w = fopen(FIFO_S_NAME, "w+");
    if (fifo_w == NULL) {
        LOG_MESSAGE(LOG_LEVEL_ERROR, "Failed to open FIFO for writing.");
        fclose(fifo_w);
        CLOSE_LOGGER();
        return 1;
    }

    LOG_MESSAGE(LOG_LEVEL_INFO, "FIFO opened for reading.");

    char buffer[1024];
    char response[1024];
    int user_is_authed = 0;
    int continue_connection = 1;
    while (continue_connection) {

        if (fgets(buffer, sizeof(buffer), fifo_r) != NULL) {
            continue_connection = parse_and_handle_request(response, sizeof(response), buffer, password_vector, &user_is_authed);
            fputs(response, fifo_w);
            fflush(fifo_w);
        }

    }

    unlink(FIFO_C_NAME);
    unlink(FIFO_S_NAME);


    CLOSE_LOGGER();
}
