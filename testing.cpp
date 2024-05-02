#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int pipefd[2];
    pipe(pipefd);

    pid_t pid = fork();

    if (pid == -1) {
        // Error al hacer fork
        std::cerr << "Error al hacer fork" << std::endl;
        return 1;
    } else if (pid == 0) {
        // Proceso hijo
        close(pipefd[0]); // Cerrar extremo de lectura del pipe en el proceso hijo

        // Redirigir la salida estándar al extremo de escritura del pipe
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        // Ejecutar el comando usando execve
        char* argv[] = {"/bin/bash", "./cgi/test.sh", nullptr};
        execve("/bin/bash", argv, nullptr);
        // Si execve tiene éxito, el código a partir de aquí no se ejecutará
        // En caso de error, imprimir el mensaje de error y salir
        perror("execve");
        return 1;
    } else {
        // Proceso padre
        close(pipefd[1]); // Cerrar extremo de escritura del pipe en el proceso padre

        // Leer la salida del pipe y almacenarla en una stringstream
        std::stringstream ss;
        char buffer[1024];
        ssize_t bytes_read;
        while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
            ss.write(buffer, bytes_read);
        }

        // Esperar a que el proceso hijo termine
        int status;
        waitpid(pid, &status, 0);

        // Imprimir la salida almacenada en la stringstream
        std::cout << "Salida del comando 'ls -l':" << std::endl;
        std::cout << ss.str() << std::endl;

        // Cerrar el extremo de lectura del pipe
        close(pipefd[0]);
    }

    return 0;
}