// class header include
#include "displaydevice/windows/winapilayer.h"

// system includes
#include <boost/algorithm/string.hpp>
#include <boost/scope/scope_exit.hpp>
#include <boost/uuid/name_generator_sha1.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <cstdint>
#include <iomanip>

// local includes
#include "displaydevice/logging.h"

// Windows includes after "windows.h"
#include <SetupApi.h>

namespace display_device {
  namespace {
    /** @brief Dumps the result of @see query_display_config into a string */
    std::string
    dump_path(const DISPLAYCONFIG_PATH_INFO &info) {
      std::ostringstream output;
      std::ios state(nullptr);
      state.copyfmt(output);

      // clang-format off
      output << "sourceInfo:" << std::endl;
      output << "    adapterId: [" << info.sourceInfo.adapterId.HighPart << ", " << info.sourceInfo.adapterId.LowPart << "]" << std::endl;
      output << "    id: " << info.sourceInfo.id << std::endl;
      output << "        cloneGroupId: " << info.sourceInfo.cloneGroupId << std::endl;
      output << "        sourceModeInfoIdx: " << info.sourceInfo.sourceModeInfoIdx << std::endl;
      output << "        modeInfoIdx: " << info.sourceInfo.modeInfoIdx << std::endl;
      output << "    statusFlags: 0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << info.sourceInfo.statusFlags << std::endl;
      output.copyfmt(state);
      output << "targetInfo:" << std::endl;
      output << "    adapterId: [" << info.targetInfo.adapterId.HighPart << ", " << info.targetInfo.adapterId.LowPart << "]" << std::endl;
      output << "    id: " << info.targetInfo.id << std::endl;
      output << "        desktopModeInfoIdx: " << info.targetInfo.desktopModeInfoIdx << std::endl;
      output << "        targetModeInfoIdx: " << info.targetInfo.targetModeInfoIdx << std::endl;
      output << "        modeInfoIdx: " << info.targetInfo.modeInfoIdx << std::endl;
      output << "    outputTechnology:  0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << info.targetInfo.outputTechnology << std::endl;
      output << "    rotation: 0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << info.targetInfo.rotation << std::endl;
      output << "    scaling: 0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << info.targetInfo.scaling << std::endl;
      output.copyfmt(state);
      output << "    refreshRate: " << info.targetInfo.refreshRate.Numerator << "/" << info.targetInfo.refreshRate.Denominator << std::endl;
      output << "    scanLineOrdering: 0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << info.targetInfo.scanLineOrdering << std::endl;
      output << "    targetAvailable: 0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << info.targetInfo.targetAvailable << std::endl;
      output << "    statusFlags: 0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << info.targetInfo.statusFlags << std::endl;
      output << "flags: 0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << info.flags;
      // clang-format on

      return output.str();
    }

