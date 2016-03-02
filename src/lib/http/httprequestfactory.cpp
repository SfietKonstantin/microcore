#include "httprequestfactory.h"
#include <QNetworkReply>

using namespace ::microcore::core;

namespace microcore { namespace http {

class HttpGetRequestJob: public IJob<QByteArray, QString>
{
public:
    explicit HttpGetRequestJob(QNetworkAccessManager &network, QNetworkRequest &&request)
        : m_network(network), m_request(std::move(request))
    {
    }
    void execute(ICallback &callback) override
    {
        m_reply = m_network.get(m_request);
        QObject::connect(m_reply, &QNetworkReply::finished, [this, &callback]() {
            callback.onResult(m_reply->readAll());
        });
    }
private:
    QNetworkAccessManager &m_network;
    QNetworkRequest m_request {};
    QNetworkReply *m_reply {}; // TODO use QObjectPtr
};

HttpRequestFactory::HttpRequestFactory(QNetworkAccessManager &network)
    : m_network(network)
{
}

std::unique_ptr<IJob<QByteArray, QString>> HttpRequestFactory::create(HttpRequest &&request) const
{
    switch (request.type())
    {
    case HttpRequest::Type::Get:
        return std::unique_ptr<IJob<QByteArray, QString>>(new HttpGetRequestJob(m_network, request.request()));
    default:
        Q_ASSERT(false);
        return std::unique_ptr<IJob<QByteArray, QString>>();
    }
}

}}
