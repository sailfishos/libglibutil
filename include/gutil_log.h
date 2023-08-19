/*
 * Copyright (C) 2014-2023 Slava Monich <slava@monich.com>
 * Copyright (C) 2014-2022 Jolla Ltd.
 *
 * You may use this file under the terms of BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *   3. Neither the names of the copyright holders nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef GUTIL_LOG_H
#define GUTIL_LOG_H

#include "gutil_types.h"
#include <stdarg.h>

G_BEGIN_DECLS

/* Log levels */
#define GLOG_LEVEL_ALWAYS         (-2)
#define GLOG_LEVEL_INHERIT        (-1)
#define GLOG_LEVEL_NONE           (0)
#define GLOG_LEVEL_ERR            (1)
#define GLOG_LEVEL_WARN           (2)
#define GLOG_LEVEL_INFO           (3)
#define GLOG_LEVEL_DEBUG          (4)
#define GLOG_LEVEL_VERBOSE        (5)

/* Allow these to be redefined */
#ifndef GLOG_LEVEL_MAX
#  ifdef DEBUG
#    define GLOG_LEVEL_MAX        GLOG_LEVEL_VERBOSE
#  else
#    define GLOG_LEVEL_MAX        GLOG_LEVEL_DEBUG
#  endif
#endif /* GLOG_LEVEL_MAX */

#ifndef GLOG_LEVEL_DEFAULT
#  ifdef DEBUG
#    define GLOG_LEVEL_DEFAULT    GLOG_LEVEL_DEBUG
#  else
#    define GLOG_LEVEL_DEFAULT    GLOG_LEVEL_INFO
#  endif
#endif /* GLOG_LEVEL_DEFAULT */

/* Do we need a separate log level for ASSERTs? */
#ifndef GLOG_LEVEL_ASSERT
#  ifdef DEBUG
#    define GLOG_LEVEL_ASSERT     GLOG_LEVEL_ERR
#  else
     /* No asserts in release build */
#    define GLOG_LEVEL_ASSERT     (GLOG_LEVEL_MAX+1)
#  endif
#endif

/* Logging function prototypes */
#define GUTIL_DEFINE_LOG_FN(fn)  void fn(const char* name, int level, \
    const char* format, va_list va)
#define GUTIL_DEFINE_LOG_FN2(fn)  void fn(const GLogModule* module, \
    int level, const char* format, va_list va)
typedef GUTIL_DEFINE_LOG_FN((*GLogProc));
typedef GUTIL_DEFINE_LOG_FN2((*GLogProc2));

/* Log module */
struct glog_module {
    const char* name;               /* Name (used as prefix) */
    const GLogModule* parent;       /* Parent log module (optional) */
    GLogProc2 log_proc;             /* Per-module logging function (1.0.43) */
    const int max_level;            /* Maximum level defined at compile time */
    int level;                      /* Current log level */
    int flags;                      /* Flags (see below) */
    int reserved2;                  /* Reserved for future expansion */
};

#define GLOG_FLAG_HIDE_NAME  (0x01) /* Don't print the module name */
#define GLOG_FLAG_DISABLE    (0x02) /* Don't print this log */

/* Command line parsing helper. Option format is [module]:level
 * where level can be either a number or log level name ("none", err etc.) */
gboolean
gutil_log_parse_option(
    const char* opt,                /* String to parse */
    GLogModule** modules,           /* Known modules */
    int count,                      /* Number of known modules */
    GError** error);                /* Optional error message */

/* Set log type by name ("syslog", "stdout" or "glib"). This is also
 * primarily for parsing command line options */
gboolean
gutil_log_set_type(
    const char* type,
    const char* default_name);

const char*
gutil_log_get_type(
    void);

/* Generates the string containg description of log levels and list of
 * log modules. The caller must deallocate the string with g_free */
char*
gutil_log_description(
    GLogModule** modules,           /* Known modules */
    int count);                     /* Number of known modules */

/* Logging function */
void
gutil_log(
    const GLogModule* module,       /* Calling module (NULL for default) */
    int level,                      /* Message log level */
    const char* format,             /* Message format */
    ...) G_GNUC_PRINTF(3,4);        /* Followed by arguments */

