#include "ai_engine.h"
#include "tinyml_engine.h"
#include <string.h>

void TinyML_RunInference(const SensorData_t *sensor,
                         InferenceData_t *result)
{
    AI_Input_t input;
    AI_Output_t output;

    input.temperature = sensor->temperature;
    input.pressure    = sensor->pressure;
    input.gas         = (float)sensor->gas;
    input.vibration   = (float)sensor->vibration;

    if (AI_RunInference(&input, &output))
    {
        result->anomaly = output.anomaly;
        result->probability = output.probability;

        if (output.anomaly)
        {
            strcpy(result->reason, "AI ANOMALY");
        }
        else
        {
            strcpy(result->reason, "NORMAL");
        }
    }
    else
    {
        result->anomaly = 0;
        result->probability = 0.0f;
        strcpy(result->reason, "AI ERROR");
    }
}
