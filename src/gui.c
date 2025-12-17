#include <gtk/gtk.h>
#include "gui.h"
#include "product.h"
#include "file.h"
#include "stock.h"
#include "report.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GtkWidget *window;
GtkWidget *treeview;
GtkListStore *store;
GtkWidget *status_label;
GtkWidget *product_scrolled;

void row_cell_data_func(GtkTreeViewColumn *column, GtkCellRenderer *renderer,
                        GtkTreeModel *model, GtkTreeIter *iter, gpointer data) {
    int quantity;
    gtk_tree_model_get(model, iter, 3, &quantity, -1);
    
    if (quantity < 5) {
        g_object_set(renderer, "foreground", "red", NULL);
    } else {
        g_object_set(renderer, "foreground", "black", NULL);
    }
}

void refresh_table() {
    if (!store) return;
    
    gtk_list_store_clear(store);
    FILE *fp = fopen("data/stock.dat", "rb");
    if (!fp) return;
    
    Product p;
    GtkTreeIter iter;
    while (fread(&p, sizeof(Product), 1, fp)) {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
            0, p.id,
            1, p.name,
            2, p.category,
            3, p.quantity,
            4, p.price,
            -1);
    }
    fclose(fp);
}

void toggle_product_view(GtkWidget *widget, gpointer data) {
    if (gtk_widget_get_visible(product_scrolled)) {
        gtk_widget_hide(product_scrolled);
        gtk_button_set_label(GTK_BUTTON(widget), "View Products");
        gtk_label_set_text(GTK_LABEL(status_label), "Product list hidden. Click 'View Products' to show.");
    } else {
        refresh_table();
        gtk_widget_show(product_scrolled);
        gtk_button_set_label(GTK_BUTTON(widget), "Hide Products");
        gtk_label_set_text(GTK_LABEL(status_label), "Product list displayed. Low stock items (< 5) shown in red.");
    }
}

void add_product_dialog(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Add New Product",
        GTK_WINDOW(window), GTK_DIALOG_MODAL,
        "Add Product", GTK_RESPONSE_OK,
        "Cancel", GTK_RESPONSE_CANCEL,
        NULL);
    
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 15);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 15);
    gtk_container_add(GTK_CONTAINER(content), grid);
    
    GtkWidget *id_entry = gtk_entry_new();
    GtkWidget *name_entry = gtk_entry_new();
    GtkWidget *cat_entry = gtk_entry_new();
    GtkWidget *qty_entry = gtk_entry_new();
    GtkWidget *price_entry = gtk_entry_new();
    
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Product ID:"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), id_entry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Product Name:"), 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), name_entry, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Category:"), 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), cat_entry, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Initial Quantity:"), 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), qty_entry, 1, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Price per Unit:"), 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), price_entry, 1, 4, 1, 1);
    
    gtk_widget_show_all(dialog);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        Product p;
        p.id = atoi(gtk_entry_get_text(GTK_ENTRY(id_entry)));
        strncpy(p.name, gtk_entry_get_text(GTK_ENTRY(name_entry)), sizeof(p.name) - 1);
        p.name[sizeof(p.name) - 1] = '\0';
        strncpy(p.category, gtk_entry_get_text(GTK_ENTRY(cat_entry)), sizeof(p.category) - 1);
        p.category[sizeof(p.category) - 1] = '\0';
        p.quantity = atoi(gtk_entry_get_text(GTK_ENTRY(qty_entry)));
        p.price = atof(gtk_entry_get_text(GTK_ENTRY(price_entry)));
        p.totalSold = 0;
        
        if (p.id > 0 && p.quantity > 0 && p.price > 0 && strlen(p.name) > 0) {
            save_product(p);
            
            char details[256];
            sprintf(details, "Product: %s | Category: %s | Initial Quantity: %d | Price: %.2f per unit", 
                    p.name, p.category, p.quantity, p.price);
            log_history_detailed("PRODUCT ADDED", p.id, details);
            
            refresh_table();
            
            char confirm_msg[512];
            sprintf(confirm_msg, 
                "Product Successfully Added!\n\n"
                "ID: %d\n"
                "Name: %s\n"
                "Category: %s\n"
                "Quantity: %d units\n"
                "Price: %.2f per unit\n"
                "Total Value: %.2f",
                p.id, p.name, p.category, p.quantity, p.price, p.quantity * p.price);
            
            GtkWidget *confirm = gtk_message_dialog_new(GTK_WINDOW(window),
                GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", confirm_msg);
            gtk_dialog_run(GTK_DIALOG(confirm));
            gtk_widget_destroy(confirm);
            
            gtk_label_set_text(GTK_LABEL(status_label), 
                "Product added successfully! Use 'View Products' to see all products.");
        } else {
            GtkWidget *error = gtk_message_dialog_new(GTK_WINDOW(window),
                GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                "Invalid Input!\n\nPlease ensure:\n- ID is a positive number\n- Name is not empty\n- Quantity > 0\n- Price > 0");
            gtk_dialog_run(GTK_DIALOG(error));
            gtk_widget_destroy(error);
            gtk_label_set_text(GTK_LABEL(status_label), "Failed to add product. Please check all fields.");
        }
    }
    gtk_widget_destroy(dialog);
}

