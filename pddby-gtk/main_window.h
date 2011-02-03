#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <gtk/gtk.h>

GtkWidget *main_window_new();

void on_training_section();
void on_training_topic();
void on_training_ticket();
void on_training_random_ticket();
void on_exam_topic();
void on_exam_ticket();
void on_exam_random_ticket();

#endif // MAIN_WINDOW_H