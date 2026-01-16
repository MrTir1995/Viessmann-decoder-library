/*
 * Viessmann Multi-Protocol Library v2.0
 * Created by Martin Saidl January, 2018
 * Extended for multi-protocol support January, 2026
 */
 
#pragma once
#ifndef VBUSDecoder_h
#define VBUSDecoder_h

#include <Arduino.h>

// Protocol types supported by the library
enum ProtocolType: uint8_t {
  PROTOCOL_VBUS = 0,    // RESOL VBUS Protocol (default)
  PROTOCOL_KW = 1,      // Viessmann KW-Bus (VS1)
  PROTOCOL_P300 = 2,    // Viessmann P300/VS2 (Optolink)
  PROTOCOL_KM = 3       // Viessmann KM-Bus
};

// Bus participant information structure
struct BusParticipant {
  uint16_t address;           // Device address
  uint32_t lastSeen;          // Last time packet was received (millis)
  uint8_t tempChannels;       // Number of temperature channels
  uint8_t pumpChannels;       // Number of pump channels
  uint8_t relayChannels;      // Number of relay channels
  bool autoDetected;          // True if auto-detected, false if manually configured
  char name[32];              // Device name/description
  bool active;                // True if participant is active
};

class VBUSDecoder {
  
  public:
    VBUSDecoder(Stream* serial);
    ~VBUSDecoder();
    
    void begin(ProtocolType protocol = PROTOCOL_VBUS);
    void loop();
    float const getTemp(uint8_t idx) const;
    uint8_t const getPump(uint8_t idx) const;
    bool const getRelay(uint8_t idx) const;
    uint8_t const getTempNum() const;
    uint8_t const getRelayNum() const;
    uint8_t const getPumpNum() const;
    bool const getVbusStat() const;
    bool const isReady() const;
    uint16_t const getErrorMask() const;
    uint16_t const getSystemTime() const;
    uint32_t const getOperatingHours(uint8_t idx) const;
    uint16_t const getHeatQuantity() const;
    uint8_t const getSystemVariant() const;
    ProtocolType const getProtocol() const;
    
    // Bus participant discovery and management
    void enableAutoDiscovery(bool enable = true);
    bool isAutoDiscoveryEnabled() const;
    uint8_t getParticipantCount() const;
    const BusParticipant* getParticipant(uint8_t idx) const;
    const BusParticipant* getParticipantByAddress(uint16_t address) const;
    bool addParticipant(uint16_t address, const char* name = nullptr, 
                       uint8_t tempChannels = 0, uint8_t pumpChannels = 0, 
                       uint8_t relayChannels = 0);
    bool removeParticipant(uint16_t address);
    void clearParticipants();
    uint16_t getCurrentSourceAddress() const;

  private:
    Stream* _stream;
    ProtocolType _protocol;
    enum T_state: uint8_t {
      SYNC,
      RECEIVE,
      DECODE,
      ERROR
    } _state;
    uint16_t _dstAddr;
    uint16_t _srcAddr;
    uint8_t _protocolVer;
    uint16_t _cmd;
    uint8_t _frameCnt;
    uint16_t _frameLen;
    static const uint16_t MAX_BUFFER_SIZE = 255;
    uint8_t _rcvBuffer[MAX_BUFFER_SIZE];
    uint8_t _rcvBufferIdx;
    bool _errorFlag;
    bool _readyFlag;
    float _temp[32];
    uint8_t _pump[32];
    bool _relay[32];
    uint8_t _tempNum;
    uint8_t _relayNum;
    uint8_t _pumpNum;
    uint32_t _lastMillis;
    uint16_t _errorMask;
    uint16_t _systemTime;
    uint32_t _operatingHours[8];
    uint16_t _heatQuantity;
    uint8_t _systemVariant;
    
    // Bus participant discovery
    static const uint8_t MAX_PARTICIPANTS = 16;
    BusParticipant _participants[MAX_PARTICIPANTS];
    uint8_t _participantCount;
    bool _autoDiscoveryEnabled;
    
    void _updateParticipant(uint16_t address);
    int8_t _findParticipantIndex(uint16_t address) const;
    void _configureParticipantChannels(BusParticipant* participant, uint16_t address);
    
    // Common utility functions
    uint8_t _calcCRC(const uint8_t *Buffer, uint8_t Offset, uint8_t Length);
    void _septetInject(uint8_t *Buffer, uint8_t Offset, uint8_t Length);
    float _calcTemp(uint8_t Byte1, uint8_t Byte2);
    void _headerDecoder();

    // VBUS protocol handlers
    void _vbusSyncHandler();
    void _vbusReceiveHandler();
    void _vbusDecodeHandler();
    void _errorHandler();

    // KW-Bus protocol handlers
    void _kwSyncHandler();
    void _kwReceiveHandler();
    void _kwDecodeHandler();
    
    // P300 protocol handlers
    void _p300SyncHandler();
    void _p300ReceiveHandler();
    void _p300DecodeHandler();
    
    // KM-Bus protocol handlers
    void _kmSyncHandler();
    void _kmReceiveHandler();
    void _kmDecodeHandler();

    // VBUS device decoders
    void _defaultDecoder();
    void _vitosolic200Decoder();
    void _deltaSolBXDecoder();
    void _deltaSolMXDecoder();
    
    // KW-Bus device decoders
    void _kwDefaultDecoder();
    
    // P300 device decoders
    void _p300DefaultDecoder();
    
    // KM-Bus device decoders
    void _kmDefaultDecoder();
};

#endif