void update_stock_dialog(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Update Stock Level",
        GTK_WINDOW(window), GTK_DIALOG_MODAL,
        "Update Stock", GTK_RESPONSE_OK,
        "Cancel", GTK_RESPONSE_CANCEL,
        NULL);
    
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 15);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 15);
    gtk_container_add(GTK_CONTAINER(content), grid);
    
    GtkWidget *id_entry = gtk_entry_new();
    GtkWidget *qty_entry = gtk_entry_new();
    
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Product ID:"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), id_entry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Quantity to Add (must be > 5):"), 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), qty_entry, 1, 1, 1, 1);
    
    gtk_widget_show_all(dialog);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        int id = atoi(gtk_entry_get_text(GTK_ENTRY(id_entry)));
        int qty = atoi(gtk_entry_get_text(GTK_ENTRY(qty_entry)));
        
        if (id > 0 && qty > 5) {
            char product_name[50] = "";
            if (update_stock(id, qty, product_name)) {
                refresh_table();
                
                char msg[256];
                sprintf(msg, "Stock Updated Successfully!\n\nProduct: %s (ID: %d)\nAdded: %d units", 
                        product_name, id, qty);
                
                GtkWidget *success = gtk_message_dialog_new(GTK_WINDOW(window),
                    GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", msg);
                gtk_dialog_run(GTK_DIALOG(success));
                gtk_widget_destroy(success);
                
                gtk_label_set_text(GTK_LABEL(status_label), "Stock updated successfully!");
            } else {
                GtkWidget *error = gtk_message_dialog_new(GTK_WINDOW(window),
                    GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                    "Update Failed!\n\nProduct ID %d not found in database.", id);
                gtk_dialog_run(GTK_DIALOG(error));
                gtk_widget_destroy(error);
                gtk_label_set_text(GTK_LABEL(status_label), "Product not found!");
            }
        } else {
            GtkWidget *error = gtk_message_dialog_new(GTK_WINDOW(window),
                GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
                "Invalid Input!\n\nQuantity to add must be greater than 5.");
            gtk_dialog_run(GTK_DIALOG(error));
            gtk_widget_destroy(error);
            gtk_label_set_text(GTK_LABEL(status_label), "Invalid quantity! Must be > 5.");
        }
    }
    gtk_widget_destroy(dialog);
}

