#ifndef RESTAURANTADMIN_SERVERCONFIG_H
#define RESTAURANTADMIN_SERVERCONFIG_H

#include <QString>
#include <QUrl>

/** 管理端访问服务端时使用的统一地址配置。 */
namespace ServerConfig {

inline const QString BaseUrl = QStringLiteral("http://localhost:8080");

inline QUrl apiUrl(const QString &path)
{
    return QUrl(BaseUrl + path);
}

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

#endif // RESTAURANTADMIN_SERVERCONFIG_H