    /** @brief Dumps the result of @see query_display_config into a string */
    std::string
    dump_mode(const DISPLAYCONFIG_MODE_INFO &info) {
      std::stringstream output;
      std::ios state(nullptr);
      state.copyfmt(output);

      if (info.infoType == DISPLAYCONFIG_MODE_INFO_TYPE_SOURCE) {
        // clang-format off
        output << "width: " << info.sourceMode.width << std::endl;
        output << "height: " << info.sourceMode.height << std::endl;
        output << "pixelFormat: " << info.sourceMode.pixelFormat << std::endl;
        output << "position: [" << info.sourceMode.position.x << ", " << info.sourceMode.position.y << "]";
        // clang-format on
      }
      else if (info.infoType == DISPLAYCONFIG_MODE_INFO_TYPE_TARGET) {
        // clang-format off
        output << "pixelRate: " << info.targetMode.targetVideoSignalInfo.pixelRate << std::endl;
        output << "hSyncFreq: " << info.targetMode.targetVideoSignalInfo.hSyncFreq.Numerator << "/" << info.targetMode.targetVideoSignalInfo.hSyncFreq.Denominator << std::endl;
        output << "vSyncFreq: " << info.targetMode.targetVideoSignalInfo.vSyncFreq.Numerator << "/" << info.targetMode.targetVideoSignalInfo.vSyncFreq.Denominator << std::endl;
        output << "activeSize: [" << info.targetMode.targetVideoSignalInfo.activeSize.cx << ", " << info.targetMode.targetVideoSignalInfo.activeSize.cy << "]" << std::endl;
        output << "totalSize: [" << info.targetMode.targetVideoSignalInfo.totalSize.cx << ", " << info.targetMode.targetVideoSignalInfo.totalSize.cy << "]" << std::endl;
        output << "videoStandard: " << info.targetMode.targetVideoSignalInfo.videoStandard << std::endl;
        output << "scanLineOrdering: " << info.targetMode.targetVideoSignalInfo.scanLineOrdering;
        // clang-format on
      }
      else if (info.infoType == DISPLAYCONFIG_MODE_INFO_TYPE_DESKTOP_IMAGE) {
        // TODO: One day MinGW will add updated struct definition and the following code can be enabled
        // clang-format off
        // output << "PathSourceSize: [" << info.desktopImageInfo.PathSourceSize.x << ", " << info.desktopImageInfo.PathSourceSize.y << "]" << std::endl;
        // output << "DesktopImageRegion: [" << info.desktopImageInfo.DesktopImageRegion.bottom << ", " << info.desktopImageInfo.DesktopImageRegion.left << ", " << info.desktopImageInfo.DesktopImageRegion.right << ", " << info.desktopImageInfo.DesktopImageRegion.top << "]" << std::endl;
        // output << "DesktopImageClip: [" << info.desktopImageInfo.DesktopImageClip.bottom << ", " << info.desktopImageInfo.DesktopImageClip.left << ", " << info.desktopImageInfo.DesktopImageClip.right << ", " << info.desktopImageInfo.DesktopImageClip.top << "]";
        // clang-format on
        output << "NOT SUPPORTED BY COMPILER YET...";
      }
      else {
        output << "NOT IMPLEMENTED YET...";
      }

      return output.str();
    }

    /** @brief Dumps the result of @see query_display_config into a string */
    std::string
    dump_paths_and_modes(const std::vector<DISPLAYCONFIG_PATH_INFO> &paths,
      const std::vector<DISPLAYCONFIG_MODE_INFO> &modes) {
      std::ostringstream output;

      output << std::endl
             << "Got " << paths.size() << " path(s):";
      bool path_dumped { false };
      for (auto i { 0u }; i < paths.size(); ++i) {
        output << std::endl
               << "----------------------------------------[index: " << i << "]" << std::endl;

        output << dump_path(paths[i]);
        path_dumped = true;
      }

      if (path_dumped) {
        output << std::endl
               << std::endl;
      }

      output << "Got " << modes.size() << " mode(s):";
      for (auto i { 0u }; i < modes.size(); ++i) {
        output << std::endl
               << "----------------------------------------[index: " << i << "]" << std::endl;

        output << dump_mode(modes[i]);
      }

      return output.str();
    }

    /**
     * @see get_monitor_device_path description for more information as this
     *      function is identical except that it returns wide-string instead
     *      of a normal one.
     */
    std::wstring
    get_monitor_device_path_wstr(const WinApiLayerInterface &w_api, const DISPLAYCONFIG_PATH_INFO &path) {
      DISPLAYCONFIG_TARGET_DEVICE_NAME target_name = {};
      target_name.header.adapterId = path.targetInfo.adapterId;
      target_name.header.id = path.targetInfo.id;
      target_name.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
      target_name.header.size = sizeof(target_name);

      LONG result { DisplayConfigGetDeviceInfo(&target_name.header) };
      if (result != ERROR_SUCCESS) {
        DD_LOG(error) << w_api.get_error_string(result) << " failed to get target device name!";
        return {};
      }

      return std::wstring { target_name.monitorDevicePath };
    }

