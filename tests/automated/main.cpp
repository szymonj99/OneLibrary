// TODO: Try sorting out linking against Catch2::Catch2WithMain when using a CMake Preset with UNICODE defined.
#include <catch2/catch_session.hpp>

#include <iostream>

int main( int argc, char* argv[] ) {
#ifdef UNICODE
    std::cout << "Unicode Defined\n";
#endif
#ifdef _UNICODE
    std::cout << "_Unicode Defined\n";
#endif
#ifdef WIN32
    std::cout << "WIN32 Defined\n";
#endif
#ifdef _WIN32
    std::cout << "_WIN32 Defined\n";
#endif
    return Catch::Session().run( argc, argv );
}
