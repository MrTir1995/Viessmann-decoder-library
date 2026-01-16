# Library Architecture

## Overview

The Viessmann Multi-Protocol Library uses a modular architecture that supports multiple protocols through a unified interface.

```
┌─────────────────────────────────────────────────────────────┐
│                     User Application                         │
│  (Arduino Sketch using VBUSDecoder class)                   │
└────────────────────────┬────────────────────────────────────┘
                         │
                         │ begin(PROTOCOL_xxx)
                         │ loop()
                         │ getTemp(), getPump(), getRelay()
                         │
┌────────────────────────▼────────────────────────────────────┐
│                   VBUSDecoder Class                          │
│                  (Common Interface)                          │
├──────────────────────────────────────────────────────────────┤
│ Public API:                                                  │
│  - begin(protocol)      Select protocol                     │
│  - loop()               Process incoming data               │
│  - getTemp(idx)         Get temperature                     │
│  - getPump(idx)         Get pump power                      │
│  - getRelay(idx)        Get relay state                     │
│  - isReady()            Check data availability             │
│  - getProtocol()        Query active protocol               │
└────────────────────────┬────────────────────────────────────┘
                         │
                         │ Protocol Selection
                         │
        ┌────────────────┼────────────────┬────────────────┐
        │                │                │                │
┌───────▼──────┐ ┌───────▼──────┐ ┌──────▼───────┐ ┌──────▼───────┐
│   VBUS       │ │   KW-Bus     │ │    P300      │ │   KM-Bus     │
│   Handler    │ │   Handler    │ │   Handler    │ │   Handler    │
├──────────────┤ ├──────────────┤ ├──────────────┤ ├──────────────┤
│ _vbusSync    │ │ _kwSync      │ │ _p300Sync    │ │ _kmSync      │
│ _vbusReceive │ │ _kwReceive   │ │ _p300Receive │ │ _kmReceive   │
│ _vbusDecode  │ │ _kwDecode    │ │ _p300Decode  │ │ _kmDecode    │
└──────┬───────┘ └──────┬───────┘ └──────┬───────┘ └──────┬───────┘
       │                │                │                │
       │ Frame Parsing  │ Frame Parsing  │ Frame Parsing  │ Frame Parsing
       │                │                │                │
┌──────▼────────────────▼────────────────▼────────────────▼───────┐
│                  Common Data Storage                             │
│  _temp[], _pump[], _relay[], _errorMask, _systemTime, etc.     │
└──────────────────────────────────────────────────────────────────┘
```

## State Machine

Each protocol handler operates as a state machine:

```
     ┌──────────────────────────────────────────┐
     │              SYNC State                  │
     │  Wait for protocol-specific sync byte    │
     │  - VBUS: 0xAA                           │
     │  - KW-Bus: 0x01                         │
     │  - P300: 0x05/0x01                      │
     │  - KM-Bus: 0x68                         │
     └──────────┬───────────────────────────────┘
                │ Sync byte detected
                │
     ┌──────────▼───────────────────────────────┐
     │            RECEIVE State                  │
     │  Accumulate frame bytes                   │
     │  Validate length and structure            │
     │  Check for buffer overflow                │
     └──────────┬───────────────────────────────┘
                │ Complete frame received
                │
     ┌──────────▼───────────────────────────────┐
     │             DECODE State                  │
     │  Validate checksum                        │
     │  Extract data fields                      │
     │  Populate member variables                │
     │  Set _readyFlag = true                   │
     └──────────┬───────────────────────────────┘
                │ Decoding complete
                │
     ┌──────────▼───────────────────────────────┐
     │         Return to SYNC                    │
     └───────────────────────────────────────────┘
     
     ┌───────────────────────────────────────────┐
     │             ERROR State                   │
     │  Invalid frame, checksum error,           │
     │  timeout, or buffer overflow              │
     │  Set _errorFlag = true                   │
     └──────────┬────────────────────────────────┘
                │
                └──────────► Return to SYNC
```

## Protocol-Specific Frame Formats

### VBUS Protocol
```
┌────┬────────┬────────┬────────┬────┬────────┬─────────┬─────┐
│0xAA│DestAddr│SrcAddr │Proto+  │Cmd │FrameCnt│  Data   │ CRC │
│    │2 bytes │2 bytes │Cmd     │    │        │ Frames  │     │
└────┴────────┴────────┴────────┴────┴────────┴─────────┴─────┘
       Header (10 bytes)              Data (N*6 bytes)

Each data frame: [4 data bytes][1 septet][1 checksum]
```

### KW-Bus Protocol
```
┌────┬─────┬────────┬─────────────┬──────────┐
│0x01│ Len │ Addr   │    Data     │ Checksum │
│    │     │        │   N bytes   │   XOR    │
└────┴─────┴────────┴─────────────┴──────────┘
```

### P300 Protocol
```
┌──────┬─────┬──────┬─────────┬─────────────┬──────────┐
│0x05/ │ Len │ Type │ Address │    Data     │ Checksum │
│0x01  │     │      │ 2 bytes │   N bytes   │   Sum    │
└──────┴─────┴──────┴─────────┴─────────────┴──────────┘
```

### KM-Bus Protocol
```
┌────┬─────┬─────┬────┬──────┬────────┬─────────┬──────────┬────┐
│0x68│ Len │ Len │0x68│ Ctrl │  Addr  │  Data   │ Checksum │0x16│
│    │     │     │    │      │        │ N bytes │   Sum    │    │
└────┴─────┴─────┴────┴──────┴────────┴─────────┴──────────┴────┘
```

