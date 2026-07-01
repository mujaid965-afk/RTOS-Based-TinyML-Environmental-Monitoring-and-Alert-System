/**
  ******************************************************************************
  * @file    network_data_params.c
  * @author  AST Embedded Analytics Research Platform
  * @date    2026-06-30T23:42:58+0530
  * @brief   AI Tool Automatic Code Generator for Embedded NN computing
  ******************************************************************************
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  */

#include "network_data_params.h"


/**  Activations Section  ****************************************************/
ai_handle g_network_activations_table[1 + 2] = {
  AI_HANDLE_PTR(AI_MAGIC_MARKER),
  AI_HANDLE_PTR(NULL),
  AI_HANDLE_PTR(AI_MAGIC_MARKER),
};




/**  Weights Section  ********************************************************/
AI_ALIGNED(32)
const ai_u64 s_network_weights_array_u64[113] = {
  0x3cdce6acbdc4498aU, 0xbe5a3f6abf23b713U, 0x3c1fa89ebe6df3d5U, 0x3f327825beb390bdU,
  0x3e17920bbec56de2U, 0xbf12a602bed30f4fU, 0x3eab9d2e3d9fd4c5U, 0xbc6124573f13acb1U,
  0x3d28cf27bcff1dafU, 0xbeb1a2d93f4eceb7U, 0x3b807660be141a4dU, 0xbcd140703fd30e8aU,
  0xbe4d0f2dbeece788U, 0xbd668f373f4eb7f3U, 0xbe98861fbf37526aU, 0xbf4a9de8be6b5606U,
  0xbe68a434bf03cf8bU, 0xbf29868bbf14b838U, 0x3c2dfb6e3f29b0f4U, 0x3eadd1e7befee56dU,
  0xbce818203f5c124bU, 0x3ee56ce5bf2481c5U, 0x3ed89959bef4111fU, 0xbd6a74acbe1252daU,
  0xbee57b58be0b9cc0U, 0x3f19b15a3d469c54U, 0x3e970085be8689f5U, 0x3f5e67b4bdeba065U,
  0x3c750e14bef314b0U, 0x3e2f18d73f9cab19U, 0xbe72a1d13f76dbc1U, 0x3e25c2f13eb105cdU,
  0x3e55fcb73e062523U, 0x3e897fbb3e38e87cU, 0x3ede2681bcd3126bU, 0x3e81b4673ed12924U,
  0xbe0273c13d139f7fU, 0x3b3ab766be090412U, 0x3da72949bc015176U, 0x3e1924573f2543f5U,
  0x3f0a8f15bd82c829U, 0x3e80bc7dbec375c6U, 0x3e8ed5fb3f28bf8dU, 0xbe50c2603ec4c3abU,
  0x3edf72bcbd81940dU, 0xbe0112543f44cabbU, 0x3f062afebe63e5ebU, 0x3ecf7c353f55d3f4U,
  0xbec39a493e8cb467U, 0xbe7f883e3f656ea9U, 0xbfc327efbedb0216U, 0x3f1ac9bc3e8d5c6dU,
  0x3dc074133e89c334U, 0x3f48efc3bed8a165U, 0xbe09a9ecbf376a7aU, 0xbee97cc9bf872644U,
  0xbec0a2863f3925acU, 0xbeb737913dcc9759U, 0xbf0afeb53daf534bU, 0x3e5af31abd67dfb0U,
  0xbf0507573f6fb394U, 0x3f2a173e3dac4139U, 0xbe8152e5beb20e19U, 0xbd87acaabc70bf4eU,
  0x3f40778abe79448fU, 0xbd77bffd3e3a2369U, 0x3f17befe3f2ac510U, 0xbf02409b3f39dc57U,
  0x3f2434d1bab309f7U, 0xbcc2ff9d3f2ee369U, 0xbdfef8f43f08747cU, 0x3ea847243f1f88f3U,
  0x3f68d970be8b752dU, 0x3e14b6bc3da04aceU, 0x3f37658d3f003971U, 0x3e6d9f303d29824cU,
  0x3f5e0d71bf31499dU, 0xbe9f48633f007a06U, 0xbb1fcc333e47ffcaU, 0x3f25f9a43f481d12U,
  0x3f061f713da83226U, 0xbead9f473cd6f6d9U, 0xbe05bfefbf0a3f83U, 0xbe2512913e863a25U,
  0x3f4b5405bf0bfce6U, 0x3ea2fed33ed8d803U, 0x3eee72b63f1df286U, 0x3f4546213d49b060U,
  0x3f1676d1be440c65U, 0x3ecc65b9bde0df61U, 0x3f3b07f73e107844U, 0xbe5924473f223fdeU,
  0x3f3f20b3be92cb59U, 0x3e5118a23f836ee6U, 0x3e876b5abde3f4fcU, 0x3e6d04563ef62519U,
  0x3f415a53bea23a79U, 0x3e8e0f123e6f1713U, 0x3f6e9faa3efa48d5U, 0xbc7d6ddd3f07b9c8U,
  0x3f357b56becdb443U, 0xbeafb5e83f551007U, 0x3ee045d6be708db7U, 0x3e39735b3ed631bdU,
  0x3ec54d213e593b47U, 0x3dd465d23e0040c7U, 0x3da628383e43dccfU, 0x3d51406c3e6432a6U,
  0xbfb5830d3fd64ec3U, 0x3fb4372dbf6e2255U, 0x3f46e5353fab61a7U, 0x3fbd4c8c3fc0dd11U,
  0xbdbdf8f7U,
};


ai_handle g_network_weights_table[1 + 2] = {
  AI_HANDLE_PTR(AI_MAGIC_MARKER),
  AI_HANDLE_PTR(s_network_weights_array_u64),
  AI_HANDLE_PTR(AI_MAGIC_MARKER),
};

