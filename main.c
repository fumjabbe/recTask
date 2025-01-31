#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
/*
ADCvalue()
Testfunction that reads a test ADC value from a file a location "index".
*/
uint16_t ADCvalue(uint16_t index) {
    FILE *file;
    uint16_t linecounter = 0;
    char buffer[32];  // Buffer to store each line

    // Open the file in read mode
    file = fopen("temperature.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        return 0;
    }

    // Read and print each line from the file
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        linecounter++;
        if(linecounter == index)
        {
            fclose(file);
            return atoi(buffer);
        }
    }
    //index not found
    perror("Error finding line number");
    fclose(file);
    return 0;
}
double getTemperature()
{

}

void main()
{
    uint16_t value;
    uint16_t index = 0;
    index = 30;
    value = ADCvalue(index);
    printf("ok\n");
    printf("%d",value);
}