void
gutil_logv(
    const GLogModule* module,
    int level,
    const char* format,
    va_list va);

void
gutil_log_dump(
    const GLogModule* module,
    int level,
    const char* prefix,
    const void* data,
    gsize size); /* Since 1.0.55 */

void
gutil_log_dump_bytes(
    const GLogModule* module,
    int level,
    const char* prefix,
    GBytes* bytes); /* Since 1.0.67 */

/* Check if logging is enabled for the specified log level */
gboolean
gutil_log_enabled(
    const GLogModule* module,
    int level);

/* Known log types */
extern const char GLOG_TYPE_STDOUT[];
extern const char GLOG_TYPE_STDERR[];
extern const char GLOG_TYPE_GLIB[];
extern const char GLOG_TYPE_CUSTOM[];
extern const char GLOG_TYPE_SYSLOG[];

/* Available log handlers */
GUTIL_DEFINE_LOG_FN(gutil_log_stdout);
GUTIL_DEFINE_LOG_FN(gutil_log_stderr);
GUTIL_DEFINE_LOG_FN(gutil_log_glib);
GUTIL_DEFINE_LOG_FN(gutil_log_syslog);
GUTIL_DEFINE_LOG_FN2(gutil_log_stdout2);  /* Since 1.0.43 */
GUTIL_DEFINE_LOG_FN2(gutil_log_stderr2);  /* Since 1.0.43 */
GUTIL_DEFINE_LOG_FN2(gutil_log_glib2);    /* Since 1.0.43 */
GUTIL_DEFINE_LOG_FN2(gutil_log_syslog2);  /* Since 1.0.43 */

/* Log configuration */
GLOG_MODULE_DECL(gutil_log_default)
extern GLogProc gutil_log_func;
extern GLogProc2 gutil_log_func2;
extern gboolean gutil_log_timestamp; /* Only affects stdout and stderr */
extern gboolean gutil_log_tid;       /* Since 1.0.51 */

/* Log module (optional) */
#define GLOG_MODULE_DEFINE_(var,name) \
  GLogModule var = {name, NULL, NULL, \
  GLOG_LEVEL_MAX, GLOG_LEVEL_INHERIT, 0, 0}
#define GLOG_MODULE_DEFINE2_(var,name,parent) \
  GLogModule var = {name, &(parent), NULL, \
  GLOG_LEVEL_MAX, GLOG_LEVEL_INHERIT, 0, 0}
#ifdef GLOG_MODULE_NAME
extern GLogModule GLOG_MODULE_NAME;
#  define GLOG_MODULE_CURRENT   (&GLOG_MODULE_NAME)
#  define GLOG_MODULE_DEFINE(name) \
    GLOG_MODULE_DEFINE_(GLOG_MODULE_NAME,name)
#  define GLOG_MODULE_DEFINE2(name,parent) \
    GLOG_MODULE_DEFINE2_(GLOG_MODULE_NAME,name,parent)
#else
#  define GLOG_MODULE_CURRENT   NULL
#endif

/* Logging macros */

#define GLOG_NOTHING ((void)0)
#define GLOG_ENABLED(level)   gutil_log_enabled(GLOG_MODULE_CURRENT,level)
#define GERRMSG(err) (((err) && (err)->message) ? (err)->message : \
  "Unknown error")

#if !defined(GLOG_VARARGS) && defined(__GNUC__)
#  define GLOG_VARARGS
#endif

#ifndef GLOG_VARARGS
#  define GLOG_VA_FN(x) _gutil_log_##x
#  define GLOG_VA_NONE(x) G_INLINE_FUNC \
    void GLOG_VA_FN(x)(const char* f, ...) {}
