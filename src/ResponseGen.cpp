
#include "ResponseGen.hpp"

ResponseGen::ResponseGen(Request &req, Server s): _req(req)
{
	_s = s;
	done = 0;
}

ResponseGen::~ResponseGen()
{
	
}

void ResponseGen::genResFile(std::string &fileToOpen, Location loca, std::string nameLoc)
{
	std::vector<std::string> indexs = loca.getIndex();
	std::vector<std::string>::iterator it = indexs.begin();

	for (; it != indexs.end(); it++)
	{
		fileToOpen = *it;
		if (access((*it).c_str(), F_OK | R_OK) == 0)
			break;
	}
	if (it == indexs.end() && loca.getAutoindex() == true)
	{
		std::string dirToOpen;						
		if (loca.getRoot().empty())
			dirToOpen = nameLoc;
		else
			dirToOpen = loca.getRoot();
		if (createDirectory(_res, dirToOpen))
			createResponseError(_res, NOT_FOUND, _s.getErrorPage(), loca.getErrorPage());
		done = 1;
	}
	else if (it == indexs.end())
	{
		if (accpetType(_req) == 0)
			createResponseError(_res, NOT_ACCEPTABLE, _s.getErrorPage(), loca.getErrorPage());
		else
			createResponseError(_res, NOT_FOUND, _s.getErrorPage(), loca.getErrorPage());
		done = 1;
	}
}

void ResponseGen::requestCgi(Location loca, std::string fileToOpen)
{
	Cgi cgi;
	std::string cgiText;
	std::string headerCookie = _req.getHeaderField()["cookie"];
	std::vector<std::string> cookiesEnv = split(headerCookie, ';');
	if (cgi.generateCgi(loca.getCgi(), fileToOpen, cgiText, cookiesEnv))
		createResponseError(_res, INTERNAL_SERVER_ERROR, _s.getErrorPage(), loca.getErrorPage());
	else
		_res.setCgiResponse(cgiText);
	done = 1;
}

void ResponseGen::responsePriority(std::string &fileToOpen, Location loca, std::string nameLoc)
{
	int fileExist = 0;
	Cgi cgi;

	if (fileToOpen.empty())
		genResFile(fileToOpen, loca, nameLoc);
	else
	{
		fileToOpen = loca.getRoot() + "/" + fileToOpen;
		fileExist = 1;
	}
	if (done == 0 && fileExist && !loca.getCgi().empty() && cgi.validExtension(fileToOpen, loca.getCgi()))
	{
		requestCgi(loca, fileToOpen);
	}
	if (done == 0)
		selectTypeOfResponse(_res, _s, loca, _req, fileToOpen);
}

Response ResponseGen::DoResponse()
{
	std::map<std::string, Location> loc = _s.getLocations();
	std::pair<std::string, std::string> dirLocFile = locFind(loc, _req.getRequestTarget());
	std::string nameLoc = dirLocFile.first;
	std::string fileToOpen = dirLocFile.second;
	if (nameLoc.empty())
		createResponseError(_res, NOT_FOUND, _s.getErrorPage());
	else
	{
		Location loca = loc.find(nameLoc)->second;

		if (loca.checkMethod(_req.getMethod()) == 1)
			createResponseError(_res, METHOD_NOT_ALLOWED, _s.getErrorPage(), loca.getErrorPage());
		if (!loca.getReturnPag().empty())
		{
			_res.setStatusLine((statusLine){"HTTP/1.1", FOUND, ERROR_MESSAGE(FOUND)});
			_res.addHeaderField(std::pair<std::string, std::string>(LOCATION, loca.getReturnPag()));
		}
		else if (_req.getMethod() == "POST")
		{
			requestCgi(loca, fileToOpen);
		}
		else if (_req.getMethod() == "DELETE")
		{
			
		}
		else
		{
			responsePriority(fileToOpen, loca, fileToOpen);
		}
	}
	return (_res);
}

int ResponseGen::createResponseImage( std::string fileToOpen, Response &res)
{
	std::ifstream fileicon(fileToOpen, std::ios::binary);

	if (!fileicon.is_open())
		return (1);

	std::string html;
	char c;

	while (fileicon.get(c))
		html += c;
	fileicon.close();
	res.addHeaderField(std::pair<std::string, std::string>(CONTENT_TYPE, "image/x-icon"));
	res.addHeaderField(std::pair<std::string, std::string>(CONTENT_LENGTH, toString(html.size())));
	res.setBody(html);
	return (0);
}

int ResponseGen::createDirectory(Response &res, std::string dir)
{
    DIR *opened = opendir(dir.c_str());
	struct dirent *entry;
	std::string body;

	if (!opened)
		return (1);

	entry = readdir(opened);

	body = "<h1>Directory: " + dir + "</h1>";
	while(entry)
	{
		body += "<li>";
		body += entry->d_name;
		body += "</li>\n";
		entry = readdir(opened);
	}
	closedir(opened);
	res.addHeaderField(std::pair<std::string, std::string>(CONTENT_TYPE, "text/html"));
	res.addHeaderField(std::pair<std::string, std::string>(CONTENT_LENGTH, toString(body.size())));
	res.setBody(body);
	return(0);
}