## Data Flow

```
Serial Port         Protocol Handler       Data Storage        User API
─────────            ────────────          ─────────          ────────

Raw Bytes     ───►   Frame Parsing   ───►  _temp[0..31]  ───►  getTemp(idx)
  0xAA 0x10          Sync Detection        _pump[0..31]  ───►  getPump(idx)
  0x00 0x01          Checksum Check        _relay[0..31] ───►  getRelay(idx)
  0x20 0x05          Septet Inject         _errorMask    ───►  getErrorMask()
  ...                Device Decoder        _systemTime   ───►  getSystemTime()
                                            _opHours[0..7]───►  getOperatingHours()
                                            _heatQuantity ───►  getHeatQuantity()
```

## Device-Specific Decoders

Each protocol can support multiple device types:

```
VBUS Protocol
├── _vitosolic200Decoder()    (0x1060)
├── _deltaSolBXDecoder()       (0x7E11, 0x7E21)
├── _deltaSolMXDecoder()       (0x7E31)
└── _defaultDecoder()          (Generic RESOL)

KW-Bus Protocol
└── _kwDefaultDecoder()        (Generic KW devices)

P300 Protocol
└── _p300DefaultDecoder()      (Generic P300 devices)

KM-Bus Protocol
└── _kmDefaultDecoder()        (Placeholder)
```

## Memory Layout

```
VBUSDecoder Object
├── Protocol Configuration
│   ├── _protocol (ProtocolType)
│   └── _stream (Stream*)
│
├── State Machine
│   ├── _state (T_state enum)
│   ├── _errorFlag (bool)
│   └── _readyFlag (bool)
│
├── Receive Buffer
│   ├── _rcvBuffer[255] (uint8_t array)
│   └── _rcvBufferIdx (uint8_t)
│
├── Frame Header
│   ├── _dstAddr (uint16_t)
│   ├── _srcAddr (uint16_t)
│   ├── _protocolVer (uint8_t)
│   ├── _cmd (uint16_t)
│   ├── _frameCnt (uint8_t)
│   └── _frameLen (uint16_t)
│
└── Decoded Data
    ├── _temp[32] (float array)
    ├── _pump[32] (uint8_t array)
    ├── _relay[32] (bool array)
    ├── _tempNum (uint8_t)
    ├── _pumpNum (uint8_t)
    ├── _relayNum (uint8_t)
    ├── _errorMask (uint16_t)
    ├── _systemTime (uint16_t)
    ├── _operatingHours[8] (uint32_t array)
    ├── _heatQuantity (uint16_t)
    └── _systemVariant (uint8_t)
```

## Timing and Performance

### Polling Rate
- `loop()` should be called frequently (ideally in every Arduino loop iteration)
- Non-blocking design allows other code to run
- Processing time: < 1ms per call when no data available

### Timeout Handling
- 20-second timeout without data → ERROR state
- Automatic recovery: returns to SYNC state
- `_lastMillis` updated on successful frame reception

### Buffer Management
- Single 255-byte receive buffer (MAX_BUFFER_SIZE)
- Overflow protection in all protocol handlers
- Buffer cleared on sync detection

## Extension Points

### Adding a New Device Decoder

1. Identify device address/ID from frame header
2. Create decoder function: `void _myDeviceDecoder()`
3. Add case in appropriate protocol's decode handler
4. Extract data fields and populate member variables

Example:
```cpp
void VBUSDecoder::_myDeviceDecoder() {
  // Set counts
  _tempNum = 4;
  _pumpNum = 2;
  
  // Extract temperatures
  _rcvBufferIdx = 9;
  _septetInject(_rcvBuffer, _rcvBufferIdx, 4);
  _temp[0] = _calcTemp(_rcvBuffer[_rcvBufferIdx + 1], _rcvBuffer[_rcvBufferIdx]);
  // ... extract more data
}
```

### Adding a New Protocol

1. Add enum value to `ProtocolType` in header
2. Create protocol handler functions:
   - `void _myProtocolSyncHandler()`
   - `void _myProtocolReceiveHandler()`
   - `void _myProtocolDecodeHandler()`
3. Add case in `loop()` method
4. Implement frame parsing and validation
5. Update documentation and examples

## Thread Safety

⚠️ **Not Thread-Safe**: This library is designed for single-threaded Arduino environment.

- Do not call methods from interrupts
- Do not call methods from multiple tasks (if using RTOS)
- Use appropriate synchronization if needed in advanced scenarios

## Best Practices

### For Users
1. Call `loop()` frequently (every iteration of Arduino loop)
2. Check `isReady()` before reading data
3. Monitor `getVbusStat()` for communication health
4. Don't block in your loop - keep it fast

### For Contributors
1. Always validate buffer indices before access
2. Use `MAX_BUFFER_SIZE` constant, not magic numbers
3. Clear buffers on state transitions
4. Update documentation when adding features
5. Maintain backward compatibility in public API

## Performance Characteristics

| Operation | Time Complexity | Space Complexity |
|-----------|----------------|------------------|
| loop() with no data | O(1) | O(1) |
| loop() with frame | O(n) where n=frame size | O(1) |
| getData methods | O(1) | O(1) |
| Frame parsing | O(n) where n=frame size | O(1) |

Memory usage: ~800 bytes per instance (mostly the 255-byte receive buffer and 32-element data arrays)
