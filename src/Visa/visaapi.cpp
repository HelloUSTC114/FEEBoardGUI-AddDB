#define VISAAPI_CXX
#pragma warning(disable : 4996)
#include "visaapi.h"

// C++
#include <string>
#include <iostream>

std::string ConvertAFGWaveform(AFGWaveform waveform)
{
    switch (waveform)
    {
    case (Pulse):
        return "Pulse";
    case (DC):
        return "DC";
    case (USER1):
        return "USER1";
    case (USER2):
        return "USER2";
    case (USER3):
        return "USER3";
    default:
        return "";
    }
    return "";
}

std::string ConvertAFGFreqUnit(AFGFreqUnit unit)
{
    switch (unit)
    {
    case (MHz):
        return "MHz";
    case (kHz):
        return "kHz";
    case (Hz):
        return "Hz";
    default:
        return "";
    }
    return "";
}

VisaAPI::VisaAPI(std::string sDeviceName)
{
    OpenDevice(sDeviceName);
    // InitDevice();
}

std::stringstream VisaAPI::gss;

VisaAPI::~VisaAPI()
{
    std::cout << "Destructor" << std::endl;
    if (fDeviceOpenFlag)
        CloseDevice();
}

bool VisaAPI::TryConnectDevice(std::string sDeviceName)
{
    if (!fDeviceOpenFlag)
    {
        return OpenDevice(sDeviceName);
    }
    return false;
}

void VisaAPI::InitDevice()
{
    if (!fDeviceFound)
        return;
    int status;

    // Send an ID query.
    status = viWrite(device, (ViBuf) "*idn?", 5, &sendCharCount);

    // Clear the buffer and read the response
    memset(buffer, 0, sizeof(buffer));
    status = viRead(device, (ViBuf)buffer, sizeof(buffer), &sendCharCount);
    // Print the response
    std::cout << "id: " << deviceName << ": " << buffer << std::endl;

    // Core write read
    std::string sCmd;

    // Oscilllator setting
    sCmd = "source:roscillator:source internal";
    WriteCMD(sCmd);

    // sCmd = "source1:function:shape edecay";
    sCmd = "source1:function:shape user1";
    WriteCMD(sCmd);

    sCmd = "source1:frequency:mode CW";
    WriteCMD(sCmd);

    sCmd = "source1:frequency:CW 1.5kHz";
    WriteCMD(sCmd);

    sCmd = "source1:voltage:level:immediate:amplitude 50mVpp";
    WriteCMD(sCmd);

    sCmd = "source1:voltage:level:immediate:offset 0mV";
    WriteCMD(sCmd);

    // Output settings
    sCmd = "output1:impedance 50";
    WriteCMD(sCmd);

    sCmd = "output1:polarity normal";
    WriteCMD(sCmd);

    sCmd = "output:trigger:mode trigger";
    WriteCMD(sCmd);

    // Burst mode setting
    sCmd = "source1:burst:mode triggered";
    WriteCMD(sCmd);

    sCmd = "source1:burst:ncycles 1";
    WriteCMD(sCmd);

    sCmd = "source1:burst:state ON";
    WriteCMD(sCmd);

    sCmd = "source1:burst:tdelay 0ns";
    WriteCMD(sCmd);

    //  on
    sCmd = "output1:state 1";
    WriteCMD(sCmd);
}

bool VisaAPI::OpenDevice(std::string sDeviceName)
{
    auto status = viOpenDefaultRM(&defaultRM);
    if (!fDeviceFound)
    {
        viFindRsrc(defaultRM, "TCPIP?*INSTR", &deviceList, &itemCnt, deviceName);
        if (itemCnt > 0)
            fDeviceFound = 1;
    }
    sprintf(deviceName, sDeviceName.c_str());
    status = viOpen(defaultRM, deviceName, VI_NULL, VI_NULL, &device);
    if (status == 0)
    {
        fDeviceFound = 1;
    }
    else
    {
        std::cout << "Error: " << deviceName << "\t not found." << std::endl;
        return false;
    }
    std::cout << "API open device: " << std::endl;
    std::cout << defaultRM << '\t' << deviceList << '\t' << itemCnt << '\t' << deviceName << '\t' << device << std::endl;
    WriteCMD("*idn?");
    ReadBuf();

    if (status == VI_SUCCESS)
    {
        fDeviceOpenFlag = true;
        return true;
    }
    else
    {
        fDeviceOpenFlag = false;
        return false;
    }
}

