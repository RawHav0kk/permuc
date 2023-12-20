#include "data.h"
#include "version.h"
#include <ctype.h>
#include <gtk/gtk.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Declarations of UI objects
GtkBuilder *builder;
GtkWidget *window;
GtkWidget *buttonEnc;
GtkWidget *buttonDec;
GtkEntry *inputEnc;
GtkEntry *inputPlain;
GtkEntry *inputKey;
GtkLabel *labelStatus;
GtkEntry *entryKey;
GtkWidget *spinPasses;
GtkWidget *spinColumns;

void createKeyIndexTable(const char *_numericalString, int *indexTable) {
  // Get the length of the string
  char *numericalString =
      (char *)malloc(strlen(_numericalString) * sizeof(char));
  if (numericalString == NULL) {
    fprintf(stderr, "Memory allocation error\n");
    exit(EXIT_FAILURE);
  }
  strcpy(numericalString, _numericalString);

  int length = 0;
  while (numericalString[length] != '\0') {
    length++;
  }

  // Create an array to store the original indices
  int *originalIndices = (int *)malloc(length * sizeof(int));
  for (int i = 0; i < length; i++) {
    originalIndices[i] = i;
  }

  // Bubble sort to order the numbers and update the original indices
  for (int i = 0; i < length - 1; i++) {
    for (int j = 0; j < length - i - 1; j++) {
      if (numericalString[j] > numericalString[j + 1]) {
        // Swap numbers
        char tempChar = numericalString[j];
        numericalString[j] = numericalString[j + 1];
        numericalString[j + 1] = tempChar;
        // Swap indices
        int tempIndex = originalIndices[j];
        originalIndices[j] = originalIndices[j + 1];
        originalIndices[j + 1] = tempIndex;
      }
    }
  }
  printf("[DEBUG]Key table: ");

  for (int i = 0; i <= length; i++) {
    printf("%d, ", originalIndices[i]);
  }
  printf("\n");

  // Update the indexTable with the ordered indices
  for (int i = 0; i < length; i++) {
    indexTable[i] = originalIndices[i];
  }

  // Free the dynamically allocated arrays
  free(numericalString);
}

//char *displayArr(int *arr[]) {
//  // make dupe of an array
//  int length = sizeof(arr) / sizeof(arr[0]);
//  int arrLength = 2 * length;
//
//  char *result = (char *)malloc(arrLength * sizeof(char));
//  if (result == NULL) {
//    fprintf(stderr, "Memory allocation error\n");
//    exit(EXIT_FAILURE);
//  }
//
//  int offset = 0;
//  for (int i = 0; i < length; i++) {
//    offset += snprintf(result + offset, arrLength - offset, "%d", arr[i]);
//    if (i < length - 1) {
//      offset += snprintf(result + offset, arrLength - offset, ", ");
//    }
//  }
//
//  return result;
//}

// Encrypt
const gchar *encrypt(const char *message, const char *key, gint numColumns,
                     gint _passes) {

  if (message == NULL || key == NULL) {
    printf("[ERROR] Encrypt or key is null!\n");
    return NULL;
  }
  int len_message = strlen(message);
  int len_key = strlen(key);
  printf("[DEBUG] len_message = %d ; len_key = %d \n", len_message, len_key);
  int columns = (numColumns <= 3) ? (columns = 17) : (columns = numColumns);
  int passes = (_passes <= 0) ? (passes = 1) : (passes = _passes);
  int rows = (int)(len_message / columns) + 1;

  int keyTable[len_key];
  createKeyIndexTable(key, keyTable);

  printf("[DEBUG] Creating grid with %d columns and %d rows\n", columns, rows);

  if (rows < 4) {
    printf("[WARN] the amount of rows is too small (min: 4), setting rows to "
           "4.\n");
    rows = 4;
  }
  printf("[INFO] Creating grid with %d columns and %d rows\n", columns, rows);
  char grid[rows][columns];
  // Create a 2D array for the grid


  int opt_len = (columns * rows) + 1;
  char temp[opt_len];
  strcpy(temp, message);
  for (int i = len_message; i < len_message + (rows - 1) * columns; i++) {
    temp[i] = (i < len_message) ? temp[i] : '\127';
  }

  for (int c = 0; c < passes; c++) {
    printf("[DEBUG] pass %d\n", c + 1);
    // Fill the grid with the message
    int rowIndex, colIndex;

    for (rowIndex = 0; rowIndex < rows; rowIndex++) {
      for (colIndex = 0; colIndex < columns; colIndex++) {
        int messageIndex = rowIndex * columns + colIndex;
          grid[rowIndex][colIndex] = temp[messageIndex];
   //     }
      }
    }

    for (int i = 0; i <= columns; i += len_key) {
      for (int j = 0; j < len_key; j++) {

        printf("%c | ", key[j]);
      }
    }

    printf("\n");
    for (int i = 0; i < rows; ++i) {
      for (int j = 0; j < columns; ++j) {
        if (grid[i][j] == '\127') {
          printf("● | ");
        } else {
          printf("%c | ", grid[i][j]);
        }
      }
      printf("\n");
    }

    // clear temp buffer
    memset(temp, 0, sizeof(temp));
    printf("[DEBUG] Successfuly cleared temp!\n");

    for (int i = 0; i < columns; i += len_key) {
      int col = i;
      for (int j = 0; j < len_key; j++) {
        int keyOff = keyTable[j];
        int cCol = col + keyOff;
        if (cCol >= 0 && cCol < columns) {
          for (int x = 0; x < rows; x++) {
            strncat(temp, &grid[x][cCol], 1);
            printf("[DEBUG]: current char: %d – %c \n", x, grid[x][cCol]);
          }
        } else {
          printf("[WARNING]: Out of bounds access at cCol = %d\n", cCol);
        }
      }
    }
    printf("[INFO] temp: %s \n", temp);
  }

  char *result = strdup(temp);

  return result;
}

