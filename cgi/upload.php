<?php
$headers = "";
$body = "";
$upload_dir = getenv('upload_store');

if (isset($_ENV['file']))
{
    $file = $_ENV['file'];
    
    $filename = "PEPITO.jpeg";
    $target_file = $upload_dir . '/' . $filename;
    
    // Verificar si el directorio de subida es válido
    if (is_dir($upload_dir)) {
        // Asegurarse de que el archivo no existe antes de abrir con 'x'
        if (!file_exists($target_file)) {
            $fop = fopen($target_file, 'x');
            if ($fop) {
                // Leer el contenido temporal del archivo subido
                $temp_file = $file['tmp_name'];
                $file_contents = file_get_contents($temp_file);
                
                // Escribir el contenido en el archivo de destino
                fwrite($fop, $file_contents);
                fclose($fop);
                $body = "El archivo se ha escrito correctamente.";
            } else {
                $body = "<h1>Error al subir el archivo</h1>";
            }
        } else {
            $body = "<h1>El archivo ya existe</h1>";
        }
    } else {
        $body = "<h1>Directorio de subida no válido</h1>";
    }
} else {
    $body = "<h1>No se ha enviado ningún archivo</h1>";
}

// Usar la función header() para enviar cabeceras HTTP
header("HTTP/1.1 200 OK");
header("Content-Type: text/html");
header("Content-Length: " . strlen($body));

echo $body;
?>