HTTP client

=== Description ===

Program files:
client.c - file that implement a http client program.

the user will enter a command using argv.

Command line usage: client [–p <text>] [–r n <pr1=value1 pr2=value2 …>] <URL>. The flags and the url
can come at any order, the only limitation is that the parameters should come right after the flag –r and
the text should come right after the flag –p.
<URL> specifies the URL of the object that the client is requesting from server. The URL
format is http://hostname[:port]/filepath.


main functions:
	splitUrl - split the URL to host, port and file path.
    buildRequest - build the request according of a http request format.
    sendRequest - open a sockect to the right host and send the http request ang print the response.



	




