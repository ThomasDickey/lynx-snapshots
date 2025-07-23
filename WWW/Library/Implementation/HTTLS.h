/*
 * $LynxId: HTTLS.h,v 1.1 2025/07/22 00:19:55 tom Exp $
 *
 * Macros for HTTLS.c
 */

#ifndef HTTLS_H
#define HTTLS_H

#include <HTUtils.h>

#ifdef USE_SSL

#define free_func free__func

#if defined(USE_OPENSSL_INCL)

#include <openssl/ssl.h>
#include <openssl/crypto.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#elif defined(USE_GNUTLS_INCL)

#include <tidy_tls.h>

#else

#include <ssl.h>
#include <crypto.h>
#include <rand.h>
#include <err.h>

#endif /* USE_OPENSSL_INCL */

#undef free_func

#ifdef __cplusplus
extern "C" {
#endif
    extern SSL *SSL_handle;

    extern SSL *HTGetSSLHandle(void);
    extern SSL *HTSSLConnect(int fd, const char *url,
			     int min_version, int max_version);
    extern void HTSSLInitPRNG(void);
    extern int HTGetSSLCharacter(void *handle);

    /* TLS version number macros for HTSSLConnect */
#define HTTLS_ANY_VERSION 0	/* Don't configure a specific min/max version,
				   just allow the lowest/highest version
				   available */

    /* Version number polyfills. The libssl TLS version number macros are 2 bit
     * unsigned integers, so by setting these to negative numbers we can detect
     * versions of TLS that libssl doesn't support, and create a consistent API.
     * */
#ifndef SSL3_VERSION
#define SSL3_VERSION -1
#endif
#ifndef TLS1_VERSION
#define TLS1_VERSION -2
#endif
#ifndef TLS1_1_VERSION
#define TLS1_1_VERSION -3
#endif
#ifndef TLS1_2_VERSION
#define TLS1_2_VERSION -4
#endif
#ifndef TLS1_3_VERSION
#define TLS1_3_VERSION -5
#endif

#define HTSSL_MAX_VERSION SSL3_VERSION	/* The highest SSL version. This can be
					   used to disable TLS support on a
					   connection. */

#ifdef __cplusplus
}
#endif
#endif				/* USE_SSL */
#endif				/* HTTLS_H */
