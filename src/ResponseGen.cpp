
#include "ResponseGen.hpp"
#include <cstdio>

ResponseGen::ResponseGen(Request &req, Server s, bool & closeOnEnd): _req(req), _closeOnEnd(closeOnEnd)
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
	std::vector<std::string> cookiesEnv;
	std::string headerCookie = _req.getHeaderField()["cookie"];
	if (!headerCookie.empty())
		cookiesEnv = split(headerCookie, ';');
	if (cgi.generateCgi(loca.getCgi(), fileToOpen, cgiText, cookiesEnv))
		createResponseError(_res, INTERNAL_SERVER_ERROR, _s.getErrorPage(), loca.getErrorPage());
	else
	{
		_res.setCgiResponse(cgiText);
	}
	done = 1;
}

void ResponseGen::requestCgiPost(Location loca, std::string fileToOpen)
{
	Cgi cgi;
	std::string cgiText;

	fileToOpen = loca.getRoot() + "/" + fileToOpen;

	std::vector<std::string> cookiesEnv;
	std::string body = _req.getBodyMssg();
	std::string file = "body=" + body;
	std::string upload = "upload_store=" + loca.getUploadStore();
	std::cout << upload << std::endl;
	std::string headerCookie = _req.getHeaderField()["cookie"];
	if (!headerCookie.empty())
		cookiesEnv = split(headerCookie, ';');
	cookiesEnv.push_back(file);
	cookiesEnv.push_back(upload);
	int status = cgi.generateCgi(loca.getCgi(), fileToOpen, cgiText, cookiesEnv);
	if (status == 1)
		createResponseError(_res, INTERNAL_SERVER_ERROR, _s.getErrorPage(), loca.getErrorPage());
	else if (status == 2)
		createResponseError(_res, REQUEST_TIMEOUT, _s.getErrorPage(), loca.getErrorPage());
	else
		_res.setCgiResponse(cgiText);
}

