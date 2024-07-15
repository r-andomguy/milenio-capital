#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libcsv.h"

char** splitString(const char* str, char delimiter, int* count) {
    char** result = NULL;
    int size = 0;
    char* token;
    char* strCopy = strdup(str);
    char* rest = strCopy;

    while ((token = strtok_r(rest, &delimiter, &rest))) {
        result = realloc(result, sizeof(char*) * ++size);
        result[size - 1] = strdup(token);
    }
    *count = size;
    free(strCopy);
    return result;
}

int compareStrings(const char* a, const char* b, const char* op) {
    int cmp = strcmp(a, b);
    if (strcmp(op, "=") == 0) return cmp == 0;
    if (strcmp(op, ">") == 0) return cmp > 0;
    if (strcmp(op, "<") == 0) return cmp < 0;
    return 0;
}

void processRow(char* row, const int* selectedIndices, int selectedCount, FILE* output) {
    int columnCount;
    char** columns = splitString(row, ',', &columnCount);

    for (int i = 0; i < selectedCount; ++i) {
        if (i > 0) fprintf(output, ",");
        fprintf(output, "%s", columns[selectedIndices[i]]);
    }
    fprintf(output, "\n");
    freeStringArray(columns, columnCount);
}

int* getSelectedIndices(const char* header, const char* selectedColumns, int* selectedCount) {
    int headerCount;
    char** headers = splitString(header, ',', &headerCount);

    if (strlen(selectedColumns) == 0) {
        *selectedCount = headerCount;
        int* indices = malloc(headerCount * sizeof(int));
        for (int i = 0; i < headerCount; ++i) indices[i] = i;
        freeStringArray(headers, headerCount);
        return indices;
    }

    int selectedColCount;
    char** selectedCols = splitString(selectedColumns, ',', &selectedColCount);

    int* indices = malloc(selectedColCount * sizeof(int));
    for (int i = 0; i < selectedColCount; ++i) {
        for (int j = 0; j < headerCount; ++j) {
            if (strcmp(selectedCols[i], headers[j]) == 0) {
                indices[i] = j;
                break;
            }
        }
    }
    *selectedCount = selectedColCount;
    freeStringArray(headers, headerCount);
    freeStringArray(selectedCols, selectedColCount);
    return indices;
}

int* getFilterIndices(const char* header, const char* rowFilterDefinitions, char** filters, char** ops, char** values, int* filterCount) {
    int headerCount;
    char** headers = splitString(header, ',', &headerCount);
    
    int defCount;
    char** definitions = splitString(rowFilterDefinitions, '\n', &defCount);

    int* indices = malloc(defCount * sizeof(int));
    for (int i = 0; i < defCount; ++i) {
        filters[i] = strdup(definitions[i]);
        ops[i] = strpbrk(filters[i], "=><");
        *ops[i]++ = '\0';
        values[i] = ops[i] + strlen(ops[i]);
        *values[i] = '\0';

        for (int j = 0; j < headerCount; ++j) {
            if (strcmp(filters[i], headers[j]) == 0) {
                indices[i] = j;
                break;
            }
        }
    }
    *filterCount = defCount;
    freeStringArray(headers, headerCount);
    freeStringArray(definitions, defCount);
    return indices;
}

int applyFilters(char** rowValues, const int* filterIndices, char** ops, char** values, int filterCount) {
    for (int i = 0; i < filterCount; ++i) {
        if (!compareStrings(rowValues[filterIndices[i]], values[i], ops[i])) {
            return 0;
        }
    }
    return 1;
}

void freeStringArray(char** array, int count) {
    for (int i = 0; i < count; ++i) {
        free(array[i]);
    }
    free(array);
}

void processCsv(const char csv[], const char selectedColumns[], const char rowFilterDefinitions[]) {
    int rowCount;
    char** rows = splitString(csv, '\n', &rowCount);

    int selectedCount, filterCount;
    int* selectedIndices = getSelectedIndices(rows[0], selectedColumns, &selectedCount);

    char** filters = malloc(rowCount * sizeof(char*));
    char** ops = malloc(rowCount * sizeof(char*));
    char** values = malloc(rowCount * sizeof(char*));
    int* filterIndices = getFilterIndices(rows[0], rowFilterDefinitions, filters, ops, values, &filterCount);

    processRow(rows[0], selectedIndices, selectedCount, stdout);

    for (int i = 1; i < rowCount; ++i) {
        int columnCount;
        char** rowValues = splitString(rows[i], ',', &columnCount);
        if (applyFilters(rowValues, filterIndices, ops, values, filterCount)) {
            processRow(rows[i], selectedIndices, selectedCount, stdout);
        }
        freeStringArray(rowValues, columnCount);
    }

    freeStringArray(rows, rowCount);
    free(selectedIndices);
    free(filterIndices);
    free(filters);
    free(ops);
    free(values);
}

void processCsvFile(const char csvFilePath[], const char selectedColumns[], const char rowFilterDefinitions[]) {
    FILE* file = fopen(csvFilePath, "r");
    if (!file) {
        perror("Failed to open file");
        return;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = malloc(fileSize + 1);
    fread(buffer, 1, fileSize, file);
    buffer[fileSize] = '\0';
    fclose(file);

    processCsv(buffer, selectedColumns, rowFilterDefinitions);
    free(buffer);
}