    /**
     * @brief Helper method for dealing with SetupAPI.
     * @returns True if device interface path was retrieved and is non-empty, false otherwise.
     * @see get_device_id implementation for more context regarding this madness.
     */
    bool
    get_device_interface_detail(const WinApiLayerInterface &w_api, HDEVINFO dev_info_handle, SP_DEVICE_INTERFACE_DATA &dev_interface_data, std::wstring &dev_interface_path, SP_DEVINFO_DATA &dev_info_data) {
      DWORD required_size_in_bytes { 0 };
      if (SetupDiGetDeviceInterfaceDetailW(dev_info_handle, &dev_interface_data, nullptr, 0, &required_size_in_bytes, nullptr)) {
        DD_LOG(error) << "\"SetupDiGetDeviceInterfaceDetailW\" did not fail, what?!";
        return false;
      }
      else if (required_size_in_bytes <= 0) {
        DD_LOG(error) << w_api.get_error_string(static_cast<LONG>(GetLastError())) << " \"SetupDiGetDeviceInterfaceDetailW\" failed while getting size.";
        return false;
      }

      std::vector<std::uint8_t> buffer;
      buffer.resize(required_size_in_bytes);

      // This part is just EVIL!
      auto detail_data { reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA_W *>(buffer.data()) };
      detail_data->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);

      if (!SetupDiGetDeviceInterfaceDetailW(dev_info_handle, &dev_interface_data, detail_data, required_size_in_bytes, nullptr, &dev_info_data)) {
        DD_LOG(error) << w_api.get_error_string(static_cast<LONG>(GetLastError())) << " \"SetupDiGetDeviceInterfaceDetailW\" failed.";
        return false;
      }

      dev_interface_path = std::wstring { detail_data->DevicePath };
      return !dev_interface_path.empty();
    }

    /**
     * @brief Helper method for dealing with SetupAPI.
     * @returns True if instance id was retrieved and is non-empty, false otherwise.
     * @see get_device_id implementation for more context regarding this madness.
     */
    bool
    get_device_instance_id(const WinApiLayerInterface &w_api, HDEVINFO dev_info_handle, SP_DEVINFO_DATA &dev_info_data, std::wstring &instance_id) {
      DWORD required_size_in_characters { 0 };
      if (SetupDiGetDeviceInstanceIdW(dev_info_handle, &dev_info_data, nullptr, 0, &required_size_in_characters)) {
        DD_LOG(error) << "\"SetupDiGetDeviceInstanceIdW\" did not fail, what?!";
        return false;
      }
      else if (required_size_in_characters <= 0) {
        DD_LOG(error) << w_api.get_error_string(static_cast<LONG>(GetLastError())) << " \"SetupDiGetDeviceInstanceIdW\" failed while getting size.";
        return false;
      }

      instance_id.resize(required_size_in_characters);
      if (!SetupDiGetDeviceInstanceIdW(dev_info_handle, &dev_info_data, instance_id.data(), instance_id.size(), nullptr)) {
        DD_LOG(error) << w_api.get_error_string(static_cast<LONG>(GetLastError())) << " \"SetupDiGetDeviceInstanceIdW\" failed.";
        return false;
      }

      return !instance_id.empty();
    }

    /**
     * @brief Helper method for dealing with SetupAPI.
     * @returns True if EDID was retrieved and is non-empty, false otherwise.
     * @see get_device_id implementation for more context regarding this madness.
     */
    bool
    get_device_edid(const WinApiLayerInterface &w_api, HDEVINFO dev_info_handle, SP_DEVINFO_DATA &dev_info_data, std::vector<BYTE> &edid) {
      // We could just directly open the registry key as the path is known, but we can also use the this
      HKEY reg_key { SetupDiOpenDevRegKey(dev_info_handle, &dev_info_data, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ) };
      if (reg_key == INVALID_HANDLE_VALUE) {
        DD_LOG(error) << w_api.get_error_string(static_cast<LONG>(GetLastError())) << " \"SetupDiOpenDevRegKey\" failed.";
        return false;
      }

      const auto reg_key_cleanup {
        boost::scope::scope_exit([&w_api, &reg_key]() {
          const auto status { RegCloseKey(reg_key) };
          if (status != ERROR_SUCCESS) {
            DD_LOG(error) << w_api.get_error_string(status) << " \"RegCloseKey\" failed.";
          }
        })
      };

      DWORD required_size_in_bytes { 0 };
      auto status { RegQueryValueExW(reg_key, L"EDID", nullptr, nullptr, nullptr, &required_size_in_bytes) };
      if (status != ERROR_SUCCESS) {
        DD_LOG(error) << w_api.get_error_string(status) << " \"RegQueryValueExW\" failed when getting size.";
        return false;
      }

      edid.resize(required_size_in_bytes);

      status = RegQueryValueExW(reg_key, L"EDID", nullptr, nullptr, edid.data(), &required_size_in_bytes);
      if (status != ERROR_SUCCESS) {
        DD_LOG(error) << w_api.get_error_string(status) << " \"RegQueryValueExW\" failed when getting size.";
        return false;
      }

      return !edid.empty();
    }

