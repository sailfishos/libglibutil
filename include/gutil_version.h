/*
 * Copyright (C) 2023 Slava Monich <slava@monich.com>
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

#ifndef GUTIL_VERSION_H
#define GUTIL_VERSION_H

#include "gutil_types.h"

/*
 * GUTIL_VERSION_X_Y_Z macros will be added with each release. The fact that
 * such macro is defined means that you're compiling against libgutil version
 * X.Y.Z or greater.
 *
 * Since 1.0.69
 */

G_BEGIN_DECLS

#define GUTIL_VERSION_MAJOR   1
#define GUTIL_VERSION_MINOR   0
#define GUTIL_VERSION_MICRO   72
#define GUTIL_VERSION_STRING  "1.0.72"

extern const guint gutil_version_major; /* GUTIL_VERSION_MAJOR */
extern const guint gutil_version_minor; /* GUTIL_VERSION_MINOR */
extern const guint gutil_version_micro; /* GUTIL_VERSION_MICRO */

/* Version as a single word */
#define GUTIL_VERSION_(v1,v2,v3) \
    ((((v1) & 0x7f) << 24) | \
     (((v2) & 0xfff) << 12) | \
      ((v3) & 0xfff))

#define GUTIL_VERSION_MAJOR_(v)   (((v) >> 24) & 0x7f)
#define GUTIL_VERSION_MINOR_(v)   (((v) >> 12) & 0xfff)
#define GUTIL_VERSION_MICRO_(v)   (((v) & 0xfff))

/* Current compile time version as a single word */
#define GUTIL_VERSION GUTIL_VERSION_ \
    (GUTIL_VERSION_MAJOR, GUTIL_VERSION_MINOR, GUTIL_VERSION_MICRO)

/* Runtime version as a single word */
#define gutil_version() GUTIL_VERSION_ \
    (gutil_version_major, gutil_version_minor, gutil_version_micro)

/* Specific versions */
#define GUTIL_VERSION_1_0_69 GUTIL_VERSION_(1,0,69)
#define GUTIL_VERSION_1_0_70 GUTIL_VERSION_(1,0,70)
#define GUTIL_VERSION_1_0_71 GUTIL_VERSION_(1,0,71)
#define GUTIL_VERSION_1_0_72 GUTIL_VERSION_(1,0,72)

G_END_DECLS

#endif /* GUTIL_VERSION_H */

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
