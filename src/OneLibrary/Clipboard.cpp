#include <OneLibrary/Clipboard.h>

namespace ol
{
    ol::Clipboard::Clipboard() {};
    ol::Clipboard::~Clipboard() {};

    void ol::Clipboard::SetContents(const std::string& kContents)
    {
        this->m_sBuffer = kContents;
    }

    std::string ol::Clipboard::GetContents() const
    {
        return this->m_sBuffer;
    }

    std::istream& operator>>(std::istream& stream, ol::Clipboard& clipboard)
    {
        std::string contents{};
        stream >> contents;
        clipboard.SetContents(contents);
        return stream;
    }

    std::ostream& operator<<(std::ostream& stream, const ol::Clipboard& kClipboard)
    {
        stream << kClipboard.GetContents();
        return stream;
    }

    std::stringstream& operator>>(std::stringstream& stream, ol::Clipboard& clipboard)
    {
        std::string contents{};
        stream >> contents;
        clipboard.SetContents(contents);
        return stream;
    }

    std::istringstream& operator>>(std::istringstream& stream, ol::Clipboard& clipboard)
    {
        std::string contents{};
        stream >> contents;
        clipboard.SetContents(contents);
        return stream;
    }

    std::stringstream& operator<<(std::stringstream& stream, const ol::Clipboard& kClipboard)
    {
        stream << kClipboard.GetContents();
        return stream;
    }

    std::ostringstream& operator<<(std::ostringstream& stream, const ol::Clipboard& kClipboard)
    {
        stream << kClipboard.GetContents();
        return stream;
    }
}

