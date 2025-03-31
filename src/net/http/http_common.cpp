#include <zg/net/http/http_common.hpp>
#include <zg/Serial.hpp>
#define RESPONSE_INDEX_PROTOCOL    0
#define RESPONSE_INDEX_VERSION     1
#define RESPONSE_INDEX_STATUS_CODE 2
#define RESPONSE_INDEX_STATUS_TEXT 3
#define RESPONSE_INDEX_HEADERS     4
template <>
Serial& deserialize(Serial& serial, zg::net::http::http_response& response)
{
    int pieceIndex = 0;
    bool lowercasePiece = true;
    std::string piece;
    bool firstCharOfPiece = true;
    std::string searchFor;
    std::vector<std::string> endWiths;  
    bool appendLastChar;
    std::vector<std::string> array;
    bool isArray = false;
    std::vector<std::string> arrayEndWiths;
    std::vector<unsigned char> skip;
    bool justReadEndsWith = false;
    unsigned char ch = 0;
    unsigned char last4ch[4] = {0};
    unsigned char last4Index = 4;
    auto resetLast4ch = [&]
    {
        for (auto i = 0; i < 4; i++)
        {
            last4ch[i] = 0;
        }
        last4Index = 4;
    };
    while (true)
    {
        for (auto i = 0; i < 3; i++)
        {
            last4ch[i] = last4ch[i+1];
        }
        if (last4Index > 0)
            --last4Index;
        ch = serial.readByte();
        last4ch[3] = ch;
        if (lowercasePiece)
        {
            ch = std::tolower(ch);
        }
        if (firstCharOfPiece)
        {
            if (pieceIndex == RESPONSE_INDEX_PROTOCOL)
            {
                appendLastChar = false;
                if(ch == 'h')
                {
                    endWiths.push_back("/");
                }
                else
                {
                    return serial;
                }
            }
            else if (pieceIndex == RESPONSE_INDEX_VERSION)
            {
                appendLastChar = false;
                endWiths.push_back(" ");
            }
            else if (pieceIndex == RESPONSE_INDEX_STATUS_CODE)
            {
                appendLastChar = false;
                endWiths.push_back(" ");
            }
            else if (pieceIndex == RESPONSE_INDEX_STATUS_TEXT)
            {
                appendLastChar = false;
                endWiths.push_back("\n");
            }
            else if (pieceIndex == RESPONSE_INDEX_HEADERS)
            {
                appendLastChar = false;
                isArray = true;
                endWiths.push_back("\n");
                endWiths.push_back("\r\n");
                arrayEndWiths.push_back("\n\n");
                arrayEndWiths.push_back("\r\n\r\n");
                skip.push_back('\n');
                skip.push_back('\r');
            }
            firstCharOfPiece = false;
        }
        std::string last4String(last4ch + last4Index, last4ch + 4);
        if (isArray && justReadEndsWith)
        {
            bool foundArrayEndWith = false;
            for (auto& endWith : arrayEndWiths)
            {
                if (last4String.find(endWith) != std::string::npos)
                {
                    foundArrayEndWith = true;
                }
            }
            if (foundArrayEndWith)
            {
                goto _appendPiece;
            }
        }
        if (!justReadEndsWith)
        {
            bool foundEndWith = false;
            for (auto& endWith : endWiths)
            {
                if (last4String.find(endWith) != std::string::npos)
                {
                    foundEndWith = true;
                }
            }
            if (foundEndWith)
            {
                if (appendLastChar)
                    piece.insert(piece.end(), ch);
                if (isArray)
                {
                    array.push_back(piece);
                    piece.clear();
                    justReadEndsWith = true;
                    continue;
                }
                else
                {
                    goto _appendPiece;
                }
            }
        }
        if (std::find(skip.begin(), skip.end(), ch) != skip.end())
        {
            continue;
        }
        else
        {
            resetLast4ch();
        }
        piece.insert(piece.end(), ch);
        justReadEndsWith = false;
        if (!searchFor.empty() && piece == searchFor)
        {
            goto _appendPiece;
        }
        continue;
_appendPiece:
        if (pieceIndex == RESPONSE_INDEX_PROTOCOL)
            response.protocol = piece;
        else if (pieceIndex == RESPONSE_INDEX_VERSION)
            response.version = piece;
        else if (pieceIndex == RESPONSE_INDEX_STATUS_CODE)
            response.statusCode = piece;
        else if (pieceIndex == RESPONSE_INDEX_STATUS_TEXT)
            response.statusText = piece;
        else if (pieceIndex == RESPONSE_INDEX_HEADERS)
        {
            for (auto& str : array)
            {
                auto colon = str.find(":");
                auto key = str.substr(0, colon);
                auto value = str.substr(colon + 1, str.size() - 1);
                if (value.size() && value[0] == ' ')
                    value.erase(value.begin());
                response.headers[key] = value;
            }
            auto contentLengthIter = response.headers.find("Content-Length");
            if (contentLengthIter == response.headers.end())
            {
                break;
            }
            if (contentLengthIter->second.empty())
            {
                break;
            }
            size_t contentLength = std::stoll(contentLengthIter->second);
            std::shared_ptr<int8_t> body((int8_t*)malloc(contentLength + 1), free);
            memset(body.get(), 0, contentLength + 1);
            serial.readBytes((char*)body.get(), contentLength);
            response.body = {contentLength, body};
            break;
        }
        pieceIndex++;
        if (pieceIndex > RESPONSE_INDEX_PROTOCOL)
        {
            lowercasePiece = false;
        }
        firstCharOfPiece = true;
        appendLastChar = false;
        searchFor.clear();
        endWiths.clear();
        piece.clear();
        array.clear();
        isArray = false;
        arrayEndWiths.clear();
        skip.clear();
        resetLast4ch();
    }
    return serial;
}
template<>
Serial& serialize(Serial& serial, const zg::net::http::http_request& request)
{
    std::string header;
    header += request.verb + " " + request.path + " HTTP/1.1\r\n";
    header += "Host: " + request.host + "\r\n";
    for (auto& headerPair : request.headers)
    {
        header += headerPair.first + ": " + headerPair.second + "\r\n";
    }
    header += "Content-Length: " + std::to_string(request.body.first) + "\r\n";
    header += "\r\n";
    serial.writeBytes(header.c_str(), header.size());
    serial.writeBytes((const char *)request.body.second.get(), request.body.first);
    return serial;
}