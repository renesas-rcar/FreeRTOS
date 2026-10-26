## rcar_rpmsg sample applications

Three samples of the `rcar_rpmsg` API (inter-core messaging between two
CR52 cores).

| Sample | Core 0 | Core 1 | What it shows |
|---|---|---|---|
| Basic | `rcar_rpmsg_core0` | `rcar_rpmsg_core1` | FE sends 20 messages on two channels, BE prints them |
| Echo (X5H only) | `rpmsg_mfis0_cluster0_core0` | `rcar_rpmsg_echo_core1` | `rcar_rpmsg` FE talking to the OpenAMP `rpmsg-echo` sample |
| API test | `rcar_rpmsg_api_test_core0` | `rcar_rpmsg_api_test_core1` | Runs test cases on every API, prints PASS/FAIL |
| UCIe echo | `rcar_rpmsg_ucie_rc` (MDP X5H) | `rcar_rpmsg_ucie_ep` (MDP AIACC) | Two chips over UCIe CH0: RC sends 20 messages, EP echoes them; the X5H also runs a local channel to `rcar_rpmsg_core1` |

### How to build

Follow the project [Readme.md](../../Readme.md) and add
`-DENABLE_OPENAMP=1` to the CMake command: it builds `rcar_rpmsg` and these
samples, together with the OpenAMP samples (`rpmsg_sample` with the Echo
peer `rpmsg-echo`, `virtio_*`).

Output: `build/rcar_rpmsg/<name>.elf` (and `.srec`). Each core has its own
ELF: X5H core 0 / core 1 load in VRAM1 / VRAM2, AIACC in DRAM3 / DRAM.
On AIACC the Echo sample is not built (its `rpmsg-echo` peer is X5H only).

### How to run

Load the ELFs and start both cores at the same time. The samples wait a few
seconds after start-up before opening channels.

#### Basic

| Core 0 | Core 1 |
|---|---|
| `rcar_rpmsg_core0` | `rcar_rpmsg_core1` |

Expected log on core 1:

```
[BE smmu] request: hello smmu 0
...
[BE smmu] request: hello smmu 19
[BE camera] request: hello camera 0
...
[BE camera] request: hello camera 19
```

All 20 messages of each channel, in order.

#### Echo

| Core 0 | Core 1 |
|---|---|
| `rpmsg_mfis0_cluster0_core0` (from `sample_apps/rpmsg_sample`) | `rcar_rpmsg_echo_core1` |

Expected log on core 1, every 2 seconds:

```
[FE] sent: hello
[FE] received:  CR52/Free-RTOS!
```

Core 0 prints `Incoming msg:` with an empty text. Both are expected: the two
samples use different message formats, the connection itself works.

#### API test

| Core 0 | Core 1 |
|---|---|
| `rcar_rpmsg_api_test_core0` | `rcar_rpmsg_api_test_core1` |

Each core prints one line per test case and a result line:

```
[FE] T05 smmu round trip: PASS
...
=== FE RESULT: 39/39 PASS ===
```

```
=== BE RESULT: 14/14 PASS ===
```

Lines marked `[INFO]` are measurements, not test cases. The test closes its
channels at the end: reboot both cores before running it again.

Test cases. Core 0 (FE) drives the tests on two channels: `smmu`
(interrupt-driven) and `camera` (polled). Core 1 (BE) echoes every message
back, and on command sends a burst, stops reading or closes its channel.

| ID | Core | Checks |
|---|---|---|
| P01–P05 | both | `open()` rejects a NULL channel, a bad side, a `driver_id` with no route (`RCAR_RPMSG_ID_4`), a 32-character name and an empty name: `RCAR_ERR_INVALID_PARAM` |
| P06–P08 | both | `send()`, `receive()`, `close()` on a channel never opened: `RCAR_ERR_NOT_INITIALIZED` |
| P09–P11 | both | `send()`, `receive()`, `close()` with a NULL channel: `RCAR_ERR_INVALID_PARAM` |
| B01 | BE | `open()` of smmu returns once FE is up |
| T01–T02 | FE | `open()` of smmu and camera succeed |
| T03 | FE | smmu returns `RCAR_ERR_NOT_CONNECTED` until the peer binds, then sends (bind time printed) |
| T04 | FE | camera sends at once, no binding step |
| T05–T06 | FE | Message echoed back unchanged, on smmu and on camera |
| T07 | FE | 0-byte message echoed back |
| T08 | FE | 120-byte message (maximum) echoed back unchanged |
| T09 | FE | 121-byte send: `RCAR_ERR_PAYLOAD_TOO_LARGE` |
| T10 | FE | `send()` with NULL data and length 5: `RCAR_ERR_INVALID_PARAM` |
| T11 | FE | `receive()` with NULL buffer and size 5: `RCAR_ERR_INVALID_PARAM` |
| T12 | FE | `receive(NO_WAIT)` with nothing pending: `RCAR_ERR_TIMEOUT` at once |
| T13 | FE | `receive(50 ms)` on smmu with nothing pending: `RCAR_ERR_TIMEOUT` after 50–70 ms |
| T14 | FE | Same on camera (polled channel) |
| T15 | FE | 10-byte message into a 4-byte buffer: `RCAR_ERR_PAYLOAD_TOO_LARGE`, first 4 bytes copied |
| T16 | FE | `receive()` with `out_len` NULL works |
| T17 | FE | 3 messages arrive before FE reads: only the last one (`"b3"`) is received |
| T18 | FE | BE stops reading, FE sends until `RCAR_ERR_BUFFER_FULL` (count printed, about 256) |
| T19 | FE | Sending works again once BE reads |
| T20 | FE | BE closes smmu: `receive()` returns `RCAR_ERR_PEER_DISCONNECTED` |
| T21 | FE | `on_unbind` is called with `cfg.ctx` |
| T22 | FE | `send()` after BE closed: `RCAR_ERR_PEER_DISCONNECTED` |
| B02 | BE | `close()` of smmu succeeds |
| B03 | BE | `open()` of smmu again: `RCAR_ERR_NOT_INITIALIZED` (closing the last channel released it) |
| T23 | FE | `close()` of smmu succeeds |
| T24 | FE | `close()` of smmu a second time: `RCAR_ERR_NOT_INITIALIZED` |
| T25 | FE | `open()` of smmu again: `RCAR_ERR_NOT_INITIALIZED` |
| T26 | FE | camera still works after smmu was released |
| T27 | FE | `close()` of camera succeeds |
| T28 | FE | `open()` of camera again: `RCAR_ERR_NOT_INITIALIZED` |
| [INFO] | FE | Average and maximum round-trip time over 20 messages, smmu vs camera |