// Key function

gint keyValid(const gchar *key) {
  if (key == NULL || key[0] == '\0') {
    return -1;
  }
  for (int i = 0; i < strlen(key); i++) {
    if (isdigit(key[i]) == false) {
      return 0;
    }
  }
  return 1;
}

// Get functions
const gchar *getKey() {
  const gchar *stringKey = gtk_entry_get_text(entryKey);
  printf("[DEBUG] KeyStr:%s \n", stringKey);
  switch (keyValid(stringKey)) {
  case 1:
    return gtk_entry_get_text(GTK_ENTRY(entryKey));
    break;
  case 0:
    printf("[ERROR] Key %s is not a number!\n", stringKey);
    return NULL;
    break;
  case -1:
    printf("[ERROR] Key is empty!\n");
    return NULL;
    break;
  }
}

const gchar *getPlain() { return gtk_entry_get_text(GTK_ENTRY(inputPlain)); }
const gchar *getEnc() { return gtk_entry_get_text(GTK_ENTRY(inputEnc)); }
size_t getEncL() { return gtk_entry_get_text_length(GTK_ENTRY(inputEnc)); }
size_t getPlainL() { return gtk_entry_get_text_length(GTK_ENTRY(inputPlain)); }
gint getColumns() {
  return gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinColumns));
}
gint getPasses() {
  return gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinPasses));
}

int main(int argc, char *argv[]) {
  if (argc >= 2) {
    if (0 == strcmp(argv[1], "--version"))
      version();
  }

  gtk_init(&argc, &argv);

  builder = gtk_builder_new_from_resource(APP_PREFIX "/window_main.glade");
  window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));

  gtk_builder_connect_signals(builder, NULL);
  // g_object_unref(builder);
  // bindings
  window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
  buttonEnc = GTK_WIDGET(gtk_builder_get_object(builder, "buttonEnc"));
  buttonDec = GTK_WIDGET(gtk_builder_get_object(builder, "buttonDec"));
  inputEnc = GTK_ENTRY(gtk_builder_get_object(builder, "inputEnc"));
  inputPlain = GTK_ENTRY(gtk_builder_get_object(builder, "inputPlain"));
  labelStatus = GTK_LABEL(gtk_builder_get_object(builder, "labelStatus"));
  entryKey = GTK_ENTRY(gtk_builder_get_object(builder, "entryKey"));
  spinPasses = GTK_WIDGET(gtk_builder_get_object(builder, "spinPasses"));
  spinColumns = GTK_WIDGET(gtk_builder_get_object(builder, "spinColumns"));

  gtk_widget_show(window);
  gtk_main();
  return 0;
}

void on_buttonDec_clicked() {}

void on_buttonEnc_clicked() {

  if (getPlainL() == 0) {
    gtk_label_set_text(GTK_ENTRY(labelStatus),
                       (const gchar *)"Plain input is empty!");
  } else {

    const gchar *enc_out =
        encrypt(getPlain(), getKey(), getColumns(), getPasses());
    printf("[DEBUG] encrypt returned: %s \n", enc_out);

    if (enc_out != NULL) {
      gtk_entry_set_text(GTK_ENTRY(inputEnc), enc_out);
      gtk_label_set_text(GTK_ENTRY(labelStatus),
                         (const gchar *)"Message encrypted");
      free(enc_out);
    } else {
      gtk_label_set_text(GTK_ENTRY(labelStatus),
                         (const gchar *)"Returned text is NULL!");
    }
  }
}

void on_window_main_destroy() { gtk_main_quit(); }
