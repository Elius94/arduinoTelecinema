# Telecinema Arduino Controller (Main unit)
This is the firmware of the main controller unit of a Film Acquiring system.
This machine is based on an old modded projector with a Mirrorless Camera istead of the projector lens.
The moving of the projector is done with a stepper motor connected to the original pulley.
A magnetic on/off switch is placed in front of the shutter rotative disc that close and reopen the frame window. 
With a magnet fixed on one of the shutter arms the sensor can trigger for the frame presence.
An output GPIO pin connected to an opto isolator circuit simulate the remote camera trigger button to make it shoot.
The frame presence pin could not be really in sync with the correct frame position, so you can introduce a delay with the param 'CMD_SET_SHOOT_DELAY' (milliseconds).
This main controller unit is controllable with a serial port that can be the built in USB or the tx0, rx0 (uart) of the Arduino.

## Hardware required
* Arduino Uno or any other compatible board
* Stepper motor (correct torque)
* Stepper motor driver (with dir, step and enable pins, not H bridge)
* Magnetic on/off switch (to detect if the frame window is open)
* Power supply (12V to 5V)
* Optocoupler (to fake the camera remote button press)
* Serial port (to communicate with the Arduino)

## Serial commands (USB or uart0)
All the commands follows this pattern:

```sh
[COMMAND_NUMBER]:[VALUE]
```

This is the list of commands that the Arduino can receive:

```c++
enum serial_commands
{
    CMD_SET_SPEED = 0, 
    CMD_SET_DIRECTION = 1,
    CMD_SET_MODE = 2,
    CMD_SET_RUN = 3,
    CMD_GET_STATUS = 4,
    CMD_GET_SPEED = 5,
    CMD_GET_DIRECTION = 6,
    CMD_GET_MODE = 7,
    CMD_GET_RUN = 8,
    CMD_GET_FRAME_COUNT = 9,
    CMD_GO_TO_FRAME = 10,
    CMD_SET_SHOOT_DELAY = 11,
    CMD_RESET = 12
};
```

And this is the values of some of the commands:
```c++
// CMD_SET_RUN -> 0: stop, 1: run

enum status_codes
{
    STATUS_IDLE = 0,
    STATUS_RUNNING = 1,
    STATUS_ERROR = 2
};

enum direction_codes
{
    DIRECTION_FORWARD = 0,
    DIRECTION_BACKWARD = 1
};

enum mode_codes
{
    MODE_FREE = 0,
    MODE_CAMERA = 1
};

```

So, for example, to turn on the acquisition, you can send the following commands:

```sh
$machine> Welcome to the telecinema machine!
$you> 0:80
$machine> OK - Speed set to 80
$you> 2:1
$machine> OK - Mode set to camera
$you> 3:1
$machine> OK - Run set to on
```