#  define GLOG_VA(x) GLOG_VA_(GLOG_VA_FN(x),x)
#  define GLOG_VA_(fn,x) G_INLINE_FUNC \
    void fn(const char* f, ...) { \
    if (f && f[0]) {                                                    \
        va_list va; va_start(va,f);                                     \
        gutil_logv(GLOG_MODULE_CURRENT, GLOG_LEVEL_##x, f, va);         \
        va_end(va);                                                     \
    }                                                                   \
}
#endif /* GLOG_VARARGS */

#define GUTIL_LOG_ANY           (GLOG_LEVEL_MAX >= GLOG_LEVEL_NONE)
#define GUTIL_LOG_ERR           (GLOG_LEVEL_MAX >= GLOG_LEVEL_ERR)
#define GUTIL_LOG_WARN          (GLOG_LEVEL_MAX >= GLOG_LEVEL_WARN)
#define GUTIL_LOG_INFO          (GLOG_LEVEL_MAX >= GLOG_LEVEL_INFO)
#define GUTIL_LOG_DEBUG         (GLOG_LEVEL_MAX >= GLOG_LEVEL_DEBUG)
#define GUTIL_LOG_VERBOSE       (GLOG_LEVEL_MAX >= GLOG_LEVEL_VERBOSE)
#define GUTIL_LOG_ASSERT        (GLOG_LEVEL_MAX >= GLOG_LEVEL_ASSERT)

#if GUTIL_LOG_ASSERT
void
gutil_log_assert(
    const GLogModule* module, /* Calling module (NULL for default) */
    int level,                /* Assert log level */
    const char* expr,         /* Assert expression */
    const char* file,         /* File name */
    int line);                /* Line number */
#  define GASSERT(expr)         ((expr) ? GLOG_NOTHING : \
   gutil_log_assert(GLOG_MODULE_CURRENT, GLOG_LEVEL_ASSERT, \
   #expr, __FILE__, __LINE__))
#  define GVERIFY(expr)         GASSERT(expr)
#  define GVERIFY_FALSE(expr)   GASSERT(!(expr))
#  define GVERIFY_EQ(expr,val)  GASSERT((expr) == (val))
#  define GVERIFY_NE(expr,val)  GASSERT((expr) != (val))
#  define GVERIFY_GE(expr,val)  GASSERT((expr) >= (val))
#  define GVERIFY_GT(expr,val)  GASSERT((expr) >  (val))
#  define GVERIFY_LE(expr,val)  GASSERT((expr) <= (val))
#  define GVERIFY_LT(expr,val)  GASSERT((expr) <  (val))
#else
#  define GASSERT(expr)
#  define GVERIFY(expr)         (expr)
#  define GVERIFY_FALSE(expr)   (expr)
#  define GVERIFY_EQ(expr,val)  (expr)
#  define GVERIFY_NE(expr,val)  (expr)
#  define GVERIFY_GE(expr,val)  (expr)
#  define GVERIFY_GT(expr,val)  (expr)
#  define GVERIFY_LE(expr,val)  (expr)
#  define GVERIFY_LT(expr,val)  (expr)
#endif

#ifdef GLOG_VARARGS
#  if GUTIL_LOG_ERR
#    define GERR(f,args...)     gutil_log(GLOG_MODULE_CURRENT, \
       GLOG_LEVEL_ERR, f, ##args)
#    define GERR_(f,args...)    gutil_log(GLOG_MODULE_CURRENT, \
       GLOG_LEVEL_ERR, "%s() " f, __FUNCTION__, ##args)
#  else
#    define GERR(f,args...)     GLOG_NOTHING
#    define GERR_(f,args...)    GLOG_NOTHING
#  endif /* GUTIL_LOG_ERR */
#else
#  define GERR_                 GERR
#  define GERR                  GLOG_VA_FN(ERR)
#  if GUTIL_LOG_ERR
     GLOG_VA(ERR)
#  else
     GLOG_VA_NONE(ERR)
#  endif /* GUTIL_LOG_ERR */
#endif /* GLOG_VARARGS */

#ifdef GLOG_VARARGS
#  if GUTIL_LOG_WARN
#    define GWARN(f,args...)    gutil_log(GLOG_MODULE_CURRENT, \
       GLOG_LEVEL_WARN, f, ##args)
#    define GWARN_(f,args...)   gutil_log(GLOG_MODULE_CURRENT, \
       GLOG_LEVEL_WARN, "%s() " f, __FUNCTION__, ##args)
#  else
#    define GWARN(f,args...)    GLOG_NOTHING
#    define GWARN_(f,args...)   GLOG_NOTHING
#  endif /* GUTIL_LOGL_WARN */
#else
#  define GWARN_                GWARN
#  define GWARN                 GLOG_VA_FN(WARN)
#  if GUTIL_LOG_WARN
     GLOG_VA(WARN)
#  else
     GLOG_VA_NONE(WARN)
#  endif /* GUTIL_LOGL_WARN */
#  endif /* GLOG_VARARGS */

#ifdef GLOG_VARARGS
#  if GUTIL_LOG_INFO
#    define GINFO(f,args...)    gutil_log(GLOG_MODULE_CURRENT, \
       GLOG_LEVEL_INFO, f, ##args)
#    define GINFO_(f,args...)   gutil_log(GLOG_MODULE_CURRENT, \
       GLOG_LEVEL_INFO, "%s() " f, __FUNCTION__, ##args)
#  else
#    define GINFO(f,args...)    GLOG_NOTHING
#    define GINFO_(f,args...)   GLOG_NOTHING
#  endif /* GUTIL_LOG_INFO */
#else
#  define GINFO_                GINFO
#  define GINFO                 GLOG_VA_FN(INFO)
#  if GUTIL_LOG_INFO
     GLOG_VA(INFO)
#  else
     GLOG_VA_NONE(INFO)
#  endif /* GUTIL_LOG_INFO */
#endif /* GLOG_VARARGS */

#ifdef GLOG_VARARGS
#  if GUTIL_LOG_DEBUG
#    define GDEBUG(f,args...)   gutil_log(GLOG_MODULE_CURRENT, \
       GLOG_LEVEL_DEBUG, f, ##args)
#    define GDEBUG_(f,args...)  gutil_log(GLOG_MODULE_CURRENT, \
       GLOG_LEVEL_DEBUG, "%s() " f, __FUNCTION__, ##args)
#    define GDEBUG_DUMP(buf,n)  gutil_log_dump(GLOG_MODULE_CURRENT, \
       GLOG_LEVEL_DEBUG, NULL, buf, n) /* Since 1.0.55 */
#    define GDEBUG_DUMP_BYTES(b) gutil_log_dump_bytes(GLOG_MODULE_CURRENT, \
       GLOG_LEVEL_DEBUG, NULL, b) /* Since 1.0.67 */