#### UCIe echo (+ local channel on the X5H)

One source, `rcar_rpmsg_ucie_sample.c`, role by `BOARD`. Two chips: build
`rcar_rpmsg_ucie_rc` with `-DBOARD=mdp_x5h_hil` and
`rcar_rpmsg_ucie_ep` with `-DBOARD=mdp_aiacc_hil` (RFS:
`-DBOARD=x5h_rfs2` and `-DBOARD=mdp_aiacc_rfs2`, ELF only).

| Chip | Core 0 | Core 1 (optional) |
|---|---|---|
| X5H | `rcar_rpmsg_ucie_rc`: UCIe FE (`RCAR_RPMSG_ID_6`) + local FE (`RCAR_RPMSG_ID_0`) | `rcar_rpmsg_core1`: local BE |
| AIACC | `rcar_rpmsg_ucie_ep`: UCIe BE (`RCAR_RPMSG_ID_6`) + local FE (`RCAR_RPMSG_ID_0`) | `rcar_rpmsg_core1`: local BE |

Core 0 of each chip runs both links at the same time. `UCIE_SAMPLE_LOCAL 0`
in `rcar_rpmsg_ucie_sample.c` turns the local channel off (UCIe only). Each link has its own rsc
table and shared memory, placed by its rsc table linker script (extra
arguments of `add_rcar_rpmsg_app()`):

| Link | Linker script | Shared memory | Notify |
|---|---|---|---|
| Local (CR <-> CR/CA, same chip) | `lscript_rsc_table_rcar_rpmsg.ld` | 0x96600000 (`SHARE_CR_CA`) | MFIS |
| UCIe CH0 (X5H CR <-> AIACC CR) | `lscript_rsc_table_rcar_rpmsg_ucie0.ld` | 0x9E600000 (`UCIE_SHARE`), AIACC DRAM | polling |

An app links the scripts of the links it uses; `open()` of a `driver_id`
whose link is not linked returns `RCAR_ERR_TRANSPORT`.

UCIe link, done by `rcar_rpmsg` (as `sample_apps/ucie/ucie_rc.c` +
`ucie_ep0.c`):

- AIACC: its system task links CH0 up as EP at boot (`init_with_system =
  true`). `open()` waits for it, then maps iATU inbound region 0:
  0x200_0000_0000 to 0x9E600000.
- X5H: the first `open()` links CH0 up as RC (same steps as
  `ucie0_setup_task`), then enables SMMU_RT: 0x9E600000 (2 MB) to the UCIe
  window 0x200_0000_0000, the rest of 0x0-0xFFFFFFFF identity mapped (the
  local shared memory 0x96600000 included).
- RFS: iATU outbound region 0 on the X5H, 0x9E600000 to 0x9E600000.
- Both sides then shake hands: the X5H only reads the UCIe rsc table once
  the AIACC has written it.

Not 0x96600000 for UCIe: that is the CR-CA shared memory, the echo stops
after a few messages there.

Rules on a chip using `RCAR_RPMSG_ID_6`:

- X5H: do not call `R_UCIE_Config()` for CH0 and do not use SMMU_RT or
  iATU region 0 of CH0: `rcar_rpmsg` owns them.
- On UCIe, one chip opens FE channels only, the other BE channels only.

Start the AIACC (core 1, then core 0) and X5H core 1 first, then X5H
core 0. Expected log on X5H core 0:

```
[RC] endpoint ready
[RC] sent: hello ucie 0
[RC] received: hello ucie 0 (OK)
[LOCAL] endpoint ready
[LOCAL] sent: hello smmu 0
...
=== RC RESULT: 20/20 echoed ===
=== LOCAL RESULT: 20/20 sent ===
```

on the AIACC core 0, for each message: `[EP] request: hello ucie 0`, and
also `=== LOCAL RESULT: 20/20 sent ===`; on core 1 of each chip:
`[BE smmu] request: hello smmu 0` .. `19`.

Without core 1, core 0 prints `[LOCAL] ... skipped` (or 0/20 sent) and the
UCIe echo still runs. On a HIL board both cores of a chip share one UART:
their lines interleave. If the link or the AIACC is not up within 10 s,
X5H core 0 prints `rcar_rpmsg_open failed (9), retry` and tries again.
Reboot both chips before running it again: the EP does not answer a second
start-up of the RC.

### Notes

- Run a sample on both cores built from the same source revision.
- If a core stops at start-up, start core 1 first, then core 0.
- Timing cases (T13, T14) assume a 1 ms FreeRTOS tick.
