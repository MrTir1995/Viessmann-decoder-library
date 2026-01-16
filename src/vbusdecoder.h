/*
 * Resol VBUS decoder library v1.0
 * Created by Martin Saidl January, 2018
 */
 
#pragma once
#ifndef VBUSDecoder_h
#define VBUSDecoder_h

#include <Arduino.h>

class VBUSDecoder {
  
  public:
    VBUSDecoder(Stream* serial);
    ~VBUSDecoder();
    
    void begin();
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

  private:
    Stream* _stream;
    enum T_state: uint8_t {
      SYNC,
      RECEIVE,
      DECODE,
      ERROR
    } _state;
    uint16_t _dstAddr;
    uint16_t _srcAddr;
    uint8_t _protocol;
    uint16_t _cmd;
    uint8_t _frameCnt;
    uint16_t _frameLen;
    uint8_t _rcvBuffer[255];
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
    
    uint8_t _calcCRC(const uint8_t *Buffer, uint8_t Offset, uint8_t Length);
    void _septetInject(uint8_t *Buffer, uint8_t Offset, uint8_t Length);
    float _calcTemp(uint8_t Byte1, uint8_t Byte2);
    void _headerDecoder();

    void _syncHandler();
    void _receiveHandler();
    void _decodeHandler();
    void _errorHandler();

    void _defaultDecoder();
    void _vitosolic200Decoder();
    void _deltaSolBXDecoder();
    void _deltaSolMXDecoder();
};

#endif
