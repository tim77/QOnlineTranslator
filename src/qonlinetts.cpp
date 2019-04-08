#include "qonlinetts.h"

#include <QUrl>

constexpr int googleTtsLimit = 200;
constexpr int yandexTtsLimit = 1400;
constexpr int bingTtsLimit = 2001;

QOnlineTts::QOnlineTts(QObject *parent) :
    QObject(parent)
{
}

void QOnlineTts::generateUrls(const QString &text, QOnlineTranslator::Engine engine, QOnlineTranslator::Language lang, QOnlineTts::Voice voice, QOnlineTts::Emotion emotion)
{
    // Get codes
    const QString langString = ttsLanguageCode(engine, lang);
    if (langString.isEmpty()) {
        setError(UnsupportedLanguage, tr("Selected language %1 is not supported by: %2").arg(lang).arg(engine));
        return;
    }

    const QString voiceString = voiceCode(engine, voice);
    if (voiceString.isEmpty()) {
        setError(UnsupportedVoice, tr("Selected voice %1 is not supported by: %2").arg(voice).arg(engine));
        return;
    }

    const QString emotionString = emotionCode(engine, emotion);
    if (emotionString.isEmpty()) {
        setError(UnsupportedEmotion, tr("Selected emotion %1 is not supported by: %2").arg(emotion).arg(engine));
        return;
    }

    // Get speech
    QString unparsedText = text;
    switch (engine) {
    case QOnlineTranslator::Google:
        // Google has a limit of characters per tts request. If the query is larger, then it should be splited into several
        while (!unparsedText.isEmpty()) {
            const int splitIndex = QOnlineTranslator::getSplitIndex(unparsedText, googleTtsLimit); // Split the part by special symbol

            // Generate URL API for add it to the playlist
            QUrl apiUrl("http://translate.googleapis.com/translate_tts");
#if defined(Q_OS_LINUX)
            apiUrl.setQuery("ie=UTF-8&client=gtx&tl=" + langString
                            + "&q=" + QUrl::toPercentEncoding(unparsedText.left(splitIndex)));
#elif defined(Q_OS_WIN)
            apiUrl.setQuery("ie=UTF-8&client=gtx&tl=" + langCode
                            + "&q=" + QUrl::toPercentEncoding(unparsedText.left(splitIndex)), QUrl::DecodedMode);
#endif
            m_media.append(apiUrl);

            // Remove the said part from the next saying
            unparsedText = unparsedText.mid(splitIndex);
        }
        break;
    case QOnlineTranslator::Yandex:
        // Yandex has a limit of characters per tts request. If the query is larger, then it should be splited into several
        while (!unparsedText.isEmpty()) {
            const int splitIndex = QOnlineTranslator::getSplitIndex(unparsedText, yandexTtsLimit); // Split the part by special symbol

            // Generate URL API for add it to the playlist
            QUrl apiUrl("https://tts.voicetech.yandex.net/tts");
#if defined(Q_OS_LINUX)
            apiUrl.setQuery("text=" + QUrl::toPercentEncoding(unparsedText.left(splitIndex))
                            + "&lang=" + langString
                            + "&speaker=" + voiceString
                            + "&emotion=" + emotionString
                            + "&format=mp3");
#elif defined(Q_OS_WIN)
            apiUrl.setQuery("text=" + QUrl::toPercentEncoding(unparsedText.left(splitIndex))
                            + "&lang=" + langCode
                            + "&speaker=" + voiceCode
                            + "&emotion=" + emotionCode
                            + "&format=mp3", QUrl::DecodedMode);
#endif
            m_media.append(apiUrl);

            // Remove the said part from the next saying
            unparsedText = unparsedText.mid(splitIndex);
        }
        break;
    case QOnlineTranslator::Bing:
        while (!unparsedText.isEmpty()) {
            const int splitIndex = QOnlineTranslator::getSplitIndex(unparsedText, bingTtsLimit); // Split the part by special symbol

            // Generate URL API for add it to the playlist
            QUrl apiUrl("https://www.bing.com/tspeak");
#if defined(Q_OS_LINUX)
            apiUrl.setQuery("&language=" + langString
                            + "&text=" + QUrl::toPercentEncoding(unparsedText.left(splitIndex))
                            + "&options=" + voiceString
                            + "&format=audio/mp3");
#elif defined(Q_OS_WIN)
            apiUrl.setQuery("&language=" + langCode
                            + "&text=" + QUrl::toPercentEncoding(unparsedText.left(splitIndex))
                            + "&options=" + voiceCode
                            + "&format=audio/mp3", QUrl::DecodedMode);
#endif
            m_media.append(apiUrl);

            // Remove the said part from the next saying
            unparsedText = unparsedText.mid(splitIndex);
        }
    }
}

QList<QMediaContent> QOnlineTts::media() const
{
    return m_media;
}

