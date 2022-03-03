// Stepper motor control pins
#define motor_enable_pin 4
#define motor_dir_pin 5
#define motor_step_pin 6

// Sensors pins
#define frame_detect_pin 2 // Must be an interrupt pin
#define film_end_pin 3

// Output pins
#define led_pin 13
#define camera_pin 7

// Other parameters
#define serial_speed 9600
#define serial_ack "OK"
#define serial_nack "KO"

// Serial Commands Enum
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

// Parameters
int speed = 0;
int max_speed = 200;
int direction = DIRECTION_FORWARD;
int mode = MODE_FREE;
int run = 0;
int frame_count = 0;
int desired_frame = -1;
int status = STATUS_IDLE;
unsigned long last_step_time = 0;
unsigned long step_delay = 0;
unsigned long shoot_request_time = 0; // Time when the shoot request was sent
long shoot_delay = 0;                 // in milliseconds (0 = disabled)

void setPins()
{
    pinMode(motor_enable_pin, OUTPUT);
    pinMode(motor_dir_pin, OUTPUT);
    pinMode(motor_step_pin, OUTPUT);
    pinMode(frame_detect_pin, INPUT);
    pinMode(film_end_pin, INPUT);
    pinMode(led_pin, OUTPUT);
    pinMode(camera_pin, OUTPUT);

    // Configure Frame Detect pin as an interrupt
    attachInterrupt(digitalPinToInterrupt(frame_detect_pin), updateFrameCount, RISING);
}

bool setMotorDirection(int direction)
{
    if (direction == DIRECTION_FORWARD)
    {
        digitalWrite(motor_dir_pin, HIGH);
    }
    else if (direction == DIRECTION_BACKWARD)
    {
        digitalWrite(motor_dir_pin, LOW);
    }
    else
    {
        return false;
    }
    return true;
}

bool setMode(int _mode)
{
    if (_mode == MODE_FREE)
    {
        mode = MODE_FREE;
        return true;
    }
    else if (_mode == MODE_CAMERA)
    {
        mode = MODE_CAMERA;
        return true;
    }
    else
    {
        return false;
    }
}

bool setRun(int _run)
{
    if (_run == 1)
    {
        run = 1;
        return true;
    }
    else if (_run == 0)
    {
        run = 0;
        return true;
    }
    else
    {
        return false;
    }
}

bool setSpeed(int _speed) // 0-100 % of max speed that is in steps per second. The step_delay is the period between steps in microseconds.
{
    if (_speed > 0 && _speed <= 100)
    {
        speed = _speed;
        step_delay = 1000000 / (max_speed * speed / 100);
        return true;
    }
    else
    {
        speed = 0;
        step_delay = 1000000 / max_speed;
        return false;
    }
}

bool setTargetFrame(int frame_number)
{
    if (frame_number > 0)
    {
        desired_frame = frame_number;
        return true;
    }
    else
    {
        desired_frame = -1;
        return false;
    }
}

void setFrameCount(int _frame_count)
{
    if (_frame_count > 0)
    {
        frame_count = _frame_count;
    }
    else
    {
        frame_count = 0;
    }
}

void setLed(int led)
{
    if (led == 1)
    {
        digitalWrite(led_pin, HIGH);
    }
    else
    {
        digitalWrite(led_pin, LOW);
    }
}

void setStatus(int _status)
{
    if (_status == STATUS_IDLE)
    {
        status = STATUS_IDLE;
    }
    else if (_status == STATUS_RUNNING)
    {
        status = STATUS_RUNNING;
    }
    else
    {
        status = STATUS_ERROR;
    }
}

bool setShootDelay(int _shoot_delay)
{
    if (_shoot_delay > 0)
    {
        shoot_delay = _shoot_delay;
    }
    else if (_shoot_delay == 0)
    {
        shoot_delay = 0;
    }
    else
    {
        return false;
    }
    return true;
}

int getStatus()
{
    return status;
}

int getSpeed()
{
    return speed;
}

int getMotorDirection()
{
    return direction;
}

int getMode()
{
    return mode;
}

int getRun()
{
    return run;
}

int getFrameCount()
{
    return frame_count;
}

int getTargetFrame()
{
    return desired_frame;
}

// Interrupt callback function
void updateFrameCount()
{
    // Serial.println("Frame detected");
    if (getMotorDirection() == DIRECTION_FORWARD)
    {
        setFrameCount(getFrameCount() + 1);
    }
    else
    {
        setFrameCount(getFrameCount() - 1);
    }
    if (getMode() == MODE_CAMERA)
    {
        if (shoot_delay > 0)
        {
            shoot_request_time = millis();
        }
        else
        {
            shoot(); // Shoot the film
        }
    }
    // Serial.println(getFrameCount());
}

void shoot()
{
    digitalWrite(camera_pin, HIGH);
    delay(2);
    digitalWrite(camera_pin, LOW);
}

void setMotorEnable(int enable)
{
    digitalWrite(motor_enable_pin, enable);
}

/**
 * @brief The serial commands are received from the computer and processed here
 * The command is sent as a string, with the following format:
 * <command>:<parameter>
 * The command is an serial_commands enum, and the parameter is a 4 bytes integer
 * After parsing the commandm a switch case is used to process the command and parameter
 *
 * @return int
 */
