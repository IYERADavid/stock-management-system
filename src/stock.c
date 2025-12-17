#include "stock.h"
#include "product.h"
#include "file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int update_stock(int id, int qty, char *product_name) {
    if (qty <= 5) return 0;
    
    FILE *fp = fopen("data/stock.dat", "rb+");
    if (!fp) return 0;
    
    Product p;
    int found = 0;
    while (fread(&p, sizeof(Product), 1, fp)) {
        if (p.id == id) {
            int old_qty = p.quantity;
            p.quantity += qty;
            long offset = -(long)sizeof(Product);
            fseek(fp, offset, SEEK_CUR);
            fwrite(&p, sizeof(Product), 1, fp);
            found = 1;
            
            char details[256];
            sprintf(details, "Product: %s | Added: %d units | Old Stock: %d | New Stock: %d", 
                    p.name, qty, old_qty, p.quantity);
            log_history_detailed("STOCK UPDATE", id, details);
            if (product_name) strcpy(product_name, p.name);
            break;
        }
    }
    fclose(fp);
    return found;
}

float sell_product(int id, int qty, int discount, char *product_name, float *original_price) {
    if (qty <= 5 || discount < 10 || discount > 20) return -1;
    
    FILE *fp = fopen("data/stock.dat", "rb+");
    if (!fp) return -1;
    
    Product p;
    float total_amount = -1;
    while (fread(&p, sizeof(Product), 1, fp)) {
        if (p.id == id && p.quantity >= qty) {
            float original_total = p.price * qty;
            float discount_amount = original_total * (discount / 100.0);
            total_amount = original_total - discount_amount;
            
            int old_qty = p.quantity;
            p.quantity -= qty;
            p.totalSold += qty;
            long offset = -(long)sizeof(Product);
            fseek(fp, offset, SEEK_CUR);
            fwrite(&p, sizeof(Product), 1, fp);
            
            char details[512];
            sprintf(details, "Product: %s | Quantity: %d | Unit Price: %.2f | Original Total: %.2f | Discount: %d%% (%.2f) | Final Amount: %.2f | Stock: %d -> %d", 
                    p.name, qty, p.price, original_total, discount, discount_amount, total_amount, old_qty, p.quantity);
            log_history_detailed("PRODUCT SOLD", id, details);
            
            if (product_name) strcpy(product_name, p.name);
            if (original_price) *original_price = p.price;
            break;
        }
    }
    fclose(fp);
    return total_amount;
}

int remove_product(int id, char *product_name) {
    FILE *fp = fopen("data/stock.dat", "rb");
    if (!fp) return 0;
    
    FILE *temp = fopen("data/temp.dat", "wb");
    if (!temp) {
        fclose(fp);
        return 0;
    }
    
    Product p;
    int found = 0;
    while (fread(&p, sizeof(Product), 1, fp)) {
        if (p.id != id) {
            fwrite(&p, sizeof(Product), 1, temp);
        } else {
            found = 1;
            if (product_name) strcpy(product_name, p.name);
            
            char details[256];
            sprintf(details, "Product: %s | Category: %s | Removed from inventory", p.name, p.category);
            log_history_detailed("PRODUCT REMOVED", id, details);
        }
    }
    fclose(fp);
    fclose(temp);
    
    if (found) {
        remove("data/stock.dat");
        rename("data/temp.dat", "data/stock.dat");
    } else {
        remove("data/temp.dat");
    }
    return found;
}

int check_stock_level(int id) {
    Product *p = find_product(id);
    return p ? p->quantity : -1;
}
