<?php
$headers;
$body;
$cookie_name = "user";
$cookie_value = "John Doe";

if(!$_ENV[$cookie_name]) {
  $body = "Cookie named '" . $cookie_name . "' is not set!";
} else {
  $body = $body . "Cookie '" . $cookie_name . "' is set!<br>";
  $body = $body . "Value is: " . $_ENV[$cookie_name];
}

$headers = $headers . "HTTP/1.1 200 OK\r\n";
$headers = $headers . "Content-Type:text/html\r\n";
$headers = $headers . "Content-Length:" . strlen($body) . "\r\n";
$headers = $headers . "Set-Cookie:" . $cookie_name . "=" . $cookie_value . "\r\n";
$headers = $headers . "\r\n";
$headers = $headers . $body;

echo $headers;
?>
