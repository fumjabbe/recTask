#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define ADC_READ_INTERVAL 100 //100ms
#define MAX_BUFFER_SIZE 1024
#define DURATION_MS 120000 // Total duration 120000ms (2 minutes)
#define NUM_READINGS (DURATION_MS / ADC_READ_INTERVAL)

static clock_t last_time_100ms = 0;       // Store last time 100ms was reached
static uint16_t buffer[MAX_BUFFER_SIZE];
static uint16_t totalLinesCounter = 0;

/*
getTestADCvaluesFromFile()
Testfunction that reads a test ADC file and stores in memory
This approach was chosen to not constantly open, read and close the file each 100ms for 2 minutes or longer.
*/
bool getTestADCvaluesFromFile() 
{
    FILE *file;
    //open file
    file = fopen("temperature.txt", "r");
    if (file == NULL) 
    {
        perror("Error opening file");
        return 0;
    }

    // Read thorugh the file
    while (totalLinesCounter < MAX_BUFFER_SIZE && fscanf(file, "%d", &buffer[totalLinesCounter]) == 1) 
    {
        totalLinesCounter++;
    }

    fclose(file);
    return 1;
}
/*
Function that gets the integer at a certain index
Wraps back to start .
*/
uint16_t getADCvalue(uint16_t index)
{
    uint16_t targetIndex = (index - 1) % totalLinesCounter + 1;
    return buffer[targetIndex];
}
/*
The temperature sensor reports a temperature range of -50C to +50C and can be read every 100ms.
For example the ADC can read the following values from the sensor:

2048 (rougly 0C)
3000 (rougly 23C)
*/
double getTemperature(uint16_t index)
{
    return ((getADCvalue(index) / 4095.0) * 100.0) - 50.0;
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
    double  temperature = 0.0,
            minVal = 0.0,
            maxVal = 0.0,
            avgVal = 0.0;
    uint16_t reading = 0;

    if(getTestADCvaluesFromFile())                          //read file and store content to memory
    {                                                       //only start program if file is found
        while (1)
        {
            if (tasks100ms)
            {
                temperature = getTemperature(reading);      //Get a new reading each 100ms
                reading++;
                
                if (temperature < minVal) 
                {
                    minVal = temperature;                   //Store the lowest temperature registered in this periode
                }
                if (temperature > maxVal)
                {
                    maxVal = temperature;                   //Store the highest temperature registered in this periode
                }
                avgVal += temperature;

                if(reading >= NUM_READINGS)                 //Each 2.minute, calculate average and post to rest api
                {
                    avgVal = avgVal / NUM_READINGS;
                    
                    minVal = 0.0;
                    maxVal = 0.0;
                }
            }
        }
    } 
}