    /**
     * @brief Converts a UTF-16 wide string into a UTF-8 string.
     * @param w_api Reference to the WinApiLayer.
     * @param value The UTF-16 wide string.
     * @return The converted UTF-8 string.
     */
    std::string
    to_utf8(const WinApiLayerInterface &w_api, const std::wstring &value) {
      // No conversion needed if the string is empty
      if (value.empty()) {
        return {};
      }

      // Get the output size required to store the string
      auto output_size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()), nullptr, 0, nullptr, nullptr);
      if (output_size == 0) {
        DD_LOG(error) << w_api.get_error_string(static_cast<LONG>(GetLastError())) << " failed to get UTF-8 buffer size.";
        return {};
      }

      // Perform the conversion
      std::string output(output_size, '\0');
      output_size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()), output.data(), static_cast<int>(output.size()), nullptr, nullptr);
      if (output_size == 0) {
        DD_LOG(error) << w_api.get_error_string(static_cast<LONG>(GetLastError())) << " failed to convert string to UTF-8.";
        return {};
      }

      return output;
    }
  }  // namespace

  std::string
  WinApiLayer::get_error_string(LONG error_code) const {
    std::ostringstream error;
    error << "[code: ";
    switch (error_code) {
      case ERROR_INVALID_PARAMETER:
        error << "ERROR_INVALID_PARAMETER";
        break;
      case ERROR_NOT_SUPPORTED:
        error << "ERROR_NOT_SUPPORTED";
        break;
      case ERROR_ACCESS_DENIED:
        error << "ERROR_ACCESS_DENIED";
        break;
      case ERROR_INSUFFICIENT_BUFFER:
        error << "ERROR_INSUFFICIENT_BUFFER";
        break;
      case ERROR_GEN_FAILURE:
        error << "ERROR_GEN_FAILURE";
        break;
      case ERROR_SUCCESS:
        error << "ERROR_SUCCESS";
        break;
      default:
        error << error_code;
        break;
    }
    error << ", message: " << std::system_category().message(static_cast<int>(error_code)) << "]";
    return error.str();
  }

  std::optional<WinApiLayerInterface::path_and_mode_data_t>
  WinApiLayer::query_display_config(query_type_e type) const {
    std::vector<DISPLAYCONFIG_PATH_INFO> paths;
    std::vector<DISPLAYCONFIG_MODE_INFO> modes;
    LONG result = ERROR_SUCCESS;

    // When we want to enable/disable displays, we need to get all paths as they will not be active.
    // This will require some additional filtering of duplicate and otherwise useless paths.
    UINT32 flags = type == query_type_e::Active ? QDC_ONLY_ACTIVE_PATHS : QDC_ALL_PATHS;
    flags |= QDC_VIRTUAL_MODE_AWARE;  // supported from W10 onwards

    do {
      UINT32 path_count { 0 };
      UINT32 mode_count { 0 };

      result = GetDisplayConfigBufferSizes(flags, &path_count, &mode_count);
      if (result != ERROR_SUCCESS) {
        DD_LOG(error) << get_error_string(result) << " failed to get display paths and modes!";
        return std::nullopt;
      }

      paths.resize(path_count);
      modes.resize(mode_count);
      result = QueryDisplayConfig(flags, &path_count, paths.data(), &mode_count, modes.data(), nullptr);

      // The function may have returned fewer paths/modes than estimated
      paths.resize(path_count);
      modes.resize(mode_count);

      // It's possible that between the call to GetDisplayConfigBufferSizes and QueryDisplayConfig
      // that the display state changed, so loop on the case of ERROR_INSUFFICIENT_BUFFER.
    } while (result == ERROR_INSUFFICIENT_BUFFER);

    if (result != ERROR_SUCCESS) {
      DD_LOG(error) << get_error_string(result) << " failed to query display paths and modes!";
      return std::nullopt;
    }

    DD_LOG(verbose) << "Result of " << (type == query_type_e::Active ? "ACTIVE" : "ALL") << " display config query:\n"
                    << dump_paths_and_modes(paths, modes) << "\n";
    return path_and_mode_data_t { paths, modes };
  }

  std::string
  WinApiLayer::get_device_id(const DISPLAYCONFIG_PATH_INFO &path) const {
    const auto device_path { get_monitor_device_path_wstr(*this, path) };
    if (device_path.empty()) {
      // Error already logged
      return {};
    }

    static const GUID monitor_guid { 0xe6f07b5f, 0xee97, 0x4a90, { 0xb0, 0x76, 0x33, 0xf5, 0x7b, 0xf4, 0xea, 0xa7 } };
    std::vector<BYTE> device_id_data;

    HDEVINFO dev_info_handle { SetupDiGetClassDevsW(&monitor_guid, nullptr, nullptr, DIGCF_DEVICEINTERFACE) };
    if (dev_info_handle) {
      const auto dev_info_handle_cleanup {
        boost::scope::scope_exit([this, &dev_info_handle]() {
          if (!SetupDiDestroyDeviceInfoList(dev_info_handle)) {
            DD_LOG(error) << get_error_string(static_cast<LONG>(GetLastError())) << " \"SetupDiDestroyDeviceInfoList\" failed.";
          }
        })
      };

      SP_DEVICE_INTERFACE_DATA dev_interface_data {};
      dev_interface_data.cbSize = sizeof(dev_interface_data);
      for (DWORD monitor_index = 0;; ++monitor_index) {
        if (!SetupDiEnumDeviceInterfaces(dev_info_handle, nullptr, &monitor_guid, monitor_index, &dev_interface_data)) {
          const DWORD error_code { GetLastError() };
          if (error_code == ERROR_NO_MORE_ITEMS) {
            break;
          }

          DD_LOG(warning) << get_error_string(static_cast<LONG>(error_code)) << " \"SetupDiEnumDeviceInterfaces\" failed.";
          continue;
        }

        std::wstring dev_interface_path;
        SP_DEVINFO_DATA dev_info_data {};
        dev_info_data.cbSize = sizeof(dev_info_data);
        if (!get_device_interface_detail(*this, dev_info_handle, dev_interface_data, dev_interface_path, dev_info_data)) {
          // Error already logged
          continue;
        }

        if (!boost::iequals(dev_interface_path, device_path)) {
          continue;
        }

        // Instance ID is unique in the system and persists restarts, but not driver re-installs.
        // It looks like this:
        //     DISPLAY\ACI27EC\5&4FD2DE4&5&UID4352 (also used in the device path it seems)
        //                a    b    c    d    e
        //
        //  a) Hardware ID - stable
        //  b) Either a bus number or has something to do with device capabilities - stable
        //  c) Another ID, somehow tied to adapter (not an adapter ID from path object) - stable
        //  d) Some sort of rotating counter thing, changes after driver reinstall - unstable
        //  e) Seems to be the same as a target ID from path, it changes based on GPU port - semi-stable
        //
        // The instance ID also seems to be a part of the registry key (in case some other info is needed in the future):
        //     HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Enum\DISPLAY\ACI27EC\5&4fd2de4&5&UID4352

        std::wstring instance_id;
        if (!get_device_instance_id(*this, dev_info_handle, dev_info_data, instance_id)) {
          // Error already logged
          break;
        }

        if (!get_device_edid(*this, dev_info_handle, dev_info_data, device_id_data)) {
          // Error already logged
          break;
        }

        // We are going to discard the unstable parts of the instance ID and merge the stable parts with the edid buffer (if available)
        auto unstable_part_index = instance_id.find_first_of(L'&', 0);
        if (unstable_part_index != std::wstring::npos) {
          unstable_part_index = instance_id.find_first_of(L'&', unstable_part_index + 1);
        }

        if (unstable_part_index == std::wstring::npos) {
          DD_LOG(error) << "Failed to split off the stable part from instance id string " << to_utf8(*this, instance_id);
          break;
        }

        auto semi_stable_part_index = instance_id.find_first_of(L'&', unstable_part_index + 1);
        if (semi_stable_part_index == std::wstring::npos) {
          DD_LOG(error) << "Failed to split off the semi-stable part from instance id string " << to_utf8(*this, instance_id);
          break;
        }

        device_id_data.insert(std::end(device_id_data),
          reinterpret_cast<const BYTE *>(instance_id.data()),
          reinterpret_cast<const BYTE *>(instance_id.data() + unstable_part_index));
        device_id_data.insert(std::end(device_id_data),
          reinterpret_cast<const BYTE *>(instance_id.data() + semi_stable_part_index),
          reinterpret_cast<const BYTE *>(instance_id.data() + instance_id.size()));

        static const auto dump_device_id_data { [](const auto &data) -> std::string {
          if (data.empty()) {
            return {};
          };

          std::ostringstream output;
          output << "[";
          for (std::size_t i = 0; i < data.size(); ++i) {
            output << "0x" << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << static_cast<int>(data[i]);
            if (i + 1 < data.size()) {
              output << " ";
            }
          }
          output << "]";

          return output.str();
        } };
        DD_LOG(verbose) << "Creating device id from EDID + instance ID: " << dump_device_id_data(device_id_data);
        break;
      }
    }

    if (device_id_data.empty()) {
      // Using the device path as a fallback, which is always unique, but not as stable as the preferred one
      DD_LOG(verbose) << "Creating device id from path " << to_utf8(*this, device_path);
      device_id_data.insert(std::end(device_id_data),
        reinterpret_cast<const BYTE *>(device_path.data()),
        reinterpret_cast<const BYTE *>(device_path.data() + device_path.size()));
    }

    static constexpr boost::uuids::uuid ns_id {};  // null namespace = no salt
    const auto boost_uuid { boost::uuids::name_generator_sha1 { ns_id }(device_id_data.data(), device_id_data.size()) };
    const std::string device_id { "{" + boost::uuids::to_string(boost_uuid) + "}" };

    DD_LOG(verbose) << "Created device id: " << to_utf8(*this, device_path) << " -> " << device_id;
    return device_id;
  }

  std::string
  WinApiLayer::get_monitor_device_path(const DISPLAYCONFIG_PATH_INFO &path) const {
    return to_utf8(*this, get_monitor_device_path_wstr(*this, path));
  }

  std::string
  WinApiLayer::get_friendly_name(const DISPLAYCONFIG_PATH_INFO &path) const {
    DISPLAYCONFIG_TARGET_DEVICE_NAME target_name = {};
    target_name.header.adapterId = path.targetInfo.adapterId;
    target_name.header.id = path.targetInfo.id;
    target_name.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
    target_name.header.size = sizeof(target_name);

    LONG result { DisplayConfigGetDeviceInfo(&target_name.header) };
    if (result != ERROR_SUCCESS) {
      DD_LOG(error) << get_error_string(result) << " failed to get target device name!";
      return {};
    }

    return target_name.flags.friendlyNameFromEdid ? to_utf8(*this, target_name.monitorFriendlyDeviceName) : std::string {};
  }

  std::string
  WinApiLayer::get_display_name(const DISPLAYCONFIG_PATH_INFO &path) const {
    DISPLAYCONFIG_SOURCE_DEVICE_NAME source_name = {};
    source_name.header.id = path.sourceInfo.id;
    source_name.header.adapterId = path.sourceInfo.adapterId;
    source_name.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
    source_name.header.size = sizeof(source_name);

    LONG result { DisplayConfigGetDeviceInfo(&source_name.header) };
    if (result != ERROR_SUCCESS) {
      DD_LOG(error) << get_error_string(result) << " failed to get display name! ";
      return {};
    }

    return to_utf8(*this, source_name.viewGdiDeviceName);
  }
}  // namespace display_device
