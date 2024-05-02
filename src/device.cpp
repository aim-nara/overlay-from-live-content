/*
 * SPDX-FileCopyrightText: Copyright (c) DELTACAST.TV. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at * * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "device.hpp"
#include "rx_stream.hpp"

#include <string>
#include <thread>

#include "VideoMasterHD_Keyer.h"

#include "VideoMasterAPIHelper/handle_manager.hpp"

#include "VideoMasterAPIHelper/VideoInformation/dv.hpp"
#include "VideoMasterAPIHelper/VideoInformation/sdi.hpp"

using Deltacast::Helper::ApiSuccess;

const std::unordered_map<uint32_t, VHD_CORE_BOARDPROPERTY> id_to_rx_channel_type_prop = {
   { 0, VHD_CORE_BP_RX0_TYPE }, { 1, VHD_CORE_BP_RX1_TYPE },   { 2, VHD_CORE_BP_RX2_TYPE },
   { 3, VHD_CORE_BP_RX3_TYPE }, { 4, VHD_CORE_BP_RX4_TYPE },   { 5, VHD_CORE_BP_RX5_TYPE },
   { 6, VHD_CORE_BP_RX6_TYPE }, { 7, VHD_CORE_BP_RX7_TYPE },   { 8, VHD_CORE_BP_RX8_TYPE },
   { 9, VHD_CORE_BP_RX9_TYPE }, { 10, VHD_CORE_BP_RX10_TYPE }, { 11, VHD_CORE_BP_RX11_TYPE }
};

const std::unordered_map<uint32_t, VHD_CORE_BOARDPROPERTY> id_to_tx_channel_type_prop = {
   { 0, VHD_CORE_BP_TX0_TYPE }, { 1, VHD_CORE_BP_TX1_TYPE },   { 2, VHD_CORE_BP_TX2_TYPE },
   { 3, VHD_CORE_BP_TX3_TYPE }, { 4, VHD_CORE_BP_TX4_TYPE },   { 5, VHD_CORE_BP_TX5_TYPE },
   { 6, VHD_CORE_BP_TX6_TYPE }, { 7, VHD_CORE_BP_TX7_TYPE },   { 8, VHD_CORE_BP_TX8_TYPE },
   { 9, VHD_CORE_BP_TX9_TYPE }, { 10, VHD_CORE_BP_TX10_TYPE }, { 11, VHD_CORE_BP_TX11_TYPE }
};

const std::unordered_map<uint32_t, VHD_CORE_BOARDPROPERTY> id_to_passive_loopback_prop = {
   { 0, VHD_CORE_BP_BYPASS_RELAY_0 },
   { 1, VHD_CORE_BP_BYPASS_RELAY_1 },
   { 2, VHD_CORE_BP_BYPASS_RELAY_2 },
   { 3, VHD_CORE_BP_BYPASS_RELAY_3 }
};
const std::unordered_map<uint32_t, VHD_CORE_BOARDPROPERTY> id_to_active_loopback_prop = {
   { 0, VHD_CORE_BP_ACTIVE_LOOPBACK_0 }
};
const std::unordered_map<uint32_t, VHD_CORE_BOARDPROPERTY> id_to_firmware_loopback_prop = {
   { 0, VHD_CORE_BP_FIRMWARE_LOOPBACK_0 },
   { 1, VHD_CORE_BP_FIRMWARE_LOOPBACK_1 }
};

const std::unordered_map<uint32_t, VHD_CORE_BOARDPROPERTY> id_to_rx_status_prop = {
   { 0, VHD_CORE_BP_RX0_STATUS }, { 1, VHD_CORE_BP_RX1_STATUS },   { 2, VHD_CORE_BP_RX2_STATUS },
   { 3, VHD_CORE_BP_RX3_STATUS }, { 4, VHD_CORE_BP_RX4_STATUS },   { 5, VHD_CORE_BP_RX5_STATUS },
   { 6, VHD_CORE_BP_RX6_STATUS }, { 7, VHD_CORE_BP_RX7_STATUS },   { 8, VHD_CORE_BP_RX8_STATUS },
   { 9, VHD_CORE_BP_RX9_STATUS }, { 10, VHD_CORE_BP_RX10_STATUS }, { 11, VHD_CORE_BP_RX11_STATUS },
};

const std::unordered_map<uint32_t, VHD_KEYERINPUT> id_to_keyer_rx_input = {
   { 0, VHD_KINPUT_RX0 },
   { 1, VHD_KINPUT_RX1 },
   { 2, VHD_KINPUT_RX2 },
   { 3, VHD_KINPUT_RX3 },
};
const std::unordered_map<uint32_t, VHD_KEYERINPUT> id_to_keyer_tx_input = {
   { 0, VHD_KINPUT_TX0 },
   { 1, VHD_KINPUT_TX1 },
   { 2, VHD_KINPUT_TX2 },
   { 3, VHD_KINPUT_TX3 },
};
const std::unordered_map<uint32_t, VHD_KEYEROUTPUT> id_to_keyer_rx_output = {
   { 0, VHD_KOUTPUT_RX0 },
   { 1, VHD_KOUTPUT_RX1 },
   { 2, VHD_KOUTPUT_RX2 },
   { 3, VHD_KOUTPUT_RX3 },
};
const std::unordered_map<uint32_t, VHD_KEYER_BOARDPROPERTY> id_to_keyer_video_output_prop = {
   { 0, VHD_KEYER_BP_VIDEOOUTPUT_TX_0 },
   { 1, VHD_KEYER_BP_VIDEOOUTPUT_TX_1 },
   { 2, VHD_KEYER_BP_VIDEOOUTPUT_TX_2 },
   { 3, VHD_KEYER_BP_VIDEOOUTPUT_TX_3 },
};
const std::unordered_map<uint32_t, VHD_KEYER_BOARDPROPERTY> id_to_keyer_anc_output_prop = {
   { 0, VHD_KEYER_BP_ANCOUTPUT_TX_0 },
   { 1, VHD_KEYER_BP_ANCOUTPUT_TX_1 },
   { 2, VHD_KEYER_BP_ANCOUTPUT_TX_2 },
   { 3, VHD_KEYER_BP_ANCOUTPUT_TX_3 },
};

Deltacast::Device::~Device()
{
   ULONG number_of_rx_channels = 0;
   VHD_GetBoardProperty(*handle(), VHD_CORE_BP_NB_RXCHANNELS, &number_of_rx_channels);

   for (auto i = 0; i < number_of_rx_channels; i++)
      enable_loopback(i);
}

std::unique_ptr<Deltacast::Device> Deltacast::Device::create(int device_index)
{
   auto device_handle = Helper::get_board_handle(device_index);
   if (!device_handle)
      return nullptr;

   return std::unique_ptr<Device>(new Device(device_index, std::move(device_handle)));
}

bool Deltacast::Device::suitable()
{
   ULONG number_of_on_board_keyers = 0;
   VHD_GetBoardCapability(*handle(), VHD_KEYER_BOARD_CAP_KEYER, &number_of_on_board_keyers);

   return (number_of_on_board_keyers > 0);
}

bool Deltacast::Device::set_loopback_state(int index, bool enabled)
{
   ULONG has_passive_loopback = FALSE;
   ULONG has_active_loopback = FALSE;
   ULONG has_firmware_loopback = FALSE;

   VHD_GetBoardCapability(*handle(), VHD_CORE_BOARD_CAP_PASSIVE_LOOPBACK, &has_passive_loopback);
   VHD_GetBoardCapability(*handle(), VHD_CORE_BOARD_CAP_ACTIVE_LOOPBACK, &has_active_loopback);
   VHD_GetBoardCapability(*handle(), VHD_CORE_BOARD_CAP_FIRMWARE_LOOPBACK, &has_firmware_loopback);

   if (has_firmware_loopback &&
       id_to_firmware_loopback_prop.find(index) != id_to_firmware_loopback_prop.end())
      return ApiSuccess{ VHD_SetBoardProperty(*handle(), id_to_firmware_loopback_prop.at(index),
                                              enabled) };
   else if (has_active_loopback &&
            id_to_active_loopback_prop.find(index) != id_to_active_loopback_prop.end())
      return ApiSuccess{ VHD_SetBoardProperty(*handle(), id_to_active_loopback_prop.at(index),
                                              enabled) };
   else if (has_passive_loopback &&
            id_to_passive_loopback_prop.find(index) != id_to_passive_loopback_prop.end())
      return ApiSuccess{ VHD_SetBoardProperty(*handle(), id_to_passive_loopback_prop.at(index),
                                              enabled) };
   return true;
}

void Deltacast::Device::enable_loopback(int index)
{
   set_loopback_state(index, true);
}

void Deltacast::Device::disable_loopback(int index)
{
   set_loopback_state(index, false);
}

bool Deltacast::Device::wait_for_incoming_signal(int                     rx_index,
                                                 const std::atomic_bool& stop_is_requested)
{
   if (id_to_rx_status_prop.find(rx_index) == id_to_rx_status_prop.end())
      return false;

   while (!stop_is_requested.load())
   {
      ULONG status = VHD_CORE_RXSTS_UNLOCKED;
      auto  api_success = ApiSuccess{
         VHD_GetBoardProperty(*handle(), id_to_rx_status_prop.at(rx_index), &status)
      };
      if (api_success && !(status & VHD_CORE_RXSTS_UNLOCKED))
         return true;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
   }

   return false;
}

bool Deltacast::Device::wait_sync_locked(const std::atomic_bool& stop_is_requested, std::unique_ptr<Helper::VideoInformation>& video_info)
{
    auto genlock_status_prop_optional = video_info->get_sync_status_properties();
    if (!genlock_status_prop_optional.has_value())
      return true;

   while (!stop_is_requested.load())
   {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));

      ULONG status = VHD_SDI_GNLKSTS_NOREF | VHD_SDI_GNLKSTS_UNLOCKED;
      auto  api_success = ApiSuccess{ VHD_GetBoardProperty(*handle(), genlock_status_prop_optional.value(), &status) };
      if (api_success && !(status & VHD_SDI_GNLKSTS_NOREF) && !(status & VHD_SDI_GNLKSTS_UNLOCKED))
         return true;
   }

   return false;
}

bool Deltacast::Device::configure_sync(int genlock_source_rx_index,
                                          std::unique_ptr<Helper::VideoInformation>& video_info)
{
   if (video_info->configure_sync(handle(), genlock_source_rx_index))
      return true;
   return false;
}

std::unique_ptr<Deltacast::Helper::VideoInformation>
Deltacast::Device::factory_create_video_information_for_channel(int index, Direction direction)
{
   std::unique_ptr<Helper::VideoInformation> _video_information = {};

   // identify the channel type to know which VideoInformation implementation to use
   auto channel_type_optional = get_channel_type(index, direction);
   if (!channel_type_optional.has_value())
      return {};

   ULONG channel_type = channel_type_optional.value();

   switch (channel_type)
   {
   case VHD_CHNTYPE_HDSDI:
   case VHD_CHNTYPE_3GSDI:
   case VHD_CHNTYPE_12GSDI:
      _video_information = std::make_unique<Helper::SdiVideoInformation>();
      break;
   case VHD_CHNTYPE_HDMI:
   case VHD_CHNTYPE_DISPLAYPORT:
      _video_information = std::make_unique<Helper::DvVideoInformation>();
      break;
   default:
      break;
   }

   return _video_information;
}

bool Deltacast::Device::configure_keyer(int rx_index, int tx_index, std::unique_ptr<Helper::VideoInformation>& video_info)
{
   if ((id_to_keyer_rx_input.find(rx_index) == id_to_keyer_rx_input.end()) ||
       (id_to_keyer_tx_input.find(tx_index) == id_to_keyer_tx_input.end()) ||
       (id_to_keyer_rx_output.find(rx_index) == id_to_keyer_rx_output.end()) ||
       (id_to_keyer_video_output_prop.find(tx_index) == id_to_keyer_video_output_prop.end()) ||
       (id_to_keyer_anc_output_prop.find(tx_index) == id_to_keyer_anc_output_prop.end()))
      return false;

   // get the right keyer properties according to board and input output connectors
   auto keyer_props = video_info->get_keyer_properties(handle());
   if ((keyer_props.find(VHD_KEYER_BP_INPUT_A) == keyer_props.end()) ||
       (keyer_props.find(VHD_KEYER_BP_INPUT_B) == keyer_props.end()) ||
       (keyer_props.find(VHD_KEYER_BP_INPUT_K) == keyer_props.end()) ||
       (keyer_props.find(VHD_KEYER_BP_ALPHACLIP_MIN) == keyer_props.end()) ||
       (keyer_props.find(VHD_KEYER_BP_ALPHACLIP_MAX) == keyer_props.end()) ||
       (keyer_props.find(VHD_KEYER_BP_ALPHABLEND_FACTOR) == keyer_props.end()) ||
       (keyer_props.find(VHD_KEYER_BP_ALPHABLEND_FACTOR) == keyer_props.end()) ||
       (keyer_props.find(VHD_KEYER_BP_ENABLE) == keyer_props.end())) {
      std::cout << "ERROR: Missing keyer properties" << std::endl;
      return false;
       }

   ApiSuccess api_success;
   if (!(api_success = ApiSuccess{ VHD_SetBoardProperty(*handle(),
                                                        keyer_props.at(VHD_KEYER_BP_INPUT_A),
                                                        id_to_keyer_rx_input.at(rx_index)) }) ||
       !(api_success = ApiSuccess{ VHD_SetBoardProperty(*handle(),
                                                        keyer_props.at(VHD_KEYER_BP_INPUT_B),
                                                        id_to_keyer_tx_input.at(tx_index)) }) ||
       !(api_success = ApiSuccess{ VHD_SetBoardProperty(*handle(),
                                                        keyer_props.at(VHD_KEYER_BP_INPUT_K),
                                                        id_to_keyer_tx_input.at(tx_index)) }) ||
       !(api_success = ApiSuccess{ VHD_SetBoardProperty(
             *handle(), id_to_keyer_video_output_prop.at(tx_index), VHD_KOUTPUT_KEYER) }) ||
       !(api_success = ApiSuccess{ VHD_SetBoardProperty(*handle(), keyer_props.at(VHD_KEYER_BP_ALPHACLIP_MIN),
                                                        0) }) ||
       !(api_success = ApiSuccess{ VHD_SetBoardProperty(*handle(), keyer_props.at(VHD_KEYER_BP_ALPHACLIP_MAX),
                                                        1020) }) ||
       !(api_success = ApiSuccess{ VHD_SetBoardProperty(*handle(), keyer_props.at(VHD_KEYER_BP_ALPHABLEND_FACTOR),
                                                        1023) }) ||
       !(api_success = ApiSuccess{ VHD_SetBoardProperty(*handle(), keyer_props.at(VHD_KEYER_BP_ENABLE), TRUE) }))
   {
      std::cout << "ERROR: Cannot configure keyer (" << api_success << ")" << std::endl;
      return false;
   }

   return true;
}

std::optional<ULONG> Deltacast::Device::get_channel_type(int                          index,
                                                         Deltacast::Device::Direction direction)
{
   ULONG       _channel_type;
   const auto& id_to_channel_type_prop = direction == Direction::RX ? id_to_rx_channel_type_prop
                                                                    : id_to_tx_channel_type_prop;

   ApiSuccess api_success;
   api_success = VHD_GetBoardProperty(*handle(), id_to_channel_type_prop.at(index),
                                      (ULONG*)&_channel_type);

   if (!api_success)
   {
      std::cout << "ERROR: Cannot get channel type (" << api_success << ")" << std::endl;
      return {};
   }

   return _channel_type;
}

namespace Deltacast
{
std::ostream& operator<<(std::ostream& os, const Device& device)
{
   ULONG firmware_version, number_of_rx_channels, number_of_tx_channels;
   const char* driver_version;

   driver_version = VHD_GetDriverStringVersion(**(device._device_handle));
   VHD_GetBoardProperty(**(device._device_handle), VHD_CORE_BP_FIRMWARE_VERSION, &firmware_version);
   VHD_GetBoardProperty(**(device._device_handle), VHD_CORE_BP_NB_RXCHANNELS,
                        &number_of_rx_channels);
   VHD_GetBoardProperty(**(device._device_handle), VHD_CORE_BP_NB_TXCHANNELS,
                        &number_of_tx_channels);

   char pcie_id_string[64];
   VHD_GetPCIeIdentificationString(device._device_index, pcie_id_string);

   os << "  Board " << device._device_index << ":  [ " << VHD_GetBoardModel(device._device_index)
      << " ]"
      << "\n";
   os << "    - PCIe Id string: " << pcie_id_string << "\n";
   os << "    - Driver v" << driver_version << "\n";
   os << "    - Board fpga firmware v" << ((firmware_version & 0xFF000000) >> 24) << "."
      << ((firmware_version & 0x00FF0000) >> 16) << "." << ((firmware_version & 0x0000FF00) >> 8)
      << "." << ((firmware_version & 0x000000FF) >> 0) << "\n";

   os << std::dec;

   os << "    - " << number_of_rx_channels << " In / " << number_of_tx_channels << " Out"
      << "\n";

   return os;
}
}  // namespace Deltacast