#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1024

int main() {
    char filename[] = "CarDetails.csv";  // Nome do arquivo CSV
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        return EXIT_FAILURE;
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        // Remove a quebra de linha no final da linha, se existir
        line[strcspn(line, "\n")] = '\0';

        // Divide a linha em tokens baseados na vírgula
        char *token = strtok(line, ",");
        while (token != NULL) {
            printf("%s\t", token);  // Imprime cada campo
            token = strtok(NULL, ",");
        }
        printf("\n");
    }

    fclose(file);
    return EXIT_SUCCESS;
}