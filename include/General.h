#ifndef GENERAL_H
#define GENERAL_H

// Qt
#include <QTime>

// C++
#include <string>
#include <vector>

namespace UserDefine
{
    struct DAQRequestInfo
    {
        std::string sPath;
        std::string sFileName;
        int nDAQCount;
        QTime DAQTime;
        int msBufferSleep;
        int leastBufferEvent;
        bool clearQueueFlag;
    };

    bool ParseLine(std::string sInput, std::vector<double> &vOutput);

    /// @brief Convert UInt32 to 2 UInt16
    /// @param src_data [IN] address of UInt32 array
    /// @param src_counts [IN] how many numbers of UInt32 should be converted
    /// @param dst_data [IN] destination of UInt16 array
    /// @param dst_counts [OUT] how many numbers of UInt16 has been converted
    void ConvertUInt32ToUInt16s(uint32_t *src_data, int src_counts, uint16_t *dst_data, int *dst_counts);

    /// @brief Get locak IP for this PC
    /// @param gateIPList [OUT] Result for Network Gate IP list, first is Gate IP, second is Host IP
    /// @return 0 means error, 1 means success
    int GetGateIPList(std::vector<std::pair<std::string, std::string>> &gateIPList);

    /// @brief Get first 3 ip parts in string form, such as "192.168.1.";
    /// @return 
    std::string GetIPPrefix();
}

#endif