int parseSerialCommands()
{
    String command;
    String parameter;
    int parameter_int;
    int command_int;

    command = Serial.readStringUntil(':');
    parameter = Serial.readStringUntil('\n');
    parameter_int = parameter.toInt();
    command_int = command.toInt();

    switch (command_int)
    {
    case CMD_SET_SPEED:
    {
        if (setSpeed(parameter_int))
        {
            Serial.print(serial_ack);
            Serial.println(" - Speed set to " + String(parameter_int));
        }
        else
        {
            Serial.print(serial_nack);
            Serial.println(" - Speed could not be set to " + String(parameter_int));
        }
    }
    break;
    case CMD_SET_DIRECTION:
    {
        if (setMotorDirection(parameter_int))
        {
            Serial.print(serial_ack);
            if (parameter_int == DIRECTION_FORWARD)
            {
                Serial.println(" - Direction set to forward");
            }
            else
            {
                Serial.println(" - Direction set to reverse");
            }
        }
        else
        {
            Serial.print(serial_nack);
            Serial.println(" - Direction could not be set");
        }
    }
    break;
    case CMD_SET_MODE:
    {
        if (setMode(parameter_int))
        {
            Serial.print(serial_ack);
            if (parameter_int == MODE_FREE)
            {
                Serial.println(" - Mode set to free");
            }
            else
            {
                Serial.println(" - Mode set to camera");
            }
        }
        else
        {
            Serial.print(serial_nack);
            Serial.println(" - Mode could not be set");
        }
    }
    break;
    case CMD_SET_RUN:
    {
        if (setRun(parameter_int))
        {
            Serial.print(serial_ack);
            if (parameter_int == 1)
            {
                Serial.println(" - Run set to on");
            }
            else
            {
                Serial.println(" - Run set to off");
            }
        }
        else
        {
            Serial.print(serial_nack);
            Serial.println(" - Run could not be set");
        }
    }
    break;
    case CMD_GET_STATUS:
        Serial.println(getStatus());
        break;
    case CMD_GET_SPEED:
        Serial.println(getSpeed());
        break;
    case CMD_GET_DIRECTION:
        Serial.println(getMotorDirection());
        break;
    case CMD_GET_MODE:
        Serial.println(getMode());
        break;
    case CMD_GET_RUN:
        Serial.println(getRun());
        break;
    case CMD_GET_FRAME_COUNT:
        Serial.println(getFrameCount());
        break;
    case CMD_GO_TO_FRAME:
        setTargetFrame(parameter_int);
        Serial.println(serial_ack);
        break;
    case CMD_SET_SHOOT_DELAY:
    {
        if (setShootDelay(parameter_int))
        {
            Serial.print(serial_ack);
            Serial.println(" - Shoot delay set to " + String(parameter_int));
        }
        else
        {
            Serial.print(serial_nack);
            Serial.println(" - Shoot delay could not be set to " + String(parameter_int));
        }
    }
    break;
    case CMD_RESET:
        setStatus(STATUS_IDLE);
        setSpeed(0);
        setMotorDirection(DIRECTION_FORWARD);
        setMode(MODE_FREE);
        setRun(0);
        setFrameCount(0);
        setTargetFrame(0);
        setShootDelay(0);
        Serial.print(serial_ack);
        Serial.println("Reset complete");
        break;
    default:
        return -1;
    }
    return 0;
}

void setup()
{
    setPins();
    // Enable serial communication
    Serial.begin(serial_speed);
    // Wait for serial port to be available
    while (!Serial)
    {
        delay(100);
    }
    // Print welcome message
    Serial.println("Welcome to the telecinema machine!");
}

void loop()
{
    // Check if serial commands are available
    if (Serial.available() > 0)
    {
        parseSerialCommands();
    }

    // Define the state machine
    switch (getStatus())
    {
    case STATUS_IDLE:
        setMotorEnable(0);
        setLed(0);
        if (getRun() == 1)
        {
            setStatus(STATUS_RUNNING);
        }
        break;
    case STATUS_RUNNING:
        if (getRun() == 1)
        {
            setMotorEnable(1);
            if (getMode() == MODE_CAMERA)
            {
                // Go Forward and move the motor until the last frame is reached
                setMotorDirection(DIRECTION_FORWARD);
                setLed(1);
                // check millis to see if it's time to rise a signal to the step pin (1ms)
                if (millis() - last_step_time >= step_delay)
                {
                    digitalWrite(motor_step_pin, HIGH);
                    delay(1);
                    digitalWrite(motor_step_pin, LOW);
                    last_step_time = millis();
                }
                // Check if it's time to shoot the film
                if (shoot_delay > 0 && shoot_request_time > 0)
                {
                    if (millis() - shoot_request_time >= shoot_delay)
                    {
                        shoot();
                        shoot_request_time = 0;
                    }
                }
            }
            else if (getMode() == MODE_FREE)
            {
                // Go to Direction and move the motor until the desired frame is reached or the last frame is reached
                if (getTargetFrame() > -1)
                {
                    // Find the direction to go to the target frame and set the motor direction
                    if (getFrameCount() < getTargetFrame())
                    {
                        setMotorDirection(DIRECTION_FORWARD);
                    }
                    else if (getFrameCount() > getTargetFrame())
                    {
                        setMotorDirection(DIRECTION_BACKWARD);
                    } else { // If the frame count is equal to the target frame, stop the motor
                        setStatus(STATUS_IDLE);
                    }
                }
                else
                {
                    // Go to the direction specified by the user
                    setMotorDirection(getMotorDirection());
                    setLed(1);
                }
                // check millis to see if it's time to rise a signal to the step pin (1ms)
                if (millis() - last_step_time >= step_delay)
                {
                    digitalWrite(motor_step_pin, HIGH);
                    delay(1);
                    digitalWrite(motor_step_pin, LOW);
                    last_step_time = millis();
                }
            }
        }
        else
        {
            setStatus(STATUS_IDLE);
        }
        break;
    case STATUS_ERROR:
        break;
    default:
        break;
    }
}