//program to read temperature from dht11 temp & humidity sensor module and intensity of light 
//using mutex to protect spu resources for both tasks 
//blue led will light up as temp reading is done 
//yellow led will light up as light intensity is being done 
//serial monitor will display temp and light intensity reading 


//NEED TO DEBUG CODE 
//FIRSTLY, WITH LCD TOO MANY PERIPHERALS WAS DRAWNING TOO MUCH POWER 
//SEMAPHORE NOT WORKING WAY I WANT TO. going temp->light->light->temp 
//fix ldr. super supre inconsistent readings 

//peritinent libraries 
#include <DHT11.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#define DHTPIN 2
//defining pin name  
#define blueLED 12 
#define yellowLED 11 
#define LDRpin A0

DHT11 dht11(2);

//create a mutex 
SemaphoreHandle_t mutex; 

//creating task fot tempertaure reading 
void temp(void *pvParameters){
  while(1){
    if(xSemaphoreTake(mutex, portMAX_DELAY)==pdTRUE){
      Serial.println("Mutex taken for temp reading");
      int temperature=0; 
      int humidity=0; 

      int result = dht11.readTemperatureHumidity(temperature, humidity); 
      if(result==0){ //checking if result is ready to be printed.
        digitalWrite(blueLED, HIGH); 
        Serial.print("Temperature: "); 
        Serial.print(temperature);
        Serial.print("C \t Humidity: "); 
        Serial.println(humidity); 
        vTaskDelay(500/portTICK_PERIOD_MS);

        digitalWrite(blueLED, LOW);
      }
      else{ //if result not ready 
        Serial.println("Error");
        vTaskDelay(1000/portTICK_PERIOD_MS);
      }

      if(xSemaphoreGive(mutex)==pdTRUE){
        Serial.println("Mutex GIVEN");
      }
      else{
        Serial.println("Mutex NOT given");
      }
      vTaskDelay(10/portTICK_PERIOD_MS);
    }
    else{ //semaphore wasnt taken
      Serial.println("semaphore NOT taken"); 
    }
  }
}

void light(void *pvParameters){
  while(1){
    if(xSemaphoreTake(mutex, portMAX_DELAY)==pdTRUE){
      Serial.println("Mutex TAKEN for light");
      digitalWrite(yellowLED, HIGH);
      int ldrStatus = analogRead(LDRpin);
      Serial.print("Light Intensity: ");
      Serial.println(ldrStatus); 
      vTaskDelay(500/portTICK_PERIOD_MS);
      digitalWrite(yellowLED, LOW); 
      
      if(xSemaphoreGive(mutex)==pdTRUE){
        Serial.println("Mutex GIVEN");
      }
      else{
        Serial.println("Mutex NOT given");
      }
      vTaskDelay(10/portTICK_PERIOD_MS); //block task for a while after giving it 
    }
    else{
      Serial.println("Mutex NOT taken");
    }
  }
}

void setup() {
  Serial.begin(9600); 

  pinMode(blueLED, OUTPUT); 
  pinMode(yellowLED, OUTPUT); 
  pinMode(LDRpin, INPUT); 

  mutex=xSemaphoreCreateMutex();
  if(mutex==NULL){
    Serial.println("Unable to create binary semaphore");
  }

  xTaskCreate(
    //6 parameter
    temp, //function name
    "temp_read", //task name
    256, //stack size
    NULL, //task parameters
    1, //task priority
    NULL//task handle
  );

  xTaskCreate(
    //6 parameter
    light, //function name
    "light_read", //task name
    256, //stack size
    NULL, //task parameters
    1, //task priority
    NULL//task handle
  );
}

void loop() {}
