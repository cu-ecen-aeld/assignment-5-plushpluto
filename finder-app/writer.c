#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    // Open syslog
    openlog("writer", LOG_PID, LOG_USER);
    
    if (argc != 3) {
        syslog(LOG_ERR, "Invalid number of arguments: %d", argc);
        fprintf(stderr, "Usage: %s <file> <string>\n", argv[0]);
        closelog();
        return 1;
    }
    
    const char *writefile = argv[1];
    const char *writestr = argv[2];
    
    syslog(LOG_DEBUG, "Writing %s to %s", writestr, writefile);
    
    FILE *file = fopen(writefile, "w");
    if (file == NULL) {
        syslog(LOG_ERR, "Failed to open file %s: %s", writefile, strerror(errno));
        fprintf(stderr, "Error: Could not create file %s\n", writefile);
        closelog();
        return 1;
    }
    
    if (fprintf(file, "%s", writestr) < 0) {
        syslog(LOG_ERR, "Failed to write to file %s: %s", writefile, strerror(errno));
        fprintf(stderr, "Error: Could not write to file %s\n", writefile);
        fclose(file);
        closelog();
        return 1;
    }
    
    fclose(file);
    closelog();
    return 0;
}