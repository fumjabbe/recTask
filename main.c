#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#define ADC_READ_INTERVAL 100
static clock_t last_read_time = 0;  // Store last read time

/*
ADCvalue()
Testfunction that reads a test ADC value from a file a location "index".
*/
uint16_t ADCvalue(uint16_t index) {
    FILE *file;
    uint16_t linecounter = 0;
    char buffer[32];
    uint16_t adcvalue;

    //open file
    file = fopen("temperature.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        return 500;
    }

    // Read each line from the file until location is reached
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        linecounter++;
        if(linecounter == index)
        {
            fclose(file);
            adcvalue = atoi(buffer);
            if (adcvalue < 0 || adcvalue > 4095) {
                printf("Error: ADC value must be between 0 and 4095.\n");
                return 500;  // Return an error value
            }
            return adcvalue;
        }
    }
    //index not found
    perror("Error finding line number");
    fclose(file);
    return 500;
}
/*
The temperature sensor reports a temperature range of -50C to +50C and can be read every 100ms.
For example the ADC can read the following values from the sensor:

2048 (rougly 0C)
3000 (rougly 23C)
*/
double getTemperature(uint16_t index)
{
    clock_t current_time = clock();
    double elapsed_time = (double)(current_time - last_read_time);
    
     if (elapsed_time >= ADC_READ_INTERVAL) {
        last_read_time = current_time;  // Update last read time
        //range of ADC with 12 bit resolution is 4095
        return ((ADCvalue(index) / 4095.0) * 100.0) - 50.0;
    } else {
        printf("Waiting for next valid read...\n");
        return 500;  // Indicate that it's too soon to read
    }
}

void main()
{
    double temp = 0.0;
    uint16_t index = 0;

    struct timespec ts = {0, 100 * 1000000L};  // Sleep for 100ms

    nanosleep(&ts, NULL);
    index = 30;
    temp = getTemperature(index);
    printf("Temperature: %.2fC\n", temp);
    
    nanosleep(&ts, NULL);
    index = 162;
    temp = getTemperature(index);
    printf("Temperature: %.2fC\n", temp);
    
    nanosleep(&ts, NULL);
    index = 173;
    temp = getTemperature(index);
    printf("Temperature: %.2fC\n", temp);
    
    nanosleep(&ts, NULL);
    index = 137;
    temp = getTemperature(index);
    printf("Temperature: %.2fC\n", temp);

}