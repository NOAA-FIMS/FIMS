
#ifndef FIMS_EXPORT_H
#define FIMS_EXPORT_H

#ifdef FIMS_STATIC_DEFINE
#  define FIMS_EXPORT
#  define FIMS_NO_EXPORT
#else
#  ifndef FIMS_EXPORT
#    ifdef FIMS_EXPORTS
        /* We are building this library */
#      define FIMS_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define FIMS_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef FIMS_NO_EXPORT
#    define FIMS_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef FIMS_DEPRECATED
#  define FIMS_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef FIMS_DEPRECATED_EXPORT
#  define FIMS_DEPRECATED_EXPORT FIMS_EXPORT FIMS_DEPRECATED
#endif

#ifndef FIMS_DEPRECATED_NO_EXPORT
#  define FIMS_DEPRECATED_NO_EXPORT FIMS_NO_EXPORT FIMS_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef FIMS_NO_DEPRECATED
#    define FIMS_NO_DEPRECATED
#  endif
#endif

#endif /* FIMS_EXPORT_H */