#  else
#    define GDEBUG(f,args...)   GLOG_NOTHING
#    define GDEBUG_(f,args...)  GLOG_NOTHING
#    define GDEBUG_DUMP(buf,n)  GLOG_NOTHING /* Since 1.0.55 */
#    define GDEBUG_DUMP_BYTES(b) GLOG_NOTHING /* Since 1.0.67 */
#  endif /* GUTIL_LOG_DEBUG */
#else
#  define GDEBUG_               GDEBUG
#  define GDEBUG                GLOG_VA_FN(debug)
#  if GUTIL_LOG_DEBUG
     GLOG_VA_(GDEBUG,DEBUG)
#  else
     GLOG_VA_NONE_(GDEBUG,DEBUG)
#  endif /* GUTIL_LOG_DEBUG */
#endif /* GLOG_VARARGS */

#ifdef GLOG_VARARGS
#  if GUTIL_LOG_VERBOSE
#    define GVERBOSE(f,args...)  gutil_log(GLOG_MODULE_CURRENT, \
       GLOG_LEVEL_VERBOSE, f, ##args)
#    define GVERBOSE_(f,args...) gutil_log(GLOG_MODULE_CURRENT, \
       GLOG_LEVEL_VERBOSE, "%s() " f, __FUNCTION__, ##args)
#  else
#    define GVERBOSE(f,args...)  GLOG_NOTHING
#    define GVERBOSE_(f,args...) GLOG_NOTHING
#  endif /* GUTIL_LOG_VERBOSE */
#else
#  define GVERBOSE_              GVERBOSE
#  define GVERBOSE               GLOG_VA_FN(VERBOSE)
#  if GUTIL_LOG_VERBOSE
     GLOG_VA(VERBOSE)
#  else
     GLOG_VA_NONE(VERBOSE)
#  endif /* GUTIL_LOG_VERBOSE */
#endif /* GLOG_VARARGS */

G_END_DECLS

#endif /* GUTIL_LOG_H */

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