bool QOnlineTts::isSupportTts(QOnlineTranslator::Engine engine, QOnlineTranslator::Language lang)
{
    bool isSupported = false;

    switch (engine) {
    case QOnlineTranslator::Google:
        if (lang == QOnlineTranslator::Auto)
            isSupported = false;
        else
            isSupported = QOnlineTranslator::isSupportTranslation(engine, lang); // Google use the same codes for tts
        break;
    case QOnlineTranslator::Yandex:
        switch (lang) {
        case QOnlineTranslator::Russian:
        case QOnlineTranslator::Tatar:
        case QOnlineTranslator::English:
            isSupported = true;
            break;
        default:
            isSupported = false;
            break;
        }
        break;
    case QOnlineTranslator::Bing:
        switch (lang) {
        case QOnlineTranslator::Arabic:
        case QOnlineTranslator::Catalan:
        case QOnlineTranslator::Danish:
        case QOnlineTranslator::German:
        case QOnlineTranslator::English:
        case QOnlineTranslator::Spanish:
        case QOnlineTranslator::Finnish:
        case QOnlineTranslator::French:
        case QOnlineTranslator::Hindi:
        case QOnlineTranslator::Italian:
        case QOnlineTranslator::Japanese:
        case QOnlineTranslator::Korean:
        case QOnlineTranslator::Norwegian:
        case QOnlineTranslator::Dutch:
        case QOnlineTranslator::Polish:
        case QOnlineTranslator::Portuguese:
        case QOnlineTranslator::Russian:
        case QOnlineTranslator::Swedish:
        case QOnlineTranslator::SimplifiedChinese:
        case QOnlineTranslator::TraditionalChinese:
            isSupported = true;
            break;
        default:
            isSupported = false;
            break;
        }
    }

    return isSupported;
}

void QOnlineTts::setError(QOnlineTts::TtsError error, const QString &errorString)
{
    m_error = error;
    m_errorString = errorString;
}

// Returns engine-specific language code for tts
QString QOnlineTts::ttsLanguageCode(QOnlineTranslator::Engine engine, QOnlineTranslator::Language lang)
{
    switch (engine) {
    case QOnlineTranslator::Google:
        if (lang != QOnlineTranslator::Auto)
            return QOnlineTranslator::translationLanguageCode(engine, lang); // Google use the same codes for tts (except 'auto')
        break;
    case QOnlineTranslator::Yandex:
        switch (lang) {
        case QOnlineTranslator::Russian:
            return "ru_RU";
        case QOnlineTranslator::Tatar:
            return "tr_TR";
        case QOnlineTranslator::English:
            return "en_GB";
        default:
            break;
        }
        break;
    case QOnlineTranslator::Bing:
        switch (lang) {
        case QOnlineTranslator::Arabic:
            return "ar-EG";
        case QOnlineTranslator::Catalan:
            return "ca-ES";
        case QOnlineTranslator::Danish:
            return "da-DK";
        case QOnlineTranslator::German:
            return "de-DE";
        case QOnlineTranslator::English:
            return "en-GB";
        case QOnlineTranslator::Spanish:
            return "es-ES";
        case QOnlineTranslator::Finnish:
            return "fi-FI";
        case QOnlineTranslator::French:
            return "fr-FR";
        case QOnlineTranslator::Hindi:
            return "hi-IN";
        case QOnlineTranslator::Italian:
            return "it-IT";
        case QOnlineTranslator::Japanese:
            return "ja-JP";
        case QOnlineTranslator::Korean:
            return "ko-KR";
        case QOnlineTranslator::Norwegian:
            return "nb-NO";
        case QOnlineTranslator::Dutch:
            return "nl-NL";
        case QOnlineTranslator::Polish:
            return "pl-PL";
        case QOnlineTranslator::Portuguese:
            return "pt-PT";
        case QOnlineTranslator::Russian:
            return "ru-RU";
        case QOnlineTranslator::Swedish:
            return "sv-SE";
        case QOnlineTranslator::SimplifiedChinese:
            return "zh-CN";
        case QOnlineTranslator::TraditionalChinese:
            return "zh-HK";
        default:
            break;
        }
    }

    return QString();
}

QString QOnlineTts::voiceCode(QOnlineTranslator::Engine engine, Voice voice)
{
    switch (engine) {
    case QOnlineTranslator::Google:
        if (voice == DefaultVoice)
            return "default";
        break;
    case QOnlineTranslator::Yandex:
        switch (voice) {
        case DefaultVoice:
        case Zahar:
            return "zahar";
        case Ermil:
            return "ermil";
        case Jane:
            return "jane";
        case Oksana:
            return "oksana";
        case Alyss:
            return "alyss";
        case Omazh:
            return "omazh";
        default:
            break;
        }
        break;
    case QOnlineTranslator::Bing:
        switch (voice) {
        case DefaultVoice:
        case Male:
            return "male";
        case Female:
            return "female";
        default:
            break;
        }
        break;
    }

    return QString();
}

QString QOnlineTts::emotionCode(QOnlineTranslator::Engine engine, Emotion emotion)
{
    switch (engine) {
    case QOnlineTranslator::Google:
    case QOnlineTranslator::Bing:
        if (emotion == DefaultEmotion)
            return "default";
        break;
    case QOnlineTranslator::Yandex:
        switch (emotion) {
        case DefaultEmotion:
        case Neutral:
            return "neutral";
        case Good:
            return "good";
        case Evil:
            return "evil";
        }
    }

    return QString();
}