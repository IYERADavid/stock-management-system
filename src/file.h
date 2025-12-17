#ifndef FILE_H
#define FILE_H
#include "product.h"

void save_product(Product p);
void log_history_detailed(const char *action, int id, const char *details);
char* load_history();

#endif
