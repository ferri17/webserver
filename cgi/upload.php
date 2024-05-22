<?php
$headers = "";
$body = "";
$upload_dir = $_ENV['upload_store'];

if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_FILES['file'])) {
    $file = $_FILES['file'];
    
    if ($file['error'] === UPLOAD_ERR_OK) {
        $filename = basename($file['name']);
        $target_file = $upload_dir . '/' . $filename;
        
        if (move_uploaded_file($file['tmp_name'], $target_file)) {
            $body = "<h1>Archivo '{$filename}' subido exitosamente</h1>";
            $cookie_value = "success";
        } else {
            $body = "<h1>Error al subir el archivo</h1>";
            $cookie_value = "failure";
        }
    } else {
        $body = "<h1>Error al subir el archivo</h1>";
        $cookie_value = "failure";
    }
} else {
    $body = "<h1>No se ha enviado ningún archivo</h1>";
    $cookie_value = "no_file";
}

$headers = $headers . "HTTP/1.1 200 OK\r\n";
$headers = $headers . "Content-Type:text/html\r\n";
$headers = $headers . "Content-Length:" . strlen($body) . "\r\n";
$headers = $headers . "\r\n";
$headers = $headers . $body;

echo $headers;
?>
