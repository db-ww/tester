#ifndef SR_HTTP_HANDLERS_H
#define SR_HTTP_HANDLERS_H

#include "globals.h"

#if ENABLE_HTTP
#include <HTTPServer.hpp>
#include <HTTPSServer.hpp>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>
#include <SSLCert.hpp>

#include <functional>

using namespace httpsserver;

void handleRoot(HTTPRequest * req, HTTPResponse * res);
void handleStart(HTTPRequest * req, HTTPResponse * res);
void handleReadings(HTTPRequest * req, HTTPResponse * res);
void handleConfig(HTTPRequest * req, HTTPResponse * res);
void middlewareAuthentication(HTTPRequest * req, HTTPResponse * res, std::function<void()> next);
void registerRoutes(HTTPServer *srv);
void setupHTTPServer();
void setupHTTPSServer();
#endif

#endif // SR_HTTP_HANDLERS_H
