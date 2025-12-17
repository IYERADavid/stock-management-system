#ifndef PRODUCT_H
#define PRODUCT_H

typedef struct {
    int id;
    char name[50];
    char category[30];
    int quantity;
    float price;
    int totalSold;
} Product;

Product* find_product(int id);
int get_product_count();
float calculate_stock_value();

#endif