int VisaAPI::WriteCMD(std::string sCmd)
{
    if (!fDeviceFound)
        return 1;
    auto status = viWrite(device, (ViBuf)sCmd.c_str(), sCmd.size(), &sendCharCount);
    _sleep(100);
    if (status < 0)
        ProcessError(status);
    return status;
}

int VisaAPI::WriteCMDSerial(std::vector<std::string> vCMDs)
{
    if (!fDeviceFound)
        return 1;
    for (int i = 0; i < vCMDs.size(); i++)
    {
        WriteCMD(vCMDs[i]);
    }
    return 0;
}

// int VisaAPI::ReadBuf()
std::string VisaAPI::ReadBuf()
{
    if (!fDeviceFound)
        // return 1;
        return "";
    auto status = viRead(device, (ViBuf)buffer, 4096, &sendCharCount);
    _sleep(100);
    std::cout << "Recieve: " << '\t';
    for (int i = 0; i < sendCharCount; i++)
    {
        std::cout << buffer[i];
    }
    if (status < 0)
    {
        ProcessError(status);
        return "";
    }
    return (std::string)buffer;
    // return status;
}

std::string VisaAPI::Query(std::string sCmd)
{
    if (!fDeviceFound)
        return "";
    memset(buffer, '\0', 4096 * sizeof(char));
    auto status = viQueryf(device, (ViString)sCmd.c_str(), "%s", buffer);
    if (status < 0)
        ProcessError(status);
    return std::string(buffer);
}

void VisaAPI::CloseDevice()
{

    viClose(device);
    viClose(defaultRM);
    // std::cout << "Closed." << std::endl;
}

int VisaAPI::ProcessError(ViStatus status)
{
    memset(errBuffer, '\0', 256 * sizeof(char));
    // Report error and clean up
    viStatusDesc(device, status, errBuffer);
    std::cout << "Failure: " << errBuffer << std::endl;
    return 1;
}

// const std::string AFGVisaAPI::fDeviceName = "TCPIP::192.168.1.177::INSTR";
// const std::string AFGVisaAPI::fDeviceName = "TCPIP::192.168.124.5::INSTR";
// AFGVisaAPI::AFGVisaAPI() : VisaAPI(fDeviceName.c_str())
AFGVisaAPI::AFGVisaAPI() : VisaAPI()
{
    // std::cout << "Constructing: AFG " << std::endl;
    // TryConnectDevice(fDeviceName.c_str());
}

AFGVisaAPI *AFGVisaAPI::Instance()
{
    static AFGVisaAPI instance;
    return &instance;
}

int AFGVisaAPI::SetAmp(int amp)
{
    int status = 0;
    std::string sCmd;
    sCmd = "source1:voltage:level:immediate:amplitude " + std::to_string(amp) + "mVpp";
    int rtn = WriteCMD(sCmd);
    if (rtn < 0)
        return rtn;

    // Read amplitude
    sCmd = "source1:voltage:level:immediate:amplitude?";
    double recieveAmp = 0;

    // char *buf[256];
    // status = viQueryf(device, (ViString)sCmd.c_str(), "%lf", &recieveAmp);
    std::string sQuery = Query(sCmd);
    sscanf(sQuery.c_str(), "%lf", &recieveAmp);

    std::cout << "Recieved Amplitude: " << recieveAmp * 1000 << " mV." << std::endl;
    _sleep(100);
    return 0;
}

int AFGVisaAPI::SetHigh(double high)
{
    int status = 0;
    char buf[20];
    sprintf(buf, "%.2f", high);
    std::string sCmd;
    std::string sHigh = buf;

    sCmd = "source1:voltage:high " + sHigh + "mV";
    int rtn = WriteCMD(sCmd);
    if (rtn < 0)
        return rtn;

    // Read amplitude
    sCmd = "source1:voltage:high?";
    double recieveAmp = 0;
    // char *buf[256];
    // status = viQueryf(device, (ViString)sCmd.c_str(), "%lf", &recieveAmp);
    std::string sQuery = Query(sCmd);
    sscanf(sQuery.c_str(), "%lf", &recieveAmp);

    std::cout << "Recieved high: " << recieveAmp * 1000 << " mV." << std::endl;
    _sleep(100);
    return 0;
}

