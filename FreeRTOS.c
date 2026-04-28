#include <Arduino_FreeRTOS.h>  // provides the freertos api for arduino
#include <semphr.h>  // provides semaphore api's which includes mutexes

static SemaphoreHandle_t gSerialMutex;

static void TaskA(void* pv);
static void TaskB(void* pv);

void setup() {
	Serial.begin(115200);// initializes the serial console with a baud rate of 11520

	gSerialMutex = xSemaphoreCreateMutex();// creates a mutex assigns it to gSerialMutex

	xTaskCreate(TaskA, "A", 192, nullptr, 1, nullptr);//creates a new task with a stack size of 192 bytes, a priority of 1
	xTaskCreate(TaskB, "B", 192, nullptr, 1, nullptr);//creates a new task with a stack size of 192 bytes, a priority of 1
}

void loop() {}// empty, as the tasks are running concurrently

static void SafePrint(const char* msg) {
	// Wait forever for the mutex
	xSemaphoreTake(gSerialMutex, portMAX_DELAY);//Waits forever for the mutex to become available, then takes ownership of it.
	Serial.println(msg);
	xSemaphoreGive(gSerialMutex);//Releases the mutex, allowing other tasks to take ownership.
}

static void TaskA(void* pv) {
	(void)pv;
	for (;;) {
		SafePrint("TaskA: hello");
		vTaskDelay(pdMS_TO_TICKS(400));// delays the task for 400ms
	}
}

static void TaskB(void* pv) {
	(void)pv;
	for (;;) {
		SafePrint("TaskB: world");
		vTaskDelay(pdMS_TO_TICKS(700));
	}
}
