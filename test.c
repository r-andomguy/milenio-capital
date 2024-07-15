#include <stdio.h>
#include "libcsv.h"

int main() {
    const char* csvFilePath = "data.csv";
    const char* selectedColumns = "header1,header3";
    const char* rowFilterDefinitions = "header1=4\nheader2>3";
    
    processCsvFile(csvFilePath, selectedColumns, rowFilterDefinitions);
    return 0;
}