int AFGVisaAPI::SetLow(double low)
{
    int status = 0;
    char buf[20];
    sprintf(buf, "%.2f", low);
    std::string sCmd;
    std::string sLow = buf;

    sCmd = "source1:voltage:low " + sLow + "mV";
    int rtn = WriteCMD(sCmd);
    if (rtn < 0)
        return rtn;

    // Read amplitude
    sCmd = "source1:voltage:low?";
    double recieveAmp = 0;
    // char *buf[256];
    // status = viQueryf(device, (ViString)sCmd.c_str(), "%lf", &recieveAmp);
    std::string sQuery = Query(sCmd);
    sscanf(sQuery.c_str(), "%lf", &recieveAmp);

    std::cout << "Recieved low: " << recieveAmp * 1000 << " mV." << std::endl;
    _sleep(100);
    return 0;
}

int AFGVisaAPI::SetOffset(double offset)
{
    int status = 0;
    char buf[20];
    sprintf(buf, "%.2f", offset);
    std::string sCmd;
    std::string sOffset = buf;

    sCmd = "source1:voltage:offset " + sOffset + "mV";
    int rtn = WriteCMD(sCmd);
    if (rtn < 0)
        return rtn;

    // Read amplitude
    sCmd = "source1:voltage:offset?";
    double recieveAmp = 0;
    // char *buf[256];
    // status = viQueryf(device, (ViString)sCmd.c_str(), "%lf", &recieveAmp);
    std::string sQuery = Query(sCmd);
    sscanf(sQuery.c_str(), "%lf", &recieveAmp);

    std::cout << "Recieved offset: " << recieveAmp * 1000 << " mV." << std::endl;
    _sleep(100);
    return 0;
}

int AFGVisaAPI::SetChannelStatus(int ch, bool openFlag)
{
    if (ch < 1 || ch > 2)
        return 1;
    int status = 0;
    char buf[80];
    sprintf(buf, "output%d %d", ch, openFlag);
    std::cout << "Test: " << buf << std::endl;
    std::string sCmd = buf;

    int rtn = WriteCMD(sCmd);
    if (rtn < 0)
        return rtn;

    // Read amplitude
    sCmd = "output1?";
    int recieveAmp = 0;
    // char *buf[256];
    // status = viQueryf(device, (ViString)sCmd.c_str(), "%d", &recieveAmp);
    std::string sQuery = Query(sCmd);
    sscanf(sQuery.c_str(), "%d", &recieveAmp);

    std::cout << "Channel " << ch << " status: " << recieveAmp << std::endl;
    _sleep(100);
    return 0;
}

int AFGVisaAPI::SetWaveForm(AFGWaveform wave)
{
    int status = 0;
    std::string sCmd;
    std::string sWave = ConvertAFGWaveform(wave);
    if (sWave == "")
        return 1;
    sCmd = "source1:function:shape " + sWave;
    int rtn = WriteCMD(sCmd);
    if (rtn < 0)
        return rtn;

    // Read amplitude
    sCmd = "source1:function:shape?";
    // double recieveAmp = 0;
    // memset(buffer, '\0', 256 * sizeof(char));
    // status = viQueryf(device, (ViString)sCmd.c_str(), "%s", buffer);
    // std::cout << "Recieved Amplitude: " << buffer << " mV." << std::endl;
    std::string sQuery = Query(sCmd);
    std::cout << "Recieved Amplitude: " << sQuery << " mV." << std::endl;
    _sleep(100);
    return 0;
}

// const std::string Agi344VisaAPI::fDeviceName = "TCPIP::192.168.1.178::INSTR";
// const std::string Agi344VisaAPI::fDeviceName = "USB::0x0957::0x0618::MY51520158::INSTR";
// Agi344VisaAPI::Agi344VisaAPI() : VisaAPI(fDeviceName.c_str())
Agi344VisaAPI::Agi344VisaAPI() : VisaAPI()
{
}

Agi344VisaAPI *Agi344VisaAPI::Instance()
{
    static Agi344VisaAPI instance;
    return &instance;
}