int ResponseGen::createResponseHtml( std::string fileToOpen, Response &res)
{
	std::ifstream file(fileToOpen);

	if (!file.is_open())
		return (1);
	std::string html;
	std::cout << "====================" << fileToOpen << "====================" << std::endl;

	getline(file, html, '\0');
	file.close();
	res.addHeaderField(std::pair<std::string, std::string>(CONTENT_TYPE, "text/html"));
	res.addHeaderField(std::pair<std::string, std::string>(CONTENT_LENGTH, toString(html.size())));
	res.setBody(html);
	return (0);
}

void ResponseGen::createResponseError( Response &res, int codeError, std::map<int, std::string> errorPageServ)
{
	res.setStatusLine((statusLine){"HTTP/1.1", codeError, ERROR_MESSAGE(codeError)});
	if (!errorPageServ.empty() && errorPageServ.find(codeError) != errorPageServ.end())
	{
		if (createResponseHtml(errorPageServ[codeError], res) == 0)
			return ;
	}
	std::string body;

	body += "<h1 style=\"text-align: center;\">" + toString(codeError) + " " + ERROR_MESSAGE(codeError) + "</h1>";
	res.addHeaderField(std::pair<std::string, std::string>(CONTENT_TYPE, "text/html"));
	res.addHeaderField(std::pair<std::string, std::string>(CONTENT_LENGTH, toString(body.size())));
	res.setBody(body);
	return ;
}

void ResponseGen::createResponseError( Response &res, int codeError, std::map<int, std::string> errorPageServ, std::map<int, std::string> errorPageLoc)
{
	res.setStatusLine((statusLine){"HTTP/1.1", codeError, ERROR_MESSAGE(codeError)});
	if (!errorPageLoc.empty() && errorPageLoc.find(codeError) != errorPageLoc.end())
	{
		if (createResponseHtml(errorPageLoc[codeError], res) == 0)
			return ;
	}
	else if (!errorPageServ.empty() && errorPageServ.find(codeError) != errorPageServ.end())
	{
		if (createResponseHtml(errorPageServ[codeError], res) == 0)
			return ;
	}
	std::string body;

	body += "<h1 style=\"text-align: center;\">" + toString(codeError) + " " + ERROR_MESSAGE(codeError) + "</h1>";
	res.addHeaderField(std::pair<std::string, std::string>(CONTENT_TYPE, "text/html"));
	res.addHeaderField(std::pair<std::string, std::string>(CONTENT_LENGTH, toString(body.size())));
	res.setBody(body);
	return ;
}

int ResponseGen::accpetType(Request req)
{
	std::map<std::string, std::string> headers = req.getHeaderField();

	std::string acceptTypes = headers["accept"];

	if (acceptTypes.find("text/html") != acceptTypes.npos)
		return (1);
	else if (acceptTypes.find("image/") != acceptTypes.npos)
		return (2);
	else if (acceptTypes.find("*/*") != acceptTypes.npos)
		return (1);
	return (0);
}

int ResponseGen::comparePratial(std::string src, std::string find)
{
	int i = 0;

	for (; src[i] && find[i]; i++)
	{
		if (src[i] != find[i])
			return (i);
	}
	return (i);
}

std::string ResponseGen::partialFind(std::map<std::string, Location> loc, std::string reqTarget)
{
	std::map<std::string, Location>::iterator itLoc = loc.begin();

	for (; itLoc != loc.end(); itLoc++)
	{
		int i = comparePratial(itLoc->first, reqTarget);
		if (itLoc->first[i] == '/')
			return (itLoc->first);
	}
	return ("");
}

std::string ResponseGen::absolutFind(std::map<std::string, Location> loc, std::string reqTarget)
{
	std::map<std::string, Location>::iterator itLoc = loc.find(reqTarget);
	if (itLoc == loc.end())
		return ("");
	return (itLoc->first);
}

void ResponseGen::selectTypeOfResponse(Response &res, Server s, Location loca, Request req, std::string fileToOpen)
{
	int type = accpetType(req);
	std::cout << "_________________" << fileToOpen << "_________________"<< std::endl;
	if (type == 1)
	{
		if (createResponseHtml(fileToOpen, res))
			createResponseError(res, NOT_FOUND, s.getErrorPage(), loca.getErrorPage());
	}
	else if (type == 2)
	{
		if(createResponseImage(fileToOpen, res))
				createResponseError(res, NOT_FOUND, s.getErrorPage(), loca.getErrorPage());
	}
	else
		createResponseError(res, NOT_ACCEPTABLE, s.getErrorPage(), loca.getErrorPage());
}

std::pair<std::string, std::string> ResponseGen::locFind(std::map<std::string, Location> loc, std::string reqTarget)
{
	if (reqTarget[reqTarget.size() - 1] == '/' && reqTarget.size() != 1)
		reqTarget.erase(reqTarget.size() - 1);
	std::string	test = absolutFind(loc, reqTarget);
	if (test.empty())
		test = partialFind(loc, reqTarget);
	if (test.empty())
	{
		std::vector<std::string> splited = split(reqTarget, '/');
		if (splited.size() == 0)
			return(std::pair<std::string, std::string>(test, ""));
		std::string newTarget = reqTarget;
		int i = newTarget.size();

		newTarget.erase(i - splited[splited.size() - 1].size() - 1, i);
		if (newTarget.size() == 0)
		{
			newTarget = reqTarget;
			newTarget.erase(i - splited[splited.size() - 1].size(), i);
		}
		test = absolutFind(loc, newTarget);
		if (test.empty())
			return (std::pair<std::string, std::string>(test, ""));
		return(std::pair<std::string, std::string>(test, splited[splited.size() - 1]));
	}
	return (std::pair<std::string, std::string>(test, ""));
}
