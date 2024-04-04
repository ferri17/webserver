
#include "Location.hpp"


Location::Location( void )
{
}

void Location::setAutoindex(bool autoindex) { _autoindex = autoindex; }
void Location::setErrorPage(const std::map<int, std::string> &error_page) { _error_page = error_page; }
void Location::setCgi(const t_cgi_type cgi) { _cgi.push_back(cgi); }
void Location::setUploadStore(const std::string &upload_store) { _upload_store = upload_store; }
void Location::setReturnPag(const std::string &returnPag) { _returnPag = returnPag; }
void Location::setIndex(const std::vector<std::string> &index) { _index = index; }
bool Location::getAutoindex() const { return _autoindex; }
std::map<int, std::string> Location::getErrorPage() const { return _error_page; }
std::vector<std::string > Location::getAllowMethods() const { return _allow_methods; }
std::string Location::getUploadStore() const { return _upload_store; }
std::vector<t_cgi_type> Location::getCgi() const { return _cgi; }
std::string Location::getReturnPag() const { return _returnPag; }
std::vector<std::string> Location::getIndex() const { return _index; }
void Location::pushAllowMethods(std::string methods) { _allow_methods.push_back(methods); }
void Location::pushErrorPage(std::pair<int, std::string> node) { _error_page.insert(node); }
void Location::pushIndex(std::string index) { _index.push_back(index); };

Location::~Location( void )
{
}

std::ostream	&operator<<(std::ostream &out, const Location &nb)
{
	out << "AutoIndex: " << nb.getAutoindex() << std::endl;
	out << "Upload_store: " << nb.getUploadStore() << std::endl;
	out << "Return: " << nb.getReturnPag() << std::endl;
	out << "Error Page: " << std::endl;
	std::map<int, std::string> error = nb.getErrorPage();
	std::map<int, std::string>::iterator itE = error.begin();
	for (; itE != error.end(); itE++)
		out << "\t- " << itE->second <<std::endl;
	out << "Methods: " << std::endl;
	std::vector<std::string> methods = nb.getAllowMethods();
	for (size_t i = 0; i < methods.size(); i++)
		out << "\t- " << methods[i] <<std::endl;
	out << "CGI: " << std::endl;
	std::vector<t_cgi_type> cgi = nb.getCgi();
	for (size_t i = 0; i < cgi.size(); i++)
		out << "\t- " << cgi[i].type << "---" << cgi[i].file <<std::endl;
	return (out);
}
