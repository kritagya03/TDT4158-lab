int readSenseHatJoystick()
{
    struct pollfd pollJoystick = {
        .fd = joystickFileDescriptor,
        .events = POLLIN
    };

    int key = 0;

    while (poll(&pollJoystick, 1, 0) > 0)
    {
        struct input_event event;

        ssize_t bytesRead = read(
            joystickFileDescriptor,
            &event,
            sizeof(event)
        );

        if (bytesRead != sizeof(event))
        {
            break;
        }

        if (event.type != EV_KEY)
        {
            continue;
        }

        // Ignore key-release events
        if (event.value == 0)
        {
            continue;
        }

        switch (event.code)
        {
            case KEY_LEFT:
            case KEY_RIGHT:
            case KEY_DOWN:
            case KEY_UP:
            case KEY_ENTER:
                key = event.code;
                break;
        }
    }

    return key;
}