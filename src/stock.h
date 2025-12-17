#ifndef STOCK_H
#define STOCK_H

int update_stock(int id, int qty, char *product_name);
float sell_product(int id, int qty, int discount, char *product_name, float *original_price);
int remove_product(int id, char *product_name);
int check_stock_level(int id);

#endif
