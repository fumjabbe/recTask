#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <curl/curl.h>

#define MAX_BUFFER_SIZE 1024
#define DURATION 120
#define URL "http://localhost:5000/api/temperature"
#define ALT_URL "http://localhost:5000/api/temperature/missing"

static uint16_t buffer[MAX_BUFFER_SIZE];
static uint16_t totalLinesCounter = 0;
char payloadHistory[10][512];
uint8_t historyIndex = 0;

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
    uint16_t targetIndex = index % totalLinesCounter;
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
Function to get current time.
Returns time in buffer formatted in ISO8601 format
*/
void getTime(char *buffer, size_t size) 
{
    time_t now = time(NULL);
    struct tm *tm_info = gmtime(&now);
    strftime(buffer, size, "%Y-%m-%dT%H:%M:%SZ", tm_info);
}
uint16_t postHTTP(const char *url, const char *payload)
{
    CURL *curl;
    CURLcode res;
    long responseCode = 0;

    //post the data
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    
    if (curl) 
    {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);

        // Perform the request
        res = curl_easy_perform(curl);
        
        //get response code
        if(res == CURLE_OK)
        {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);            
        }
        //error code from example
        else
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }

        // Cleanup
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    return responseCode;
}
/*
Function that formats JSON payload and post to HTTP rest api
*/
void formatJSONPostHTTP(char *startTime, char *endTime, double minTemp, double maxTemp, double avgTemp)
{
    char payload[512];
    uint16_t responseCode = 0;

    // Format the JSON payload
    snprintf(payload, sizeof(payload), 
             "{\"time\": {\"start\": \"%s\", \"end\": \"%s\"}, \"min\": %.2f, \"max\": %.2f, \"Avg\": %.2f}", 
             startTime, endTime, minTemp, maxTemp, avgTemp);

    //store last 10 payloads
    strcpy(payloadHistory[historyIndex], payload);
    (historyIndex == 10) ? (historyIndex = 0) : historyIndex++;

    responseCode = postHTTP(URL, payload);

    // Upon failure, send the last 10 stored values as JSON array
    if(responseCode == 500)
    {
        char jsonArrayPayload[1024] = "[";
        
        for (int i = 0; i < historyIndex; i++) 
        {
            strcat(jsonArrayPayload, payloadHistory[i]);
            if (i < historyIndex - 1) strcat(jsonArrayPayload, ",");
        }

        strcat(jsonArrayPayload, "]");
    }
}

/*
Check each time two minutes has passed
*/
bool task2min(void)
{
    static time_t last_call_time = 0;
    time_t current_time = time(NULL);

    if (current_time - last_call_time >= DURATION) 
    {
        last_call_time = current_time;  // Update last read time
        return true;
    }
    return false;
}

void main()
{
    double  temperature = 0.0,
            minVal = 0.0,
            maxVal = 0.0;
            
    double avgVal = 0.0;

    uint16_t reading = 0;

    char startTime[32];
    char endTime[32];
    struct timespec ts = {0, 100 * 1000000L};               // Sleep for 100ms

    if(getTestADCvaluesFromFile())                          //read file and store content to memory
    {
        system("echo recTask started");
        getTime(startTime, sizeof(startTime));              //get the time at the start of the program.
        while (1)
        {
            nanosleep(&ts, NULL);
            temperature = getTemperature(reading);          //Get a new reading
            reading++;
                
            if (temperature < minVal) 
            {
                minVal = temperature;                       //Store the lowest temperature registered in this periode
            }
            if (temperature > maxVal)
            {
                maxVal = temperature;                       //Store the highest temperature registered in this periode
            }
            avgVal += temperature;

            if(task2min())                                  //Each 2.minute, calculate average and post to HTTP REST endpoint
            {
                avgVal = avgVal / reading;
                getTime(endTime, sizeof(endTime));          //get the time each 2minute interval as endtime variable.
                formatJSONPostHTTP(startTime, endTime, minVal, maxVal, avgVal);
                 
                //reset interval variables
                reading = 0;
                minVal = 0.0;                           
                maxVal = 0.0;
                getTime(startTime, sizeof(startTime));      //get new start time for next interval
            }
        }
    } 
}