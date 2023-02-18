# wss
The goal is to have a web framework, with built-in WSS and basic HTTPS server. Hopefully this can work in conjunction with PHP framework, and slowly transition what needs to be in C++ here.

Folder setup:
- build: build files
- certs: SSL certificates
- design_docs: client requirement / design documents and research documents
- lib: all 3rd party libraries + jahan library
  * jahan: jahan library that will be shared between all projects
  * ... :any other 3rd party library. Goal is to minimize use of third party libraries as much as possible. When a 3rd party library is used, we try to static link it and copy the source here. Boost is the exception.
- src :project specific files + main where all the server setup happens.
