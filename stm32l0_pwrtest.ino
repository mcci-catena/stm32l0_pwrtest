/*

Name:	stm32l0_pwrtest.ino

Function:
        Test bench for power testing.

Copyright Notice:
        See accompanying LICENSE file.

Author:
        Terry Moore, MCCI Corporation	April 2019

*/

#include <Catena.h>

#include <Catena_Led.h>
#include <Catena_CommandStream.h>
#include <mcciadk_baselib.h>

// load the fixup.
#include <Catena_CommandStream_vmicro_fixup.h>

/****************************************************************************\
|
|       Manifest constants & typedefs.
|
\****************************************************************************/

using namespace McciCatena;

constexpr uint8_t kFramPowerOn = D10;

/****************************************************************************\
|
|       Read-only data.
|
\****************************************************************************/

static const char sVersion[] = "0.1.0";

/****************************************************************************\
|
|       Variables.
|
\****************************************************************************/

// the Catena instance
Catena gCatena;

// the LED instance object
StatusLed gLed (Catena::PIN_STATUS_LED);

/****************************************************************************\
|
|	The command table
|
\****************************************************************************/

cCommandStream::CommandFn cmdReg, cmdSleep, cmdStandby, cmdStop;

static const cCommandStream::cEntry sApplicationCommmands[] =
        {
        { "r", cmdReg },
        { "sleep", cmdSleep },
        { "standby", cmdSleep },
        { "stop", cmdStop },
        // other commands go here....
        };

/* create the top level structure for the command dispatch table */
static cCommandStream::cDispatch
sApplicationCommandDispatch(
        sApplicationCommmands,          /* this is the pointer to the table */
        sizeof(sApplicationCommmands),  /* this is the size of the table */
        nullptr                         /* this is the "first word" for all the commands in this table*/
        );

/*

Name:	setup()

Function:
        Arduino setup function.

Definition:
        void setup(
            void
            );

Description:
        This function is called by the Arduino framework after
        basic framework has been initialized. We initialize the sensors
        that are present on the platform, set up the LoRaWAN connection,
        and (ultimately) return to the framework, which then calls loop()
        forever.

Returns:
        No explicit result.

*/

void setup(void)
        {
        setup_platform();

        /* add our application-specific commands */
        gCatena.addCommands(
                /* name of app dispatch table, passed by reference */
                sApplicationCommandDispatch,
                /*
                || optionally a context pointer using static_cast<void *>().
                || normally only libraries (needing to be reentrant) need
                || to use the context pointer.
                */
                nullptr
                );
        }

// set up the platform, print hello, etc.
void setup_platform()
        {
        pinMode(kFramPowerOn, OUTPUT);
        digitalWrite(kFramPowerOn, 1);

        gCatena.begin();

        delay(5000);
        // if running unattended, don't wait for USB connect.
        if (!(gCatena.GetOperatingFlags() &
                static_cast<uint32_t>(gCatena.OPERATING_FLAGS::fUnattended)))
                {
                while (!Serial)
                        /* wait for USB attach */
                        yield();
                }

        gCatena.SafePrintf("\n");
        gCatena.SafePrintf("-------------------------------------------------------------------------------\n");
        gCatena.SafePrintf("This is the stm32l0_pwrtest program V%s.\n", sVersion);
        gCatena.SafePrintf("The program will now idle waiting for you to enter commands\n");
        gCatena.SafePrintf("Enter 'help' for a list of commands.\n");
        gCatena.SafePrintf("(remember to select 'Line Ending: Newline' at the bottom of the monitor window.)\n");
        gCatena.SafePrintf("--------------------------------------------------------------------------------\n");
        gCatena.SafePrintf("\n");

        // set up the LED
        gLed.begin();
        gCatena.registerObject(&gLed);
        gLed.Set(LedPattern::OneThirtySecond);
        }

/*

Name:	loop()

Function:
        Arduino polling function.

Definition:
        void loop(
            void
            );

Description:
        This function is called by the Arduino framework after
        initialization and setup() are complete. When it returns,
        the framework simply calls it again.

Returns:
        No explicit result.

*/

void loop()
        {
        // this drives the command processor, which in turn calls the command completion routines.
        gCatena.poll();
        }

/****************************************************************************\
|
|	The command functions
|
\****************************************************************************/

// argv[0] is "r"
// argv[1] is address to read
// argv[2] if present is the number of words

/* process "r base [len]" -- read and display len words of memory starting at base */
cCommandStream::CommandStatus cmdReg(
        cCommandStream *pThis,
        void *pContext,
        int argc,
        char **argv
        )
        {
        uint32_t uLength;
        uint32_t uBase;
        cCommandStream::CommandStatus status;

        if (argc < 2)
                return cCommandStream::CommandStatus::kInvalidParameter;

        // get arg 2 as length; default is 32 bytes
        status = cCommandStream::getuint32(argc, argv, 2, 16, uLength, 1);

        if (status != cCommandStream::CommandStatus::kSuccess)
                return status;

        // get arg 1 as base; default is irrelevant
        status = cCommandStream::getuint32(argc, argv, 1, 16, uBase, 0);

        if (status != cCommandStream::CommandStatus::kSuccess)
                return status;

        // dump the registers
        uint32_t buffer[8];
        for (uint32_t here = 0; here < uLength; here += sizeof(buffer))
                {
                char line[80];
                size_t n;

                n = uLength - here;
                if (n > sizeof(buffer)/sizeof(buffer[0]))
                        n = sizeof(buffer)/sizeof(buffer[0]);

                std::memset(buffer, 0, n * sizeof(buffer[0]));

                /* once: */
                        {
                        auto p = reinterpret_cast<const volatile uint32_t *>(uBase + here);
                        for (auto i = 0u; i < n; ++p, ++i)
                                buffer[i] = *p;
                        }

                unsigned iLine;

                iLine = McciAdkLib_Snprintf(line, sizeof(line), 0, "%08x:", uBase);
                for (auto i = 0u; i < n; ++i)
                        {
                        iLine += McciAdkLib_Snprintf(line, sizeof(line), iLine, " %08x", buffer[i]);
                        }
                pThis->printf("%s\n", line);
                }

        return status;
        }

// argv[0] is "sleep"
/* process "sleep"  */
cCommandStream::CommandStatus cmdSleep(
        cCommandStream *pThis,
        void *pContext,
        int argc,
        char **argv
        )
        {
        if (argc > 1)
                return cCommandStream::CommandStatus::kInvalidParameter;

        pThis->printf("%s not implemented yet\n", argv[0]);
        }

// argv[0] is "standby"
/* process "standby"  */
cCommandStream::CommandStatus cmdStandby(
        cCommandStream *pThis,
        void *pContext,
        int argc,
        char **argv
        )
        {
        if (argc > 1)
                return cCommandStream::CommandStatus::kInvalidParameter;

        pThis->printf("%s not implemented yet\n", argv[0]);
        }

// argv[0] is "stop"
/* process "stop"  */
cCommandStream::CommandStatus cmdStop(
        cCommandStream *pThis,
        void *pContext,
        int argc,
        char **argv
        )
        {
        if (argc > 1)
                return cCommandStream::CommandStatus::kInvalidParameter;

        pThis->printf("%s not implemented yet\n", argv[0]);
        }
