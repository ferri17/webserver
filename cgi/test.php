<?php
$headers;
$body;

$body = "<h1>HI IS PHP AWESOME</h1>";

$headers = $headers . "HTTP/1.1 200 OK\r\n";
$headers = $headers . "Content-Type:text/html\r\n";
$headers = $headers . "Content-Length:" . strlen($body) . "\r\n";
$headers = $headers . "\r\n";
$headers = $headers . $body;

echo $headers;
?>
