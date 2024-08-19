// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/http/bridge.hpp>

#include <iscool/http/request.hpp>
#include <iscool/http/setup.hpp>

#include <axmol/network/HttpClient.h>

static void send_request(const iscool::http::request& request)
{
  ax::network::HttpRequest* const ax_request(new ax::network::HttpRequest());

  ax_request->setUrl(request.get_url());

  switch (request.get_type())
    {
    case iscool::http::request::type::get:
      ax_request->setRequestType(ax::network::HttpRequest::Type::GET);
      break;
    case iscool::http::request::type::post:
      ax_request->setRequestType(ax::network::HttpRequest::Type::POST);
      ax_request->setHeaders(request.get_headers());
      ax_request->setRequestData(request.get_body().c_str(),
                                 request.get_body().size());
      break;
    }

  ax_request->setResponseCallback(
      [=](ax::network::HttpClient*,
          ax::network::HttpResponse* response) -> void
      {
        const auto& ax_buffer = *response->getResponseData();
        std::vector<char> data(std::begin(ax_buffer), std::end(ax_buffer));

        request.get_response_handler()(iscool::http::response(
            response->getResponseCode(), std::move(data)));

        response->getHttpRequest()->release();
      });

  ax::network::HttpClient::getInstance()->send(ax_request);
}

bim::axmol::http::bridge::bridge()
{
  iscool::http::initialize(&::send_request);
}

bim::axmol::http::bridge::~bridge()
{
  iscool::http::finalize();
  ax::network::HttpClient::destroyInstance();
}
