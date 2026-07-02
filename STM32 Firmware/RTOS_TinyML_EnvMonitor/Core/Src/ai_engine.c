/*
 * ai_engine.c
 *
 *  Created on: Jun 30, 2026
 *      Author: Mujaid
 */

#include "ai_engine.h"

#include "network.h"
#include "network_data.h"

#include <stdio.h>

static ai_handle network = AI_HANDLE_NULL;

static ai_buffer *ai_input;
static ai_buffer *ai_output;

AI_ALIGNED(4)
static ai_u8 activations[AI_NETWORK_DATA_ACTIVATIONS_SIZE];

static float input_data[4];
static float output_data[1];

static const float mean[4] = {
    33.63903931f,
    1002.49206f,
    1285.81100f,
    0.496f
};

static const float scale[4] = {
    6.62266954f,
    7.17027631f,
    694.137749f,
    0.499984f
};


bool AI_Init(void)
{
    ai_error err;

    const ai_handle acts[] =
    {
        activations
    };

    err = ai_network_create_and_init(
            &network,
            acts,
            NULL);

    if (err.type != AI_ERROR_NONE)
    {
        printf("\r\n===== AI INIT FAILED =====\r\n");
        printf("Error Type : %d\r\n", err.type);
        printf("Error Code : %d\r\n", err.code);
        printf("==========================\r\n");
        return false;
    }

    ai_input = ai_network_inputs_get(network, NULL);
    ai_output = ai_network_outputs_get(network, NULL);

    printf("AI network created successfully.\r\n");

    return true;
}

static void NormalizeInput(const AI_Input_t *in)
{
    input_data[0] = (in->temperature - mean[0]) / scale[0];
    input_data[1] = (in->pressure    - mean[1]) / scale[1];
    input_data[2] = (in->gas         - mean[2]) / scale[2];
    input_data[3] = (in->vibration   - mean[3]) / scale[3];
}

bool AI_RunInference(AI_Input_t *input, AI_Output_t *output)
{
    if (network == AI_HANDLE_NULL)
    {
        return false;
    }

    /* Normalize sensor values */
    NormalizeInput(input);

    /* Assign input/output buffers */
    ai_input[0].data = AI_HANDLE_PTR(input_data);
    ai_output[0].data = AI_HANDLE_PTR(output_data);

    /* Run inference */
    if (ai_network_run(network, ai_input, ai_output) != 1)
    {
        return false;
    }

    /* Read prediction */
    output->probability = output_data[0];

    printf("Raw AI Output = %.8f\r\n", output_data[0]);

    /* Binary classification */
    output->anomaly = (output_data[0] > 0.5f);

    return true;
}




