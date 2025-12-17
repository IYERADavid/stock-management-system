#include "report.h"
#include "product.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

char* generate_report_text() {
    static char buffer[4096];
    FILE *fp = fopen("data/stock.dat", "rb");
    if (!fp) {
        strcpy(buffer, "=== STOCK MANAGEMENT REPORT ===\n\nNo products found in database.\n");
        return buffer;
    }
    
    Product p;
    int count = 0;
    float value = 0.0;
    int totalSold = 0;
    int maxSold = 0;
    int activeId = -1;
    char activeName[50] = "N/A";
    int lowStockCount = 0;
    
    while (fread(&p, sizeof(Product), 1, fp)) {
        count++;
        value += p.price * p.quantity;
        totalSold += p.totalSold;
        
        if (p.totalSold > maxSold) {
            maxSold = p.totalSold;
            activeId = p.id;
            strncpy(activeName, p.name, sizeof(activeName) - 1);
            activeName[sizeof(activeName) - 1] = '\0';
        }
        
        if (p.quantity < 5) {
            lowStockCount++;
        }
    }
    fclose(fp);
    
    time_t now = time(NULL);
    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    
    if (count == 0) {
        strcpy(buffer, "=== STOCK MANAGEMENT REPORT ===\n\nNo products found in database.\n");
    } else {
        sprintf(buffer,
            "========================================================\n"
            "        STOCK MANAGEMENT SYSTEM - REPORT\n"
            "========================================================\n\n"
            "Report Generated: %s\n\n"
            "--------------------------------------------------------\n"
            "SUMMARY STATISTICS\n"
            "--------------------------------------------------------\n\n"
            "Total Products in Inventory: %d\n"
            "Total Stock Value: %.2f\n"
            "Total Units Sold (All Products): %d\n\n"
            "--------------------------------------------------------\n"
            "MOST ACTIVE PRODUCT\n"
            "--------------------------------------------------------\n\n"
            "Product ID: %d\n"
            "Product Name: %s\n"
            "Total Units Sold: %d\n\n"
            "--------------------------------------------------------\n"
            "INVENTORY ALERTS\n"
            "--------------------------------------------------------\n\n"
            "Products with Low Stock (< 5 units): %d\n\n"
            "========================================================\n"
            "End of Report\n"
            "========================================================\n",
            timestamp, count, value, totalSold, activeId, activeName, maxSold, lowStockCount);
    }
    
    return buffer;
}
