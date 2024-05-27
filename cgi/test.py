body = "Hello, Python Cgi works!!!"

print("HTTP/1.1 200 OK\r")
print("Content-Type:text/html\r")
print("Content-Length:" + repr(len(body)) + "\r")
print("\r")
print(body)