void sell_product_dialog(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Sell Product",
        GTK_WINDOW(window), GTK_DIALOG_MODAL,
        "Process Sale", GTK_RESPONSE_OK,
        "Cancel", GTK_RESPONSE_CANCEL,
        NULL);
    
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 15);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 15);
    gtk_container_add(GTK_CONTAINER(content), grid);
    
    GtkWidget *id_entry = gtk_entry_new();
    GtkWidget *qty_entry = gtk_entry_new();
    GtkWidget *disc_entry = gtk_entry_new();
    
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Product ID:"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), id_entry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Quantity to Sell (must be > 5):"), 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), qty_entry, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Discount Percentage (10-20%):"), 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), disc_entry, 1, 2, 1, 1);
    
    gtk_widget_show_all(dialog);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        int id = atoi(gtk_entry_get_text(GTK_ENTRY(id_entry)));
        int qty = atoi(gtk_entry_get_text(GTK_ENTRY(qty_entry)));
        int discount = atoi(gtk_entry_get_text(GTK_ENTRY(disc_entry)));
        
        if (id > 0 && qty > 5 && discount >= 10 && discount <= 20) {
            char product_name[50] = "";
            float original_price = 0;
            float final_amount = sell_product(id, qty, discount, product_name, &original_price);
            
            if (final_amount >= 0) {
                refresh_table();
                
                float original_total = original_price * qty;
                float discount_amount = original_total * (discount / 100.0);
                
                char msg[512];
                sprintf(msg,
                    "Sale Processed Successfully!\n\n"
                    "Product: %s (ID: %d)\n"
                    "Quantity Sold: %d units\n"
                    "Unit Price: %.2f\n"
                    "Original Total: %.2f\n"
                    "Discount: %d%% (%.2f)\n"
                    "Final Amount: %.2f",
                    product_name, id, qty, original_price, original_total, 
                    discount, discount_amount, final_amount);
                
                GtkWidget *success = gtk_message_dialog_new(GTK_WINDOW(window),
                    GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", msg);
                gtk_dialog_run(GTK_DIALOG(success));
                gtk_widget_destroy(success);
                
                gtk_label_set_text(GTK_LABEL(status_label), "Product sold successfully!");
            } else {
                GtkWidget *error = gtk_message_dialog_new(GTK_WINDOW(window),
                    GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                    "Sale Failed!\n\nProduct ID %d not found or insufficient stock available.", id);
                gtk_dialog_run(GTK_DIALOG(error));
                gtk_widget_destroy(error);
                gtk_label_set_text(GTK_LABEL(status_label), "Sale failed! Check product ID and stock level.");
            }
        } else {
            GtkWidget *error = gtk_message_dialog_new(GTK_WINDOW(window),
                GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
                "Invalid Input!\n\nPlease ensure:\n- Quantity > 5\n- Discount between 10-20%%");
            gtk_dialog_run(GTK_DIALOG(error));
            gtk_widget_destroy(error);
            gtk_label_set_text(GTK_LABEL(status_label), "Invalid input! Check quantity (>5) and discount (10-20%).");
        }
    }
    gtk_widget_destroy(dialog);
}

void remove_product_dialog(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Remove Product",
        GTK_WINDOW(window), GTK_DIALOG_MODAL,
        "Find Product", GTK_RESPONSE_OK,
        "Cancel", GTK_RESPONSE_CANCEL,
        NULL);
    
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 15);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 15);
    gtk_container_add(GTK_CONTAINER(content), grid);
    
    GtkWidget *id_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Product ID to Remove:"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), id_entry, 1, 0, 1, 1);
    
    gtk_widget_show_all(dialog);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        int id = atoi(gtk_entry_get_text(GTK_ENTRY(id_entry)));
        if (id > 0) {
            char product_name[50] = "";
            Product *p = find_product(id);
            
            if (p) {
                strcpy(product_name, p->name);
                
                char confirm_msg[256];
                sprintf(confirm_msg,
                    "Confirm Product Removal\n\n"
                    "Product ID: %d\n"
                    "Name: %s\n"
                    "Category: %s\n"
                    "Current Stock: %d units\n\n"
                    "Are you sure you want to remove this product?",
                    id, p->name, p->category, p->quantity);
                
                GtkWidget *confirm = gtk_message_dialog_new(GTK_WINDOW(window),
                    GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, 
                    GTK_BUTTONS_YES_NO, "%s", confirm_msg);
                
                if (gtk_dialog_run(GTK_DIALOG(confirm)) == GTK_RESPONSE_YES) {
                    if (remove_product(id, product_name)) {
                        refresh_table();
                        
                        GtkWidget *success = gtk_message_dialog_new(GTK_WINDOW(window),
                            GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
                            "Product Removed Successfully!\n\nProduct '%s' (ID: %d) has been removed from inventory.",
                            product_name, id);
                        gtk_dialog_run(GTK_DIALOG(success));
                        gtk_widget_destroy(success);
                        
                        gtk_label_set_text(GTK_LABEL(status_label), "Product removed successfully!");
                    }
                }
                gtk_widget_destroy(confirm);
            } else {
                GtkWidget *error = gtk_message_dialog_new(GTK_WINDOW(window),
                    GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                    "Product Not Found!\n\nProduct ID %d does not exist in the database.", id);
                gtk_dialog_run(GTK_DIALOG(error));
                gtk_widget_destroy(error);
                gtk_label_set_text(GTK_LABEL(status_label), "Product not found!");
            }
        } else {
            gtk_label_set_text(GTK_LABEL(status_label), "Invalid product ID!");
        }
    }
    gtk_widget_destroy(dialog);
}

