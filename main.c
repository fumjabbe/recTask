#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <windows.h>

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
/*
The temperature sensor reports a temperature range of -50C to +50C and can be read every 100ms.
For example the ADC can read the following values from the sensor:

2048 (rougly 0C)
3000 (rougly 23C)
*/
double getTemperature(uint16_t index)
{
    uint16_t adcvalue;

    adcvalue = ADCvalue(index);

    Sleep(100);
    
    if (adcvalue < 0 || adcvalue > 4095) {
        printf("Error: ADC value must be between 0 and 4095.\n");
        return 0;  // Return an error value
    }

    //range of ADC with 12 bit resolution is 4095
    return ((adcvalue / 4095.0) * 100.0) - 50.0;

}

void main()
{
    double temp = 0.0;
    uint16_t index = 0;
    index = 30;
    temp = getTemperature(index);
    printf("Temperature: %.2fC\n", temp);
    index = 162;
    temp = getTemperature(index);
    printf("Temperature: %.2fC\n", temp);
    index = 173;
    temp = getTemperature(index);
    printf("Temperature: %.2fC\n", temp);
    index = 137;
    temp = getTemperature(index);
    printf("Temperature: %.2fC\n", temp);

}