void ResponseGen::responseHtmlOkey()
{
	std::string body;

	_res.setStatusLine((statusLine){"HTTP/1.1", 200, "OK"});
	body = "<h1>File Deleted Successfully</h1>";
	_res.addHeaderField(std::pair<std::string, std::string>(CONTENT_LENGTH, toString(body.size())));
	_res.setBody(body);
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

void ResponseGen::deleteMethod(Location loca, std::string fileToOpen)
{
	std::cout << loca.getRoot() << "/" << fileToOpen << std::endl;
	if (std::remove((loca.getRoot() + "/" + fileToOpen).c_str()) != 0)
		createResponseError(_res, NO_CONTENT, _s.getErrorPage(), loca.getErrorPage());
	else
		responseHtmlOkey();
}

void	ResponseGen::manageConnectionState(void)
{
	std::map<std::string, std::string>				headers = _req.getHeaderField();
	std::map<std::string, std::string>::iterator	it = headers.find("connection");

	this->_closeOnEnd = true;
	if (it != headers.end())
	{
		if (stringToLower((*it).second) == "keep-alive")
			this->_closeOnEnd = false;
	}
}

int		ResponseGen::checkUpload()
{
	std::map<std::string, std::string>	heders = _req.getHeaderField();
	std::string content = heders.at("content-type");
	if(!content.empty())
	{
		std::vector<std::string> splited = split(content, ';');
		if (splited.size() == 2)
		{
			trim(splited[0]);
			if (splited[0] == "multipart/form-data")
			{
				trim(splited[1]);
				std::vector<std::string> boundary = split(splited[1], '=');
				if (boundary.size() == 2)
				{
					_boundary = boundary[1];
					return (1);
				}
			}
		}
	}
	return (0);
}

std::vector<std::string> ResponseGen::parse_multipart_form_data(const std::string& body)
{
	std::vector<std::string> parts;
	std::string delimiter = "--" + _boundary;
	std::string end_boundary = delimiter + "--";

	size_t pos = 0;
	size_t end_pos = 0;

	while ((pos = body.find(delimiter, end_pos)) != std::string::npos)
	{
		pos += delimiter.length();
		end_pos = body.find(delimiter, pos);
		if (end_pos == std::string::npos)
			end_pos = body.find(end_boundary, pos);
		if (end_pos != std::string::npos)
		{
			std::string part = body.substr(pos, end_pos - pos);
			if (!part.empty())
				parts.push_back(part);
		}
	}
	return (parts);
}

std::string extract_filename(const std::string& line)
{
    size_t name_pos = line.find("filename=\"");

    if (name_pos != std::string::npos)
	{
		size_t start_pos = name_pos + 10;
		size_t end_pos = line.find("\"", start_pos);
	
		if (end_pos != std::string::npos)
			return (line.substr(start_pos, end_pos - start_pos));
    }
    return ("");
}

int		ResponseGen::doUpload(Location loca)
{
	std::cout << _req << std::endl;
	std::string body = _req.getBodyMssg();
	std::istringstream iss(body);
    std::string line;
	std::string filename;

    while (std::getline(iss, line))
	{
        if (line.find(_boundary) != std::string::npos)
            break;
    }

	while (std::getline(iss, line))
	{
        if (line.find("Content-Disposition: form-data;") != std::string::npos)
		{
            filename = extract_filename(line);
            if (!filename.empty())
                break;
        }
    }

    while (std::getline(iss, line) && line != "\r") {}

    std::ostringstream file_content;
    while (std::getline(iss, line) && line != _boundary)
        file_content << line << "\n";
	std::cout << loca.getUploadStore() + filename;
    std::ofstream output_file((loca.getUploadStore() + filename).c_str(), std::ios::binary);
    if (!output_file)
        return (0);
    output_file << file_content.str();
    output_file.close();
	return (1);
}

Response ResponseGen::DoResponse()
{
	if (_req.getErrorCode() != 0)
	{
		createResponseError(_res, _req.getErrorCode(), _s.getErrorPage());
		return (_res);
	}
	manageConnectionState();
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
		else if (!loca.getReturnPag().empty())
		{
			_res.setStatusLine((statusLine){"HTTP/1.1", FOUND, ERROR_MESSAGE(FOUND)});
			_res.addHeaderField(std::pair<std::string, std::string>(LOCATION, loca.getReturnPag()));
		}
		else if (_req.getMethod() == "POST" && !fileToOpen.empty())
		{
			if (checkUpload())
			{
				if (doUpload(loca))
					responsePriority(fileToOpen, loca, fileToOpen);
				else
					createResponseError(_res, INTERNAL_SERVER_ERROR, _s.getErrorPage(), loca.getErrorPage());
			}
			else
				requestCgiPost(loca, fileToOpen);
		}
		else if (_req.getMethod() == "DELETE")
			deleteMethod(loca, fileToOpen);
		else
			responsePriority(fileToOpen, loca, fileToOpen);
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

	getline(file, html, '\0');
	file.close();
	res.addHeaderField(std::pair<std::string, std::string>(CONTENT_TYPE, "text/html"));
	res.addHeaderField(std::pair<std::string, std::string>(CONTENT_LENGTH, toString(html.size())));
	res.setBody(html);
	return (0);
}

void ResponseGen::createResponseError( Response &res, int codeError, std::map<int, std::string> errorPageServ)
{
	if (codeError >= HTTP_ERROR_START)
	{
		this->_closeOnEnd = true;
		res.addHeaderField(std::pair<std::string, std::string>(CONNECTION, "close"));
	}
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
	if (codeError >= HTTP_ERROR_START)
	{
		this->_closeOnEnd = true;
		res.addHeaderField(std::pair<std::string, std::string>(CONNECTION, "close"));
	}
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

void ResponseGen::selectTypeOfResponse(Response &res, Server s, Location loca, Request req, std::string fileToOpen)
{
	int type = accpetType(req);
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

std::string ResponseGen::partialFind(std::map<std::string, Location> loc, std::string reqTarget)
{
	std::map<std::string, Location>::iterator itLoc = loc.begin();

	for (; itLoc != loc.end(); itLoc++)
	{
		int i = comparePratial(itLoc->first, reqTarget);
		if (itLoc->first[i] == '/' && reqTarget[i] == '\0')
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
