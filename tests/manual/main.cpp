// TODO: Try sorting out linking against Catch2::Catch2WithMain when using a CMake Preset with UNICODE defined.
#include <catch2/catch_session.hpp>

int main( int argc, char* argv[] ) {
    return Catch::Session().run( argc, argv );
}
