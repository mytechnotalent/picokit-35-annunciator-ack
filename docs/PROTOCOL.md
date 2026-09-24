# Protocol

## Transport

The node and the gateway each carry an RYLR998 LoRa module. The node uses
UART1 (GP8/GP9); the gateway uses the second module on the computer USB serial
port. The modules are configured with AT commands:

```
AT+ADDRESS=<node>
AT+NETWORKID=<net>
AT+BAND=<hz>
AT+PARAMETER=<sf>,<bw>,<cr>,<preamble>
AT+SEND=<dest>,<len>,<payload>
+RCV=<src>,<len>,<data>,<rssi>,<snr>
```

## Payload

The payload is the lowercase hex encoding of a sealed envelope. The plaintext
telemetry is JSON:

```
{"n":35,"s":12,"s":1}
```

where `n` is the node id, `s` is the monotonic sequence number, and the second `s` is the latched annunciator state.

## Behaviour

The gateway sends the plaintext notice `ALARM` to the node. The node latches the annunciator and drives the red lamp; the button or the remote `0x45` key clears the latch and drives the green lamp.

## Envelope

```
nonce(24) | ciphertext | tag(16)
```

The nonce is random per frame. The associated data is the single byte node id.
The envelope is hex encoded for the AT payload path.

## Key

The field key is 32 bytes derived with Argon2id:

- passphrase: `picokit field key v1`
- salt: `picokit-salt-0001` (16 bytes)
- time cost 3, parallelism 1, memory 64 blocks

In production the key is provisioned per device through OTP; the passphrase
here is a lab default.

## AEAD and anti-replay

The payload is sealed with XChaCha20-Poly1305. Every frame carries a monotonic
sequence number per node; the receiver rejects any frame whose sequence is less
than or equal to the last accepted sequence, so a captured frame cannot be
replayed.

## Binary frame codec

The gateway also understands a binary frame used for commands:

```
SYNC(2) | VER(1) | TYPE(1) | NODE_ID(1) | SEQ(4) | NONCE(24) | CT | TAG(16)
```

Types are `TELEMETRY`, `COMMAND`, `ACK`, `ALARM`, `CONFIG`, and `HELLO`. The
nine byte header is authenticated as associated data.
