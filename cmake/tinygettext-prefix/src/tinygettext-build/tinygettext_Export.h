
#ifndef TINYGETTEXT_API_H
#define TINYGETTEXT_API_H

#ifdef tinygettext_BUILT_AS_STATIC
#  define TINYGETTEXT_API
#  define TINYGETTEXT_NO_EXPORT
#else
#  ifndef TINYGETTEXT_API
#    ifdef tinygettext_EXPORTS
        /* We are building this library */
#      define TINYGETTEXT_API 
#    else
        /* We are using this library */
#      define TINYGETTEXT_API 
#    endif
#  endif

#  ifndef TINYGETTEXT_NO_EXPORT
#    define TINYGETTEXT_NO_EXPORT 
#  endif
#endif

#ifndef TINYGETTEXT_DEPRECATED
#  define TINYGETTEXT_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef TINYGETTEXT_DEPRECATED_EXPORT
#  define TINYGETTEXT_DEPRECATED_EXPORT TINYGETTEXT_API TINYGETTEXT_DEPRECATED
#endif

#ifndef TINYGETTEXT_DEPRECATED_NO_EXPORT
#  define TINYGETTEXT_DEPRECATED_NO_EXPORT TINYGETTEXT_NO_EXPORT TINYGETTEXT_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef TINYGETTEXT_NO_DEPRECATED
#    define TINYGETTEXT_NO_DEPRECATED
#  endif
#endif

#endif /* TINYGETTEXT_API_H */
