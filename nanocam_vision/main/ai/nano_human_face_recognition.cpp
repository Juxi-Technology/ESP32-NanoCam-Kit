#include "who_human_face_recognition.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

extern QueueHandle_t xQueueEvent;

void face_recognition_send_event(recognizer_state_t state)
{
    if (xQueueEvent) {
        xQueueSend(xQueueEvent, &state, 0);
    }
}
