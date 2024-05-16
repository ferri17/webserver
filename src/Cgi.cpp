
#include "Cgi.hpp"


Cgi::Cgi()
{

}

Cgi::~Cgi()
{

}

t_cgi_type Cgi::findExtension(std::string str, std::vector<t_cgi_type> cgi)
{
	std::vector<std::string> vec = split(str, '.');
	t_cgi_type cgi_find;

	if (vec.size() <= 1)
		return (cgi_find);

	std::string extension = vec[vec.size() - 1];
	for (std::vector<t_cgi_type>::iterator it = cgi.begin(); it != cgi.end(); it++)
	{
		if (it->type == '.' + extension)
		{
			cgi_find = *it;
			break;
		}
	}
	return (cgi_find);
}

char **Cgi::generateEnv(std::vector<std::string> cookies)
{
	char **env;
	size_t i = 0;

	if (cookies.size() == 0)
		return (NULL);
	env = new char *[cookies.size() + 1];
	if (!env)
		return (NULL);
	for (; i < cookies.size(); i++)
	{
		trim(cookies[i]);
		env[i] = strdup(cookies[i].c_str());
	}
	env[i] = NULL;
	return (env);
}

int Cgi::generateCgi(std::vector<t_cgi_type> cgi, std::string file, std::string &s, std::vector<std::string> cookies)
{
	int pipefd[2];

    pipe(pipefd);
	t_cgi_type test = findExtension(file, cgi);

	if (test.file.empty())
		return (1);
    pid_t pid = fork();
    if (pid == -1)
        return (1);
	else if (pid == 0)
	{
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        char * const argv[] = {strdup(test.file.c_str()),strdup(file.c_str()), NULL};
		char **env = generateEnv(cookies);
        execve(test.file.c_str(), argv, env);
        exit(1);
	}
	else
	{
		close(pipefd[1]);
        std::stringstream ss;
        char buffer[1024];
        ssize_t bytes_read;
        while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer))) > 0)
            ss.write(buffer, bytes_read);
        close(pipefd[0]);
        int status;
        waitpid(pid, &status, 0);
		if (WEXITSTATUS(status) != 0)
			return (1);
		s = ss.str();
	}
	return (0);
}
