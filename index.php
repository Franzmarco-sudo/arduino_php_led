2<?php
require_once __DIR__ . '/config.php';

$stateFile = __DIR__ . '/led_state.txt';
$state = (is_file($stateFile) && trim(file_get_contents($stateFile)) === 'ON') ? 'ON' : 'OFF';

$message = '';
$type = '';

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // Toggle: send the opposite of the current state
    $newState = ($state === 'ON') ? 'OFF' : 'ON';
    $result = sendArduinoCommand($newState === 'ON' ? '1' : '0');

    if ($result['success']) {
        $state = $newState;
        file_put_contents($stateFile, $state);
    }

    $message = $result['message'];
    $type = $result['success'] ? 'success' : 'error';
}

$isOn = ($state === 'ON');
?>
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Arduino LED Controller</title>
<style>
body{margin:0;min-height:100vh;display:flex;align-items:center;justify-content:center;background:#f1f5f9;font-family:Arial,sans-serif}
.card{width:420px;max-width:92%;background:#fff;padding:35px;border-radius:18px;box-shadow:0 10px 30px rgba(0,0,0,.12);text-align:center}
h1{color:#1f2937;margin-bottom:8px}.subtitle{color:#64748b}
.port{display:inline-block;padding:8px 14px;border-radius:20px;background:#eef2ff;color:#3730a3;margin:20px}
.status{font-size:15px;color:#64748b;margin-bottom:18px}
.status strong{color:#1f2937}
button{border:0;border-radius:10px;padding:18px 40px;color:white;font-size:18px;font-weight:bold;cursor:pointer;min-width:220px}
button.is-on{background:#16a34a}
button.is-off{background:#6b7280}
.message{margin-top:22px;padding:13px;border-radius:9px;font-weight:bold}
.success{background:#dcfce7;color:#166534}.error{background:#fee2e2;color:#991b1b}
.note{font-size:12px;color:#64748b;margin-top:22px}
</style>
</head>
<body>
<div class="card">
<h1>Arduino LED Controller</h1>
<div class="subtitle">Local PC → USB Serial → Arduino</div>
<div class="port">Arduino Port: <?php echo htmlspecialchars(ARDUINO_PORT); ?></div>
<div class="status">LED is currently <strong><?php echo $state; ?></strong></div>
<form method="post">
<button type="submit" class="<?php echo $isOn ? 'is-on' : 'is-off'; ?>">
<?php echo $isOn ? 'LED ON (click to turn OFF)' : 'LED OFF (click to turn ON)'; ?>
</button>
</form>
<?php if ($message !== ''): ?>
<div class="message <?php echo htmlspecialchars($type); ?>">
<?php echo htmlspecialchars($message); ?>
</div>
<?php endif; ?>
<div class="note">Close Arduino Serial Monitor before using this page.</div>
</div>
</body>
</html>
