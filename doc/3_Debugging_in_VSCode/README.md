# Debugging & VSCode

## Table of Contents

- [Debugging & VSCode](#debugging--vscode)
  - [Table of Contents](#table-of-contents)
   - [Interpreting build errors and compiler output](#interpreting-build-errors-and-compiler-output)
      - [Read the output from the top down](#read-the-output-from-the-top-down)
      - [Example: one error with several messages](#example-one-error-with-several-messages)
      - [Recognize common kinds of output](#recognize-common-kinds-of-output)
      - [Examples from Zephyr builds](#examples-from-zephyr-builds)
      - [Useful strategies](#useful-strategies)
  - [Setup](#setup)
  - [Introduction](#introduction)
  - [Breakpoint debugging](#breakpoint-debugging)
    - [Before you start](#before-you-start)
    - [Opening the debugger](#opening-the-debugger)
    - [What happens when the debugger starts?](#what-happens-when-the-debugger-starts)
    - [First breakpoint exercise](#first-breakpoint-exercise)
  - [Serial debugging](#serial-debugging)
    - [How does it work?](#how-does-it-work)
    - [Using the VSCode serial monitor](#using-the-vscode-serial-monitor)
      - [How to use it](#how-to-use-it)
      - [Exercise: combine serial output with breakpoints](#exercise-combine-serial-output-with-breakpoints)
      - [Example: finding a null string pointer](#example-finding-a-null-string-pointer)
   - [Challenge Exercise](#challenge-exercise)
   - [References](#references)

## Interpreting build errors and compiler output

Build output can be noisy. Read it from the top down and find the first error before
investigating later messages. One mistake can produce several follow-on diagnostics.
### Read the output from the top down

Use this process when a build reports errors or warnings:

1. Find the first line that says `error` or otherwise reports a failed command. Ignore
    progress messages such as `Building`, `[n/n]`, and linker status messages while
    locating it.
2. Read the source location beside the message. It usually includes a file name, line
    number, and sometimes a column, for example:
    ```text
    app/src/main.c:24:9: error: expected ';' before '}' token
    ```
3. Open that file and inspect the reported line and the few lines above it. The actual
    mistake is often just before the highlighted location, such as a missing semicolon,
    brace, parenthesis, or quotation mark.
4. Read the explanation after the location. Identify what the compiler expected and
    what it found instead. This is usually more useful than the final build summary.
5. Fix one small problem, then build again. Treat the next build as a test of whether
    your interpretation was correct.

### Example: one error with several messages

The following is a shortened example of compiler output from a Zephyr application:

```text
[42/143] Building C object app/CMakeFiles/app.dir/src/main.c.obj
FAILED: app/CMakeFiles/app.dir/src/main.c.obj
.../app/src/main.c:18:5: error: expected ';' before 'k_msleep'
   18 |     counter = counter + 1
      |     ^~~~~~~
.../app/src/main.c:19:5: error: 'counter' undeclared (first use in this function)
   19 |     k_msleep(1000);
      |     ^~~~~~~~
.../app/src/main.c:19:5: warning: implicit declaration of function 'k_msleep'
ninja: build stopped: subcommand failed.
```

Start with line 18. The missing semicolon can make the next line look invalid, so the
`counter` and `k_msleep` messages may be follow-on diagnostics. Add the semicolon and
rebuild. If the `k_msleep` warning remains, include `<zephyr/kernel.h>`. The final
`ninja` message only reports that an earlier command failed; it is not the root cause.

### Recognize common kinds of output

- **Compiler errors** stop a source file from compiling. Common causes are syntax
   mistakes, undeclared variables, incompatible types, and missing headers.
- **Linker errors** occur when object files are combined. `undefined reference` usually
   means an implementation, library, or source file is missing from the build.
- **Warnings** may expose real bugs even when the build continues. Investigate unused
   variables, missing return values, implicit conversions, and signed/unsigned
   comparisons instead of hiding them with a cast or compiler option.
- **CMake, Kconfig, and device-tree messages** come from Zephyr configuration. Check
   `CMakeLists.txt`, `prj.conf`, board configuration, or `.dts` files as appropriate.

### Examples from Zephyr builds

Wording varies by Zephyr version and board, but these messages are common:

- **Missing Zephyr header:**
   ```text
   fatal error: zephyr/kernel.h: No such file or directory
   ```
   Build with `west build` from the application directory and check that the source
   file is included through the project's CMake setup. Zephyr normally supplies its
   include paths through CMake, so avoid adding a guessed path.
- **Missing API declaration:**
   ```text
   error: implicit declaration of function 'k_msleep'
   ```
   Include the API's header: usually `<zephyr/kernel.h>` for kernel APIs and
   `<zephyr/sys/printk.h>` for `printk`. Check for an earlier missing-header error too.
- **Kconfig symbol was not defined:**
   ```text
   warning: attempt to assign the value 'y' to the undefined symbol SENSOR
   ```
   Check the symbol's spelling and its module or driver `Kconfig` file. It may require
   a parent option or may not be available for the selected board. Inspect the final
   configuration in `build/zephyr/.config`.
- **Device-tree node is unavailable:**
   ```text
   error: '__device_dts_ord_...' undeclared
   ```
   The code is requesting a device missing from the generated device tree. Check the
   node's `status`, `compatible` value, pin configuration, and driver setting in
   `prj.conf`. For an alias or chosen node, verify the overlay name matches the `DT_*`
   macro.
- **A device was declared but its driver is not linked:**
   ```text
   undefined reference to `__device_dts_ord_...'
   ```
   The node is present, but its driver may be disabled or unavailable. Compare its
   `compatible` value with the driver's Kconfig option, then check the generated
   device tree and `.config` file.
- **The firmware is too large:**
   ```text
   region `FLASH' overflowed by ... bytes
   ```
   The image does not fit in flash. Look for unnecessary features in `prj.conf`, large
   logging settings, unused source files, or an oversized stack. This is a linker
   error, so changing C syntax will not solve it.
- **An application source file was not included:**
   ```text
   undefined reference to `my_function'
   ```
   The function may be declared in a header but defined in a source file outside the
   target. Check `app/CMakeLists.txt` and `zephyr_library_sources*` statements.

### Useful strategies

- Read the complete diagnostic, including `note:` lines that identify a declaration
   or definition.
- Inspect the surrounding code, then check the reported symbol's spelling, type,
   headers, and build inclusion.
- Fix the earliest error and rebuild before making more changes; later messages may
   disappear.
- Separate build failures from flashing and runtime failures. A successful build does
   not prove that the board is connected, firmware was flashed, or behavior is correct.
- After configuration changes, use a clean rebuild if the output seems stale.

The same top-down habit is useful in the VS Code **Terminal** and **Problems** panels.
Use the Problems panel to jump to a source location, but use the terminal output when
you need the full context of the command and the messages that came before it.

## Setup

1. Go to your local copy of the main branch using `git checkout main`
2. Create a new branch for lesson 3 using `git checkout -b lesson-3`

## Introduction

This lesson aims to cover two key topics of embedded systems: debugging via serial interface, and debugging using breakpoints. Both these topics are crucial, and you'll probably find yourself using them in some
way or another for every future lesson!

## Breakpoint debugging

Breakpoint debugging lets you pause the program while it is running and inspect what the
processor is doing. You can stop at a specific line, look at variable values, and execute the
program one line at a time. This is especially useful when the program reaches an unexpected
state or when serial messages do not explain what went wrong.

### Before you start

Before launching the debugger:

1. Connect the nRF52840 development kit to your computer with the USB cable.
2. Make sure the board is recognized by the J-Link tools.
3. Check that the project builds successfully. The debugger uses the ELF file generated at
    `build/zephyr/zephyr.elf`.
4. Open `.vscode/launch.json` and configure the `gdbPath` property. This property must point to
    the `arm-zephyr-eabi-gdb` executable installed with your Zephyr SDK.

The path is different depending on your operating system and where you installed the SDK. For
example, the setting may look similar to one of these:

The path is different depending on your operating system and where you installed the SDK. For
example, the setting may look similar to one of these:

```jsonc
// Windows: use either / or escaped backslashes in JSON paths.
"gdbPath": "C:/path/to/zephyr-sdk/arm-zephyr-eabi/bin/arm-zephyr-eabi-gdb.exe"

// Linux
"gdbPath": "/home/your-name/zephyr-sdk/arm-zephyr-eabi/bin/arm-zephyr-eabi-gdb"

// macOS
"gdbPath": "/Users/your-name/zephyr-sdk/arm-zephyr-eabi/bin/arm-zephyr-eabi-gdb"
```

Do not copy these paths literally. Find the `arm-zephyr-eabi-gdb` file in your own Zephyr SDK
installation and use its full path. On Windows, include `.exe` if it is present. The existing
line in `launch.json` is commented out, so remove the `//` and update the path before starting
the debugger.

![.vscode/launch.json](imgs/launch_json_screenshot.png)

### Opening the debugger

1. Click the **Run and Debug** icon in the VS Code activity bar. It looks like a play button next
    to a bug.
2. Select **Cortex Debug** in the configuration dropdown if it is not already selected.
3. Click the green **Start Debugging** button, or press `F5`.

![The VS Code Run and Debug view](imgs/Run&Debug_page.png)

The first launch may build and flash the application because this configuration runs the
`Build and Flash app` task before starting the debugger. Wait for that task to finish and for the
debugger to connect to the board.

### What happens when the debugger starts?

The launch configuration contains several options that control startup:

- `preLaunchTask` builds and flashes the application before debugging begins.
- `executable` tells the debugger which compiled ELF file contains the program and debug symbols.
- `device` identifies the nRF52840 microcontroller.
- `servertype: "jlink"` tells Cortex-Debug to use the J-Link debug server built into the board's
   debug interface.
- `runToEntryPoint: "main"` runs the program until it reaches `main`, then pauses there. This
   gives you a useful starting point without needing to place a breakpoint first.
- `svdFile` allows VS Code to show the microcontroller's peripheral registers, when the SVD file
   can be found.
- `rtos: "Zephyr"` enables Zephyr-aware RTOS information in the debugger.

When the program is paused, the **Debug toolbar** gives you these common controls:

- **Continue** (`F5`): resume execution until the next breakpoint.
- **Step Over** (`F10`): execute the current line without entering a function it calls.
- **Step Into** (`F11`): enter the function called on the current line.
- **Step Out** (`Shift+F11`): finish the current function and stop when it returns.
- **Restart**: flash or restart the debug session according to the launch configuration.
- **Stop** (`Shift+F5`): end the debug session.

You can also click in the gutter to the left of a source-code line to add or remove a breakpoint.
The **Variables** panel shows local and global values, the **Call Stack** panel shows how the
program reached the current line, and the **Watch** panel lets you track an expression of your
choice.

![The VS Code Run and Debug view](imgs/debugger_running.png)

### First breakpoint exercise

Open `app/src/main.c` and place a breakpoint on a line inside `main`, such as the first line in
the `while` loop. Start the debugger and observe what happens when execution reaches that line.

1. Use **Continue** to run until the breakpoint is hit.
2. Use **Step Over** to execute one line at a time.
3. Inspect the current line, the **Variables** panel, and the **Call Stack** panel.
4. Move the breakpoint to another line and press **Continue** again.
5. Remove the breakpoint by clicking its red marker in the gutter.

If the debugger does not start, check the `gdbPath` first, then check that the board is connected
and that `build/zephyr/zephyr.elf` exists.

## Serial debugging

Debugging with a serial monitor is the embedded systems equivalent to having print statements
throughout your code, and can make otherwise incredibly difficult-to-understand bugs quite easy to
diagnose.

Some examples of where you may want to use a print statement:
- To see what point the program is getting to before it crashes/something goes wrong
- To see how often your program has to recover from a bad state
- To see how often your code has to react to an interrupt
- To check that it's doing internal processing as expected

### How does it work?

For now, our system involves sending data in one direction only. so there are two endpoints to
consider when working with our serial interface:
1. **The transmitter** (your development board in this case):
   Responsible for sending data to a receiver. Note that "transmit"/"transmitter" is commonly
   abbreviated to "Tx".
1. **The receiver** (your computer in this case):
   Responsible for listening to incoming data from a transmitter. Note that "receive"/"receiver" is
   commonly abbreviated to "Rx".

It's called a "serial" interface because it sends data in serial: one bit after another in a strict
order (note that this is where USB gets its name: Universal Serial Bus).

Now you might be asking: "how do words get converted to bits?" The answer is *ASCII* (for the most part).

ASCII (American Standard Code for Information Interchange) is an encoding standard for converting
numbers, Latin alphabet letters and other common symbols to binary, here's a handy table:

![ASCII character table](imgs/ASCII.png)
(Table from ZZT32 and is in the public domain)

When you use a debug print statement with a serial interface, it converts the text you give to
binary and transmits it in order to the receiver, who then decodes it back to text using the same
standard and displays it to a *serial monitor*. A serial monitor is a program that runs on your
computer and displays incoming serial data from a specific port.

### Using the VSCode serial monitor

All serial monitors effectively do the same thing, so you're welcome to use any serial monitor you
like if you already have a preference. We recommend using the VSCode serial monitor otherwise.

1. Open the VSCode Panel, and click on the Serial Monitor tab.
   If the tab is missing, go back to [lesson 1](../1_Getting_Started/getting_started.adoc) and follow the VScode setup instructions.
   
   ![VS Code Serial Monitor tab](imgs/SerialMonitorExtension.png)
2. Open the serial monitor:
   - Select the COM port that your dev board is plugged into. It will include `JLink` or `Segger` in the port name.
   - Set the baud rate to 115200
   - Select "Start Monitoring"
   ![Serial Monitor settings](imgs/SerialMonitorSettings.png)

### How to use it

As a brief note: on Zephyr, there are two options for serial printing:
1. printf:
   A standard library function that supports many formatting options, but has a large memory
   footprint. Included in <stdio.h>
2. `printk`:
   An operating system (kernel) function that only allows basic formatting, but is much more
   lightweight. Included in `<zephyr/sys/printk.h>`.
We'll typically recommend using printk.

A typical debug statement might look something like:
```c
printk("foo got bar as: %d", bar);
```

### Exercise: combine serial output with breakpoints

After completing the serial debugging lesson, add several `printk` statements to `main` with
different messages. For example, print a message before a calculation, another after the
calculation, and a third inside a loop.

1. Build and flash the program, then open the serial monitor.
2. Place a breakpoint on the line between the first and second `printk` statements.
3. Start the debugger and observe which serial messages appear before the program pauses.
4. While paused, step over the next line and observe when the next message appears.
5. Move the breakpoint between different `printk` statements and repeat the experiment.
6. Remove the breakpoints and use **Continue** to let the program run normally. Compare the
   output with the output produced while the debugger pauses execution.

Record what you observe. In particular, note that a breakpoint stops the processor, so later
`printk` statements do not run until you continue or step the program. A breakpoint can also
change the timing of a program, which matters for timing-sensitive code.

### Example: finding a null string pointer

The debugger can reveal problems that are difficult to understand from serial output alone.
For example, this code intends to print a useful status message, but leaves `status` as a
null pointer when the button is not pressed:

```c
const char *status = NULL;

if (button_pressed) {
    status = "Button pressed";
}

printk("Status: %s\n", status);
```

Depending on the C library and console implementation, printing a null `%s` argument may
display `(null)`, print unexpected text, or cause a fault. If the expected output is
`Status: Idle`, the serial output tells you that something is wrong but not why.

To investigate it:

1. Place a breakpoint on the `printk` line.
2. Start the debugger and inspect `status` in the **Variables** panel.
3. Check the **Call Stack** to confirm that execution reached the print statement from the
   expected path.
4. Step backward through the conditional logic or move the breakpoint to the assignment
   and check whether `button_pressed` has the value you expect.
5. Fix the missing default value, for example by initializing `status` to `"Idle"`, then
   rebuild, flash, and compare the serial output.

The important observation is that the debugger shows the pointer value directly. A null
pointer is a program-state problem; changing the `printk` format string only hides the
symptom.

## Challenge Exercise

Run
```sh
git fetch upstream
```
and
```sh
git checkout linked-list
```

This branch contains a simple implementation of a doubly linked-list.
If you are not familiar a linked list is a data structure where each element contains a pointer to the next element of the list.
In a doubly linked list each element also contains a pointer to the prior element.
Typically the head and tail node will have their prior and next elements point to null respectively.

The image below shows a doubly linked list
![](imgs/Doubly-linked-list.svg)
By <a href="//commons.wikimedia.org/w/index.php?title=User:Lasindi&amp;action=edit&amp;redlink=1" class="new" title="User:Lasindi (page does not exist)">Lasindi</a> - <span class="int-own-work" lang="en">Own work</span>, Public Domain, <a href="https://commons.wikimedia.org/w/index.php?curid=2245165">Link</a>

In `main.c` you will find a program that is creating a linked list, adding, and removing data from it, and printing the list to the console.
However, if you try to build and flash the program you will find the program hard faults.
Use the VSCode debugger to debug this program and make it work.

## References

- [`k_msleep()` Zephyr API documentation](https://docs.zephyrproject.org/latest/doxygen/html/group__thread__apis.html)
- [`printk()` Zephyr API documentation](https://docs.zephyrproject.org/latest/doxygen/html/printk_8h.html)