void check_stock_dialog(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Check Stock Level",
        GTK_WINDOW(window), GTK_DIALOG_MODAL,
        "Check", GTK_RESPONSE_OK,
        "Cancel", GTK_RESPONSE_CANCEL,
        NULL);
    
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 15);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 15);
    gtk_container_add(GTK_CONTAINER(content), grid);
    
    GtkWidget *id_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Product ID:"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), id_entry, 1, 0, 1, 1);
    
    gtk_widget_show_all(dialog);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        int id = atoi(gtk_entry_get_text(GTK_ENTRY(id_entry)));
        int qty = check_stock_level(id);
        
        if (qty >= 0) {
            Product *p = find_product(id);
            char msg[512];
            
            if (qty < 5) {
                sprintf(msg,
                    "⚠️ LOW STOCK WARNING ⚠️\n\n"
                    "Product: %s (ID: %d)\n"
                    "Category: %s\n"
                    "Current Stock: %d units\n"
                    "Unit Price: %.2f\n\n"
                    "WARNING: Stock level is below 5 units!\n"
                    "Please consider restocking soon.",
                    p->name, id, p->category, qty, p->price);
                
                GtkWidget *warning = gtk_message_dialog_new(GTK_WINDOW(window),
                    GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "%s", msg);
                gtk_dialog_run(GTK_DIALOG(warning));
                gtk_widget_destroy(warning);
            } else {
                sprintf(msg,
                    "Stock Level Information\n\n"
                    "Product: %s (ID: %d)\n"
                    "Category: %s\n"
                    "Current Stock: %d units\n"
                    "Unit Price: %.2f\n"
                    "Total Stock Value: %.2f",
                    p->name, id, p->category, qty, p->price, qty * p->price);
                
                GtkWidget *info = gtk_message_dialog_new(GTK_WINDOW(window),
                    GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", msg);
                gtk_dialog_run(GTK_DIALOG(info));
                gtk_widget_destroy(info);
            }
            gtk_label_set_text(GTK_LABEL(status_label), 
                qty < 5 ? "Low stock warning displayed!" : "Stock level checked.");
        } else {
            GtkWidget *error = gtk_message_dialog_new(GTK_WINDOW(window),
                GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                "Product Not Found!\n\nProduct ID %d does not exist in the database.", id);
            gtk_dialog_run(GTK_DIALOG(error));
            gtk_widget_destroy(error);
            gtk_label_set_text(GTK_LABEL(status_label), "Product not found!");
        }
    }
    gtk_widget_destroy(dialog);
}

void calculate_value_dialog(GtkWidget *widget, gpointer data) {
    float value = calculate_stock_value();
    int count = get_product_count();
    
    char msg[512];
    sprintf(msg,
        "Total Stock Value Calculation\n\n"
        "Total Products: %d\n"
        "Total Stock Value: %.2f\n\n"
        "This represents the combined value of all products\n"
        "currently in inventory (quantity × unit price).",
        count, value);
    
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
        GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", msg);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    
    gtk_label_set_text(GTK_LABEL(status_label), "Stock value calculated.");
}

void show_report(GtkWidget *widget, gpointer data) {
    char *report = generate_report_text();
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Stock Management Report",
        GTK_WINDOW(window), GTK_DIALOG_MODAL,
        "Close", GTK_RESPONSE_CLOSE,
        NULL);
    
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *text_view = gtk_text_view_new();
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    
    gtk_text_buffer_set_text(buffer, report, -1);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
    gtk_container_add(GTK_CONTAINER(scrolled), text_view);
    gtk_container_add(GTK_CONTAINER(content), scrolled);
    gtk_widget_set_size_request(scrolled, 500, 350);
    
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    
    gtk_label_set_text(GTK_LABEL(status_label), "Report generated.");
}

void view_history(GtkWidget *widget, gpointer data) {
    char *history = load_history();
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Operation History",
        GTK_WINDOW(window), GTK_DIALOG_MODAL,
        "Close", GTK_RESPONSE_CLOSE,
        NULL);
    
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *text_view = gtk_text_view_new();
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    
    gtk_text_buffer_set_text(buffer, history, -1);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(text_view), TRUE);
    gtk_container_add(GTK_CONTAINER(scrolled), text_view);
    gtk_container_add(GTK_CONTAINER(content), scrolled);
    gtk_widget_set_size_request(scrolled, 600, 450);
    
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    
    gtk_label_set_text(GTK_LABEL(status_label), "History viewed.");
}

