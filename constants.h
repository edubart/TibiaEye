#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace Constants {

#define APP_VERSION_MAJOR 0
#define APP_VERSION_MINOR 4
#define APP_VERSION_RELEASE 0

#define STRINGIFY_INTERNAL(x) #x
#define STRINGIFY(x) STRINGIFY_INTERNAL(x)

#define APP_VERSION STRINGIFY(APP_VERSION_MAJOR) \
	"." STRINGIFY(APP_VERSION_MINOR) \
	"." STRINGIFY(APP_VERSION_RELEASE)

#define APP_BUILD   __DATE__ ", " __TIME__

const char * const APP_NAME              = "Tibia Eye";
const char * const APP_COMPANY           = "Tibia Eye";
const char * const APP_VERSION_LONG      = APP_VERSION;
const char * const APP_BUILD_LONG        = APP_BUILD;
const char * const APP_AUTHOR            = "Eduardo Bart";
const char * const APP_CONTACT           = "contact@tibiaeye.com";
const char * const APP_WEBSITE           = "http://www.tibiaeye.com/";
const char * const APP_YEAR              = "2009";

#ifdef APP_REVISION
const char * const APP_REVISION_STR      = STRINGIFY(APP_REVISION);
#endif

}

#endif // CONSTANTS_H
