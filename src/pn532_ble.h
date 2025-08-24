/**
 * @file pn532_ble.H
 * @author whywilson (https://github.com/whywilson)
 * @brief ESP PN532BLE
 * @version 0.0.1
 * @date 2024-11-06
 */

 #ifndef PN532_BLE_H
 #define PN532_BLE_H
 
 #include <NimBLEDevice.h>
 #if __has_include(<NimBLEExtAdvertising.h>)
 #define NIMBLE_V2_PLUS 1
 #include <NimBLEAdvertising.h>
 #include <NimBLEServer.h>
 #endif
 #include <array>
 #include <vector>
 
 class PN532_BLE {
 public:
     uint8_t DATA_PREAMBLE = 0x00;
     std::array<uint8_t, 2> DATA_START_CODE = {0x00, 0xFF};
     uint8_t DATA_TIF_SEND = 0xD4;
     uint8_t DATA_TIF_RECEIVE = 0xD5;
     uint8_t DATA_POSTAMBLE = 0x00;
 
     enum Command {
         Diagnose = 0x00,
         GetFirmwareVersion = 0x02,
         ReadRegister = 0x06,
         WriteRegister = 0x08,
         SAMConfiguration = 0x14,
         PowerDown = 0x16,
         InDataExchange = 0x40,
         InCommunicateThru = 0x42,
         InListPassiveTarget = 0x4A,
         InDeselect = 0x44,
         InRelease = 0x52,
         InSelect = 0x54,
         InAutoPoll = 0x60,
         TgInitAsTarget = 0x8C,
         TgGetData = 0x86,
         TgSetData = 0x8E
     };
 
     enum RspStatus {
         HF_TAG_OK = 0x00,     // IC card operation is successful
         HF_TAG_NO = 0x01,     // IC card not found
         HF_ERR_STAT = 0x02,   // Abnormal IC card communication
         HF_ERR_CRC = 0x03,    // IC card communication verification abnormal
         HF_COLLISION = 0x04,  // IC card conflict
         HF_ERR_BCC = 0x05,    // IC card BCC error
         MF_ERR_AUTH = 0x06,   // MF card verification failed
         HF_ERR_PARITY = 0x07, // IC card parity error
         HF_ERR_ATS = 0x08,    // ATS should be present but card NAKed, or ATS too large
 
         // Some operations with low frequency cards succeeded!
         LF_TAG_OK = 0x40,
         // Unable to search for a valid EM410X label
         EM410X_TAG_NO_FOUND = 0x41,
 
         // The parameters passed by the BLE instruction are wrong,
         // or the parameters passed by calling some functions are wrong
         PAR_ERR = 0x60,
         // The mode of the current device is wrong, and the corresponding
         // API cannot be called
         DEVICE_MODE_ERROR = 0x66,
         INVALID_CMD = 0x67,
         SUCCESS = 0x68,
         NOT_IMPLEMENTED = 0x69,
         FLASH_WRITE_FAIL = 0x70,
         FLASH_READ_FAIL = 0x71,
         INVALID_SLOT_TYPE = 0x72,
     };
 
     PN532_BLE(bool debug = false);
     ~PN532_BLE();
 
     bool searchForDevice();
     bool connectToDevice();
     void writeData(const std::vector<uint8_t> &data);
     void setDevice(NimBLEAdvertisedDevice device);
     bool isConnected();
     bool isPN532Killer();
     #ifdef NIMBLE_V2_PLUS
     NimBLEAdvertisedDevice *_device = nullptr;
     std::string getName() { return _device->getName(); }
     #else
     NimBLEAdvertisedDevice _device;
     std::string getName() { return _device.getName(); }
     #endif
 
     void wakeup();
     bool halt();
     bool setNormalMode();
     bool getVersion();
 
     typedef struct {
         uint8_t raw[250];
         size_t length;
         uint16_t command;
         uint8_t status;
         uint8_t dataSize;
         uint8_t data[200];
     } CmdResponse;
 
     CmdResponse cmdResponse;
     std::vector<PN532_BLE::CmdResponse> pn532Responses;
     std::vector<uint8_t> pn532bleBuffer;
 
     typedef struct {
         std::vector<uint8_t> atqa;
         uint8_t sak;
         uint8_t uidSize;
         std::vector<uint8_t> uid;
         String uid_hex;
         String sak_hex;
         String atqa_hex;
         String type;
     } Iso14aTagInfo;
     Iso14aTagInfo hf14aTagInfo;
     Iso14aTagInfo hf14aScan();
     bool mfAuth(std::vector<uint8_t> uid, uint8_t block, uint8_t *key, bool useKeyA);
     std::vector<uint8_t> mfRdbl(uint8_t block);
     bool mfWrbl(uint8_t block, std::vector<uint8_t> data);
     bool mfuWrbl(uint8_t block, std::vector<uint8_t> data);
     std::vector<uint8_t> sendData(std::vector<uint8_t> data, bool append_crc);
     std::vector<uint8_t> send7bit(std::vector<uint8_t> data);
     bool isGen1A();
     bool selectTag();
     bool isGen3();
     bool isGen4(std::string pwd);
 
     typedef struct {
         std::vector<uint8_t> uid;
         String uid_hex;
         uint8_t dsfid;
         uint8_t afi;
         uint8_t icRef;
         uint8_t blockSize;
     } Iso15TagInfo;
     Iso15TagInfo hf15TagInfo;
     std::vector<uint8_t> sendHf15Data(std::vector<uint8_t> data, bool append_crc, bool no_check_response);
     Iso15TagInfo hf15Scan();
     Iso15TagInfo hf15Info();
     std::vector<uint8_t> hf15Rdbl(uint8_t block);
     bool hf15Wrbl(uint8_t block, std::vector<uint8_t> data);
 
     std::vector<uint8_t> getData();
     std::vector<uint8_t> setData(const std::vector<uint8_t> &data);
     bool inRelease();
     std::vector<uint8_t> tgInitAsTarget(const std::vector<uint8_t> &data);
 
     typedef struct {
         std::vector<uint8_t> uid;
         String uid_hex;
         int id_dec;
     } LfTagInfo;
     LfTagInfo lfTagInfo;
 
     LfTagInfo lfScan();
 
     uint8_t mifareDefaultKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
     uint8_t mifareKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
 
 private:
     std::vector<NimBLEUUID> serviceUUIDs = {NimBLEUUID("FFF0"), NimBLEUUID("FFE0")};
     NimBLERemoteService *getService(NimBLEClient *pClient);
 
     NimBLERemoteService *pSvc = nullptr;
     NimBLERemoteCharacteristic *chrWrite = nullptr;
     NimBLERemoteCharacteristic *chrNotify = nullptr;
 
     bool _debug = false;
 
     bool writeCommand(Command cmd, uint8_t *data = nullptr, size_t length = 0);
     bool writeCommand(Command cmd, const std::vector<uint8_t> &data);
     bool checkResponse(uint8_t cmd);
     bool isCompleteFrame(uint8_t *pData, size_t length);
 
     bool resetRegister();
 
     Iso14aTagInfo parseHf14aScan(uint8_t *data, uint8_t dataSize);
     String getTagType();
     String getHf14aTagType();
     Iso15TagInfo parseHf15Scan(uint8_t *data, uint8_t dataSize);
     Iso15TagInfo parseHf15TagInfo(uint8_t *data, uint8_t dataSize);
     String getHf15TagType();
     LfTagInfo parseLfScan(uint8_t *data, uint8_t dataSize);
 
     uint8_t dcs(uint8_t *data, size_t length);
     void appendCrcA(std::vector<uint8_t> &data);
     void appendCrc16Ccitt(std::vector<uint8_t> &data);
     String bytes2HexString(std::vector<uint8_t> *data, uint8_t dataSize);
     std::vector<uint8_t> hexStringToUint8Array(const std::string &hexString);
     void NotifyCallBack(
         NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify
     );
     void handleNotifyCallBack(
         NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify
     );
 };
 
 #endif // PN532_BLE_H
 