void create_main_window() {
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Stock Management System - AUCA 2025");
    gtk_window_set_default_size(GTK_WINDOW(window), 850, 550);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 12);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    
    GtkWidget *title = gtk_label_new("Stock Management System");
    PangoFontDescription *font_desc = pango_font_description_from_string("Sans Bold 18");
    gtk_widget_override_font(title, font_desc);
    pango_font_description_free(font_desc);
    gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE, 8);
    
    GtkWidget *button_box1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), button_box1, FALSE, FALSE, 5);
    
    GtkWidget *add_btn = gtk_button_new_with_label("Add Product");
    GtkWidget *update_btn = gtk_button_new_with_label("Update Stock");
    GtkWidget *sell_btn = gtk_button_new_with_label("Sell Product");
    GtkWidget *remove_btn = gtk_button_new_with_label("Remove Product");
    
    gtk_box_pack_start(GTK_BOX(button_box1), add_btn, TRUE, TRUE, 3);
    gtk_box_pack_start(GTK_BOX(button_box1), update_btn, TRUE, TRUE, 3);
    gtk_box_pack_start(GTK_BOX(button_box1), sell_btn, TRUE, TRUE, 3);
    gtk_box_pack_start(GTK_BOX(button_box1), remove_btn, TRUE, TRUE, 3);
    
    GtkWidget *button_box2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), button_box2, FALSE, FALSE, 5);
    
    GtkWidget *check_btn = gtk_button_new_with_label("Check Stock");
    GtkWidget *value_btn = gtk_button_new_with_label("Calculate Value");
    GtkWidget *report_btn = gtk_button_new_with_label("Generate Report");
    GtkWidget *history_btn = gtk_button_new_with_label("View History");
    GtkWidget *view_btn = gtk_button_new_with_label("View Products");
    
    gtk_box_pack_start(GTK_BOX(button_box2), check_btn, TRUE, TRUE, 3);
    gtk_box_pack_start(GTK_BOX(button_box2), value_btn, TRUE, TRUE, 3);
    gtk_box_pack_start(GTK_BOX(button_box2), report_btn, TRUE, TRUE, 3);
    gtk_box_pack_start(GTK_BOX(button_box2), history_btn, TRUE, TRUE, 3);
    gtk_box_pack_start(GTK_BOX(button_box2), view_btn, TRUE, TRUE, 3);
    
    g_signal_connect(add_btn, "clicked", G_CALLBACK(add_product_dialog), NULL);
    g_signal_connect(update_btn, "clicked", G_CALLBACK(update_stock_dialog), NULL);
    g_signal_connect(sell_btn, "clicked", G_CALLBACK(sell_product_dialog), NULL);
    g_signal_connect(remove_btn, "clicked", G_CALLBACK(remove_product_dialog), NULL);
    g_signal_connect(check_btn, "clicked", G_CALLBACK(check_stock_dialog), NULL);
    g_signal_connect(value_btn, "clicked", G_CALLBACK(calculate_value_dialog), NULL);
    g_signal_connect(report_btn, "clicked", G_CALLBACK(show_report), NULL);
    g_signal_connect(history_btn, "clicked", G_CALLBACK(view_history), NULL);
    g_signal_connect(view_btn, "clicked", G_CALLBACK(toggle_product_view), NULL);
    
    product_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(product_scrolled),
        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_hide(product_scrolled);
    gtk_box_pack_start(GTK_BOX(vbox), product_scrolled, TRUE, TRUE, 5);
    
    store = gtk_list_store_new(5, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, G_TYPE_FLOAT);
    treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("ID", renderer, "text", 0, NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer, row_cell_data_func, NULL, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Name", renderer, "text", 1, NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer, row_cell_data_func, NULL, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Category", renderer, "text", 2, NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer, row_cell_data_func, NULL, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Quantity", renderer, "text", 3, NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer, row_cell_data_func, NULL, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Price", renderer, "text", 4, NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer, row_cell_data_func, NULL, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    
    gtk_container_add(GTK_CONTAINER(product_scrolled), treeview);
    
    status_label = gtk_label_new("Welcome! Use the buttons above to manage your stock.");
    gtk_label_set_line_wrap(GTK_LABEL(status_label), TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), status_label, FALSE, FALSE, 5);
    
    gtk_widget_show_all(window);
    gtk_widget_hide(product_scrolled);
}
