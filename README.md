<p align="center">
	<img alt="GitHub Repo stars" src="https://img.shields.io/github/stars/ferri17/webserver?color=yellow" />
	<img alt="GitHub top language" src="https://img.shields.io/github/languages/top/ferri17/webserver" />
	<img alt="GitHub code size in bytes" src="https://img.shields.io/github/languages/code-size/ferri17/webserver?color=red" />
	<img alt="GitHub last commit (by committer)" src="https://img.shields.io/github/last-commit/ferri17/webserver" />
	<img alt="MacOS compatibility" src="https://img.shields.io/badge/macOS-compatible-brightgreen.svg" />
</p>

<h3 align="center">HTTP 1.1 non-blocking webserver</h3>

  <p align="center">
    The goal of this project is to build a HTTP 1.1 complaint webserver with non-blocking I/O operations.
    <br />
  </p>
</div>

<!-- ABOUT THE PROJECT -->
## About The Project


![Webserver demo intro](https://github.com/ferri17/webserver/assets/19575860/4e6b753f-4980-46b7-97a5-9a2f971d502e)

This repository contains a **non-blocking HTTP/1.1 server implemented in C++98.** The server is designed to handle multiple connections simultaneously without blocking on I/O operations. Non-blocking I/O is a critical feature in modern network servers, enabling the server to handle many connections simultaneously without waiting for I/O operations to complete. The server supports basic HTTP/1.1 functionality, including handling **GET, POST and DELETE requests**. It also includes a simple configuration system that allows users to set up the server parameters, such as listening ports, document roots, and more, through a **configuration file very similar to nginx.**

<!-- GETTING STARTED -->
## Getting Started
In order to run the program first clone the repository:
```bash
git clone git@github.com:ferri17/webserver.git
```
Open the folder:
```bash
cd webserver/
```
Compile the program:
```bash
make
```
Run the program:
```bash
./webserv [path to configuration file]
```
> [!NOTE]
> If a path to a configuration file is not specified, the server will use a default configuration.
<!-- Authors -->

## Configuration file
The configuration file syntax is very similar to the one used by nginx in it's http server directives. Multiple server directives can be added, multiple listen directives in each server, server names, root locations, autoindexing and more.
```nginx
server
{
	client_max_body_size	500;
	listen	localhost:8010;
	server_name	 adria;
	root ./html;
	location / {
		autoindex		on;
		error_page 404 		/index.html;
		allow_methods		GET POST;
		upload_store		./files_upload/;
		index			empty.html;
		cgi			.sh /bin/bash;
	}
	location /assets/ {
		root		./cgi;
		autoindex	on;
	}
	location /test {
		autoindex	on;
		root		./html/error_pages;
	}

}

server
{
	listen		127.0.0.1:8050;
	server_name	ferran;
	root		./html;
	location / {
		autoindex		on;
		error_page 404 		./a/../error_pages/404.html;
		allow_methods		GET POST;
		upload_store		./files_upload/;
		index			index.html;
		cgi			.sh /bin/bash;
	}
}
``` 

## Authors
This project is written by @AdriaPriego and me as part of 42 School core curriculum.
