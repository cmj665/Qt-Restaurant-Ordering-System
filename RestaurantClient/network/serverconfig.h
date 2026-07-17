#ifndef RESTAURANTCLIENT_SERVERCONFIG_H
#define RESTAURANTCLIENT_SERVERCONFIG_H

#include <QString>
#include <QUrl>

/** 客户端访问服务端时使用的统一地址配置。 */
namespace ServerConfig {

inline const QString BaseUrl = QStringLiteral("http://localhost:8080");

/** 拼接 REST 路径；path 应以斜杠开头。 */
inline QUrl apiUrl(const QString &path)
{
    return QUrl(BaseUrl + path);
}

/** 将数据库中的图片字段统一转换为可访问的绝对 URL。 */
inline QUrl imageUrl(const QString &path)
{
    if (path.startsWith(QStringLiteral("http://"))
        || path.startsWith(QStringLiteral("https://"))) {
        return QUrl(path);
    }
    return apiUrl(path.startsWith(QLatin1Char('/'))
                      ? path
                      : QStringLiteral("/images/") + path);
}

} // namespace ServerConfig

#endif // RESTAURANTCLIENT_SERVERCONFIG_H
