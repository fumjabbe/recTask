#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define ADC_READ_INTERVAL 100
#define FILE_READ_END 100
static clock_t last_time_100ms = 0;       // Store last time 100ms was reached

/*
ADCvalue()
Testfunction that reads a test ADC value from a file a location "index".
*/
uint16_t ADCvalue(uint16_t index) 
{
    FILE *file;
    uint16_t linecounter = 0;
    char buffer[32];
    uint16_t adcvalue;

    //open file
    file = fopen("temperature.txt", "r");
    if (file == NULL) 
    {
        perror("Error opening file");
        return 0;
    }

    // Read each line from the file until location is reached or end-of-file is reached
    while (fgets(buffer, sizeof(buffer), file) != NULL) 
    {
        //check if we are at the current index
        if(linecounter == index)
        {
            fclose(file);
            adcvalue = atoi(buffer);
            if (adcvalue < 0 || adcvalue > 4095) 
            {
                printf("Error: ADC value must be between 0 and 4095.\n");
                return 0;  // Return an error value
            }
            return adcvalue;
        }
        //increase for next round
        linecounter++;
    }
    //file index ended
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
    uint16_t readADCvalue;

    readADCvalue = ADCvalue(index);

    if(readADCvalue == 0)
    {
        printf("Error: problem reading from file.\n");
        return 0;
    }
    else
    {
        return ((readADCvalue / 4095.0) * 100.0) - 50.0;
    } 
}
/*
Function that is true each time 100ms clock cycles has passed
*/
bool tasks100ms(void)
{
    //get current clock cycles since program start
    clock_t current_time = clock();
    double elapsed_time = (double)(current_time - last_time_100ms);
    if (elapsed_time >= ADC_READ_INTERVAL) 
    {
        last_time_100ms = current_time;  // Update last read time
        return true;
    }
    return false;
}

void main()
{
    double temp = 0.0;
    uint16_t index = 0;

    while (1)
    {
        if (tasks100ms)
        {
            temp = getTemperature(index);
            index++;
            printf("Temperature: %.2fC\n", temp);
        }
        if(index == 767) break;
    } 
}