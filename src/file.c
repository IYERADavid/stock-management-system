#include "file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void save_product(Product p) {
    FILE *fp = fopen("data/stock.dat", "ab");
    if (fp) {
        fwrite(&p, sizeof(Product), 1, fp);
        fclose(fp);
    }
}

void log_history_detailed(const char *action, int id, const char *details) {
    FILE *fp = fopen("data/history.log", "a");
    if (fp) {
        time_t now = time(NULL);
        char timestamp[30];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
        fprintf(fp, "[%s] %s - Product ID: %d - %s\n", timestamp, action, id, details);
        fclose(fp);
    }
}

char* load_history() {
    static char hist[8192] = "";
    FILE *fp = fopen("data/history.log", "r");
    if (!fp) {
        strcpy(hist, "No history available.\n");
        return hist;
    }
    
    hist[0] = '\0';
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        strcat(hist, line);
    }
    fclose(fp);
    
    if (strlen(hist) == 0) {
        strcpy(hist, "No history available.\n");
    }
    return hist;
}
