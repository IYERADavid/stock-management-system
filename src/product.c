#include "product.h"
#include <stdio.h>
#include <stdlib.h>

Product* find_product(int id) {
    FILE *fp = fopen("data/stock.dat", "rb");
    if (!fp) return NULL;
    
    static Product p;
    while (fread(&p, sizeof(Product), 1, fp)) {
        if (p.id == id) {
            fclose(fp);
            return &p;
        }
    }
    fclose(fp);
    return NULL;
}

int get_product_count() {
    FILE *fp = fopen("data/stock.dat", "rb");
    if (!fp) return 0;
    
    fseek(fp, 0, SEEK_END);
    int count = ftell(fp) / sizeof(Product);
    fclose(fp);
    return count;
}

float calculate_stock_value() {
    FILE *fp = fopen("data/stock.dat", "rb");
    if (!fp) return 0.0;
    
    Product p;
    float total = 0.0;
    while (fread(&p, sizeof(Product), 1, fp)) {
        total += p.price * p.quantity;
    }
    fclose(fp);
    return total;
}
