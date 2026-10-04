bool initializeSenseHat()
{
    char path[32];
    struct fb_fix_screeninfo information;

    // -------------------------
    // Find Sense HAT framebuffer
    // -------------------------
    for (int i = 0; i < FRAMEBUFFER_CYCLE; i++)
    {
        snprintf(path, sizeof(path), "/dev/fb%d", i);

        int fileDescriptor = open(path, O_RDWR);

        if (fileDescriptor < 0)
        {
            continue;
        }

        if (ioctl(fileDescriptor, FBIOGET_FSCREENINFO, &information) == -1)
        {
            close(fileDescriptor);
            continue;
        }

        printf("Found framebuffer %s: %.16s\n",
               path, information.id);

        if (strncmp(information.id,
                    "RPi-Sense FB",
                    sizeof(information.id)) != 0)
        {
            close(fileDescriptor);
            continue;
        }

        // We found the Sense HAT framebuffer
        framebufferFileDescriptor = fileDescriptor;
        framebufferInfo = information;
        framebufferSize = information.smem_len;

        framebuffer = mmap(
            NULL,
            framebufferSize,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            framebufferFileDescriptor,
            0
        );

        if (framebuffer == MAP_FAILED)
        {
            printf("Failed to map framebuffer\n");

            framebuffer = NULL;

            close(framebufferFileDescriptor);
            framebufferFileDescriptor = -1;

            return false;
        }

        printf("Sense HAT framebuffer found at %s\n", path);

        break;
    }

    // Did we actually find a framebuffer?
    if (framebufferFileDescriptor < 0)
    {
        printf("ERROR: could not find RPi-Sense FB\n");
        return false;
    }


    // -------------------------
    // Find Sense HAT joystick
    // -------------------------
    for (int i = 0; i < JOYSTICK_CYCLE; i++)
    {
        snprintf(path, sizeof(path), "/dev/input/event%d", i);

        int fileDescriptor =
            open(path, O_RDONLY | O_NONBLOCK);

        if (fileDescriptor < 0)
        {
            continue;
        }

        char name[256] = {0};

        if (ioctl(fileDescriptor,
                  EVIOCGNAME(sizeof(name)),
                  name) == -1)
        {
            close(fileDescriptor);
            continue;
        }

        printf("Found input device %s: %s\n",
               path, name);

        if (strcmp(name,
                   "Raspberry Pi Sense HAT Joystick") != 0)
        {
            close(fileDescriptor);
            continue;
        }

        // We found the Sense HAT joystick
        joystickFileDescriptor = fileDescriptor;

        printf("Sense HAT joystick found at %s\n", path);

        break;
    }

    // Did we actually find the joystick?
    if (joystickFileDescriptor < 0)
    {
        printf("ERROR: could not find Sense HAT joystick\n");

        // Framebuffer was already initialized,
        // so clean it up before returning failure.
        munmap(framebuffer, framebufferSize);
        framebuffer = NULL;
        framebufferSize = 0;

        close(framebufferFileDescriptor);
        framebufferFileDescriptor = -1;

        return false;
    }

    return true;
}