# ULF_DECUP_EIN


ULF_DECUP_EIN is a protocol for updating both firmware and sound data on ZIMO MX decoders over the PC. This protocol gives direct access to the MXULF track outputs. Data sent by the host PC is used as input for the so-called unipolar track signal. After each packet, the MXULF checks if the decoder has given a response to the packets, and the response is sent back to the host.

It is up to the host the manage the update process in such a way that a sound or firmware update is successful. For this a description of the basic layout of a data packet, the used commands, and the usual workflow of a sound or firmware update is provided in the following paragraphs.

## Unipolar data packet

Unlike DCC, the unipolar track signal utilizes only a single track output. As the name implies, it is unipolar. Low is equal to zero Volt, and High is equal is +15 Volt.
Each byte is made up of a logical zero startbit, followed by the regular zero and one data bits starting with the LSB. A logical one stop bit marks the end of the byte transmission. Stopbit length should differ depending on whether a firmware or a sound update is done.

| Regular Bit | Stopbit Sound | Stopbit SW |
|---|---|---|
| 25 µs | 50 µs | 200 µs |

After each packet, the decoder can give a response. There three response types:

| ACK | NACK | No response |
|---|---|---|
| long response| short response | nothing

## Firmware update packets

### Firmware Update Preamble

???

| 239 | 191 |
| ---| --- |

### Startbyte
The so called startbyte contained in the zsu updatefile is sent as a byte packet to select the given decoder. ACK is expected for a successful selection.

### Blocklength byte

Contains the number of flashblocks to be written. Both ACK and NACK are accepted.

### Security Byte 1

Byte 0x55

### Security Byte 2

Byte 0xAA

### Data
The main firmware data packet is made up of the block count, the data frame, and and XOR checksum for the preceding bytes.

| Blockcount Byte | 32 or 64 bytes data frame | XOR |
| --- | --- | --- |

## Sound update packets

### Preamble

| 191 | 239 | 239 |
| --- | --- | --- |

### Read CV

| Cmd Byte | Address high | Address low |
| --- | --- | --- |
| 1 |  | |

### Write CV

| 6 | 0xAA | Address high | Address low | Checksum CRC8 | Value |
| --- |--- |--- |--- |--- |--- |
| 2 | 0xAA | Address high | Address low | Value |

### Delete Flash

| 3 | 85 | 255 | 255 |
| --- | --- | --- | --- |

### Read Decoder Type

| 4 |
| ---|

Response is interpreted as single byte value. Valid response ( != 0) marks the completion flash memory erasure.

### CRC or XOR Query

| 7 |
| ---|

Response is interpreted as single byte value. If the read value equals 1, the decoder will expect a CRC8 checksum, otherwise an XOR checksum has to be calculated.

### Data Packet

| Command 5 | 85 | Blocknumber high | Blocknumber low | 256 bytes sound data | CRC8 |
| --- |--- |--- |--- |--- |--- |
| Command 3 | 85 | Blocknumber high | Blocknumber low | 256 bytes sound data | XOR |


=======
<img src="data/images/logo.png" width="20%" align="right">

