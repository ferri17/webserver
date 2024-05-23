
#pragma once

#include <iostream>
#include <vector>
#include "Response.hpp"
#include <dirent.h>
#include "Headers.hpp"
#include "Request.hpp"
#include "Cgi.hpp"

class ResponseGen
{
	private:
		Request &_req;
		Response _res;
		Server _s;
		int done;
		void genResFile(std::string &fileToOpen, Location loca, std::string nameLoc);
		void deleteMethod(Location loca, std::string fileToOpen);
		void responseHtmlOkey();
		void requestCgi(Location loca, std::string fileToOpen);
		void responsePriority(std::string &fileToOpen, Location loca, std::string nameLoc);
		int createResponseImage( std::string fileToOpen, Response &res);
		int createDirectory(Response &res, std::string dir);
		int createResponseHtml( std::string fileToOpen, Response &res);
		void createResponseError( Response &res, int codeError, std::map<int, std::string> errorPageServ);
		void createResponseError( Response &res, int codeError, std::map<int, std::string> errorPageServ, std::map<int, std::string> errorPageLoc);
		int accpetType(Request req);
		int comparePratial(std::string src, std::string find);
		std::string partialFind(std::map<std::string, Location> loc, std::string reqTarget);
		std::string absolutFind(std::map<std::string, Location> loc, std::string reqTarget);
		void selectTypeOfResponse(Response &res, Server s, Location loca, Request req, std::string fileToOpen);
		std::pair<std::string, std::string> locFind(std::map<std::string, Location> loc, std::string reqTarget);
	public:
		ResponseGen(Request &req, Server s);
		Response DoResponse();
		~ResponseGen();
};
