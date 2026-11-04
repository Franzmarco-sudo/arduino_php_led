<?php
define('ARDUINO_PORT', 'COM5');
define('ARDUINO_BAUD', 9600);

function configureArduinoSerial(): array
{
    $port = strtoupper(trim(ARDUINO_PORT));

    if (!preg_match('/^COM([1-9][0-9]*)$/', $port)) {
        return [
            'success' => false,
            'message' => "Invalid COM port: $port"
        ];
    }

    // Windows MODE command. Do not use escapeshellarg() here.
    $command = 'mode ' . $port .
               ' BAUD=' . (int) ARDUINO_BAUD .
               ' PARITY=N DATA=8 STOP=1 XON=OFF';

    $output = [];
    $exitCode = 0;

    exec($command . ' 2>&1', $output, $exitCode);

    if ($exitCode !== 0) {
        $details = trim(implode(' ', $output));

        return [
            'success' => false,
            'message' => "Windows could not configure $port." .
                         ($details !== '' ? " Windows says: $details" : '')
        ];
    }

    return [
        'success' => true,
        'message' => 'Serial port configured.'
    ];
}

function sendArduinoCommand(string $command): array
{
    if (!in_array($command, ['0', '1'], true)) {
        return [
            'success' => false,
            'message' => 'Invalid Arduino command.'
        ];
    }

    $configured = configureArduinoSerial();

    if (!$configured['success']) {
        return $configured;
    }

    $port = strtoupper(trim(ARDUINO_PORT));

    /*
     * IMPORTANT:
     * In a PHP single-quoted string, \\ represents one backslash.
     * This creates the Windows device path:
     *
     *     \\.\COM6
     */
    $device = '\\\\.\\' . $port;

    $handle = @fopen($device, 'w+b');

    if ($handle === false) {
        return [
            'success' => false,
            'message' => "PHP could not open $port. Close Arduino Serial Monitor and other serial programs."
        ];
    }

    /*
     * Opening the serial port can reset an Arduino Uno.
     * Wait for the sketch to start before sending the command.
     */
    usleep(2500000);

    $written = @fwrite($handle, $command);
    fflush($handle);
    usleep(100000);
    fclose($handle);

    if ($written === false || $written < 1) {
        return [
            'success' => false,
            'message' => "PHP opened $port but could not write to it."
        ];
    }

    return [
        'success' => true,
        'message' => $command === '1'
            ? 'LED ON command sent successfully.'
            : 'LED OFF command sent successfully.'
    ];
}
?>
