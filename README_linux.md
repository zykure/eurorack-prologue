Eurorack Oscillators for Korg prologue, minilogue xd and Nu:tekt NTS-1
=================================

Building on Linux
====

1. Clone this repository:

    ```
    git clone https://gitlab.com/zykure/eurorack-prologue.git
    ```

   Then navigate into the project folder.

2. Clone git submodules:

    ```
    git submodule --init --remote --recursive
    ```

   This will fetch the `logue-sdk` from KORG (tm) and the `eurorack` modules from Mutable Instruments (tm).

3. To use the logue-sdk, you need to download some additional files that are provided by KORG (tm) for Windows, MacOS and Linux.

    ```
    cd logue-sdk/tools/gcc && ./get_gcc_linux.sh && cd -
    cd logue-sdk/tools/logue-cli && ./get_logue_cli_linux.sh && cd -
    ```

4. Now you're ready to build the code. This should be as simple as running `make -j1` in the main project folder.

   Note that running the build in parallel (e.g. `make -j2`) is not supported and leads to corrupted object files.

   The main `Makefile` will build all the oscillators for the three devices (Prologue, Minilogue, NTS-1) and package them into corresponding units that are ready for transfer.

5. To transfer the units to your synthesizer, you can use the Librarian software from KORG (tm) or the CLI tools they provide. The `logue-cli` allows to probe, list, update, and clear oscillators on the device; see their [README.md](https://github.com/korginc/logue-sdk/blob/master/tools/logue-cli/README.md) for details.

   ```
   ./logue-cli probe                       # list devices
   ./logue-cli probe -i2 -o2 -m osc        # list current slots on device 2
   ./logue-cli clear -i2 -o2 -m osc -s0    # clears the 1st slot on device 2
   ./logue-cli load -i2 -o2 -s0 -u <FILE>  # uploads to 1st slot on device 2
   ```

   The unit files have the endings `.prlgunit` for Prologue, `.mnlgxdunit` for Minilogue XD, `.ntkdigunit` for NTS-1.

   It happens occasionally that the `logue-cli load` command fails to upload to the device. In this case, the `logue-load.sh` script can be used. It extracts the raw MIDI SysEx data from the unit file, and then uses `amidi` for the transfer. This needs MIDI tools installed on your system.

   In some cases it also helps to increase the MIDI buffer size: `echo "65536" | sudo tee /sys/module/snd_seq_midi/parameters/output_buffer_size`

   Note: Make sure that no applications is blocking the MIDI ports on your system. You can check this e.g. with the `fuser -fv /dev/snd/*` command.

6. The `logue-cli` program sometimes fails to transfer data to the device. This typically results in a message

    ```
    MidiOutAlsa::sendMessage: error sending MIDI message to port.
    ```

    To circumvent the issue, one can use the `logue-cli` program to create a SysEx file which can then be transferred manually. The script `logue-load.sh` takes care of the necessary steps:

    ```
   ./logue-load.sh -s0 -u <FILE>  # uploads to 1st slot on device 2
    ```

    Note that the script automatically detects the correct device port, but this should be checked before transferring SysEx files.