int Agi344VisaAPI::SetImpedance(bool autoON)
{
    Clear34410();
    int status = 0;
    std::string sCmd;
    sCmd = "sense:voltage:dc:impedance:auto" + std::to_string(autoON);
    int rtn = WriteCMD(sCmd);
    if (rtn < 0)
        return rtn;

    // Read amplitude
    sCmd = "sense:voltage:dc:impedance:auto?";
    int recieveAmp = 0;

    // char *buf[256];
    // status = viQueryf(device, (ViString)sCmd.c_str(), "%lf", &recieveAmp);
    std::string sQuery = Query(sCmd);
    sscanf(sQuery.c_str(), "%d", &recieveAmp);

    std::cout << "Recieved Impedance auto: " << recieveAmp << std::endl;
    // std::cout << "Recieved Impedance auto: " << sQuery << std::endl;
    _sleep(100);
    return 0;
}

int Agi344VisaAPI::SetSamplePoints(int points)
{
    int status = 0;
    std::string sCmd;
    sCmd = "sample:count " + std::to_string(points);
    int rtn = WriteCMD(sCmd);
    if (rtn < 0)
    {
        fNPoints = 0;
        return rtn;
    }
    int recieveAmp;
    status = GetSamplePoints(recieveAmp);
    if (rtn < 0)
    {
        fNPoints = 0;
        return rtn;
    }
    std::cout << "Recieved sample points: " << recieveAmp << std::endl;
    fNPoints = points;
    _sleep(100);
    return 0;
}

int Agi344VisaAPI::GetSamplePoints(int &points)
{
    int status = 0;
    std::string sCmd;
    sCmd = "sample:count?";
    std::string sQuery = Query(sCmd);

    if (sQuery == "")
    {
        points = 0;
    }
    double recieveAmp = 0;
    sscanf(sQuery.c_str(), "%lf", &recieveAmp);
    points = (int)recieveAmp;

    return 0;
}

int Agi344VisaAPI::InitMeasure()
{
    int rtn = 0;
    std::string sCmd = "";

    sCmd = "measure:dc";
    WriteCMD(sCmd);
    // sCmd = "sense:voltage:dc:nplc 10";
    // WriteCMD(sCmd);
    sCmd = "trigger:source bus";
    WriteCMD(sCmd);

    return 0;
}

double Agi344VisaAPI::MeasureOnce()
{
    int rtn = 0;
    std::string sCmd = "";

    sCmd = "init";
    WriteCMD(sCmd);
    sCmd = "*trg";
    WriteCMD(sCmd);

    sCmd = "fetch?"; // Query
    std::string sQuery = Query(sCmd);
    double temp = 0;
    sscanf(sQuery.c_str(), "%lf", &temp);
    return temp;
}

void Agi344VisaAPI::Clear34410()
{
    std::vector<std::string> sCMDs;
    sCMDs.push_back("measure:dc");
    sCMDs.push_back("init");
    sCMDs.push_back("*trg");
}

void Agi344VisaAPI::InitMeasure34410()
{
    int rtn = 0;
    std::string sCmd = "";

    // sCmd = "*rst";
    // WriteCMD(sCmd);
    // sCmd = "measure:dc";
    // WriteCMD(sCmd);

    sCmd = "trigger:source bus";
    WriteCMD(sCmd);
    // sCmd = "init";
    // WriteCMD(sCmd);
    // sCmd = "*trg";
    // WriteCMD(sCmd);
    // _sleep(5000);

    sCmd = "sense:voltage:dc:impedance:auto 1";
    WriteCMD(sCmd);

    sCmd = "sample:count 40";
    WriteCMD(sCmd);

    sCmd = "sense:voltage:dc:nplc 1";
    WriteCMD(sCmd);
}

void Agi344VisaAPI::Measure34410(std::vector<double> &vResult)
{
    std::string sCmd;
    sCmd = "init";
    WriteCMD(sCmd);
    sCmd = "*trg";
    WriteCMD(sCmd);
    _sleep(2000);

    sCmd = "R? ";
    // WriteCMD(sCmd);
    std::string sReply = Query(sCmd);
    std::cout << sReply << std::endl;

    gss.clear();
    gss.str(sReply);
    char c;
    gss >> c;
    int header;
    gss >> header;
    for (int i = 0; i < fNPoints; i++)
    {
        if (gss.eof())
            break;
        double temp;
        gss >> temp;
        if (i != fNPoints - 1)
            gss >> c;
        std::cout << i << '\t' << temp << std::endl;
        vResult.push_back(temp);
    }
}
