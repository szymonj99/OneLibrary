#pragma once

#include <string>
#include <sstream>
#include <iostream>

namespace ol
{
    class Clipboard
    {
    private:
        std::string m_sBuffer{};

    public:
        Clipboard();
        ~Clipboard();
        void SetContents(const std::string& kContents);
        [[nodiscard]] std::string GetContents() const;
        bool operator==(const ol::Clipboard& kClipboard) const = default;
    };

    // TODO: Current issue: As the user can copy a string with spaces in it, the clipboard data would get separated on the delimiter.
    // Let's call to a base64 encoder first, then decode it on the receiver end.

    std::istream& operator>>(std::istream& stream, ol::Clipboard& clipboard);
    std::ostream& operator<<(std::ostream& stream, const ol::Clipboard& kClipboard);

    std::stringstream& operator>>(std::stringstream& stream, ol::Clipboard& clipboard);
    std::istringstream& operator>>(std::istringstream& stream, ol::Clipboard& clipboard);

    std::stringstream& operator<<(std::stringstream& stream, const ol::Clipboard& kClipboard);
    std::ostringstream& operator<<(std::ostringstream& stream, const ol::Clipboard& kClipboard);
}